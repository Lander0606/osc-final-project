#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <inttypes.h>
#include "lib/tcpsock.h"
#include "connmgr.h"
#include "config.h"


void* connectionManager(void* param) {
    tcpsock_t *server;
    int conn_counter = 0;
    conn_param * params = param;

    int MAX_CONN = params->max_conn;
    int PORT = params->port;
    //int fd_write = params->fd_write;
    //sbuffer_t * buffer = params->buffer;

    tcpsock_t *client[MAX_CONN];

    pthread_t tid[MAX_CONN];
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    if (tcp_passive_open(&server, PORT) != TCP_NO_ERROR) exit(EXIT_FAILURE);
    do {
        if (tcp_wait_for_connection(server, &client[conn_counter]) != TCP_NO_ERROR) exit(EXIT_FAILURE);
        //TODO: write to log process
        printf("Incoming client connection\n");
        pthread_create(&tid[conn_counter], &attr, listenToSocket, client[conn_counter]);
        conn_counter++;
    } while (conn_counter < MAX_CONN);
    int i;
    for(i = 0; i<MAX_CONN; i++) {
        pthread_join(tid[i], NULL);
    }
    if (tcp_close(&server) != TCP_NO_ERROR) exit(EXIT_FAILURE);
    pthread_exit(0);
}

void* listenToSocket(void *param) {
    tcpsock_t *client = param;
    sensor_data_t data;
    int bytes, result;
    do {
        // read sensor ID
        bytes = sizeof(data.id);
        result = tcp_receive(client, (void *) &data.id, &bytes);
        // read temperature
        bytes = sizeof(data.value);
        result = tcp_receive(client, (void *) &data.value, &bytes);
        // read timestamp
        bytes = sizeof(data.ts);
        result = tcp_receive(client, (void *) &data.ts, &bytes);
        if ((result == TCP_NO_ERROR) && bytes) {
            printf("sensor id = %" PRIu16 " - temperature = %g - timestamp = %ld\n", data.id, data.value,
                    (long int) data.ts);
        }
    } while (result == TCP_NO_ERROR);
    if (result == TCP_CONNECTION_CLOSED)
        //TODO: write to log process
        printf("Peer has closed connection\n");
    else
        printf("Error occured on connection to peer\n");
    tcp_close(&client);
    pthread_exit(0);
}