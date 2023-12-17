/**
 * \author Lander Van Loock
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "lib/tcpsock.h"
#include "connmgr.h"
#include "config.h"

int fd_write_conn;
sbuffer_t * buffer;

void* connectionManager(void* param) {
    tcpsock_t *server;
    int conn_counter = 0;
    conn_param * params = param;

    int MAX_CONN = params->max_conn;
    int PORT = params->port;
    fd_write_conn = params->fd_write;
    buffer = params->buffer;

    tcpsock_t *client[MAX_CONN];

    // Connection threads initialization
    pthread_t tid[MAX_CONN];
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    if (tcp_passive_open(&server, PORT) != TCP_NO_ERROR) exit(EXIT_FAILURE);
    do {
        if (tcp_wait_for_connection(server, &client[conn_counter]) != TCP_NO_ERROR) exit(EXIT_FAILURE);
        pthread_create(&tid[conn_counter], &attr, listenToSocket, client[conn_counter]);
        conn_counter++;
    } while (conn_counter < MAX_CONN);
    int i;
    for(i = 0; i<MAX_CONN; i++) {
        pthread_join(tid[i], NULL);
    }
    if (tcp_close(&server) != TCP_NO_ERROR) exit(EXIT_FAILURE);

    // Put end of data character in buffer
    sensor_data_t * end_data = malloc(sizeof(sensor_data_t));
    end_data->id = 0;
    end_data->value = 0;
    end_data->ts = 0;
    sbuffer_insert(buffer, end_data);
    free(end_data);

    // Exit connection thread
    pthread_exit(0);
}

void* listenToSocket(void *param) {
    tcpsock_t *client = param;
    sensor_data_t data_conn;
    int logged = 0;
    int bytes, result;
    do {
        // read sensor ID
        bytes = sizeof(data_conn.id);
        result = tcp_receive(client, (void *) &data_conn.id, &bytes);
        // read temperature
        bytes = sizeof(data_conn.value);
        result = tcp_receive(client, (void *) &data_conn.value, &bytes);
        // read timestamp
        bytes = sizeof(data_conn.ts);
        result = tcp_receive(client, (void *) &data_conn.ts, &bytes);
        if ((result == TCP_NO_ERROR) && bytes) {
            if(logged == 0) {
                char write_msg_conn[SIZE] = "";
                sprintf(write_msg_conn, "Sensor node %d has opened a new connection", data_conn.id);
                write(fd_write_conn, &write_msg_conn, SIZE);
                logged = 1;
            }
            sbuffer_insert(buffer, &data_conn);
        }
    } while (result == TCP_NO_ERROR);
    if (result == TCP_CONNECTION_CLOSED) {
        char write_msg_conn[SIZE];
        sprintf(write_msg_conn, "Sensor node %d has closed the connection", data_conn.id);
        write(fd_write_conn, &write_msg_conn, SIZE);
    } else
        printf("Error occured on connection to peer\n");
    
    tcp_close(&client);
    pthread_exit(0);
}