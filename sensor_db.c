#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "sensor_db.h"
#include "sbuffer.h"

void* storageManager(void* param) {
    thread_param * params = param;
    int fd_write = params->fd_write;
    sbuffer_t * buffer = params->buffer;

    // Create empty database file
    FILE * file = fopen("data.csv", "w");
    fclose(file);
    char open_msg[SIZE] = "A new data.csv file has been created.";
    write(fd_write, &open_msg, SIZE);

    sensor_data_t * data_db = malloc(sizeof(sensor_data_t));
    int result = sbuffer_remove(buffer, data_db);

    while(result != 1) {
        file = fopen("data.csv", "a");
        insert_sensor(file, data_db, fd_write);
        fclose(file);

        result = sbuffer_remove(buffer, data_db);
    }

    char close_msg_db[SIZE] = "The data.csv file has been closed.";
    write(fd_write, &close_msg_db, SIZE);
    free(data_db);
    pthread_exit(0);
}

int insert_sensor(FILE * f, sensor_data_t * data, int fd) {

    if(f == NULL) {
        char write_msg_db[SIZE] = "Invalid file pointer in storage manager";
        write(fd, &write_msg_db, SIZE);
        return -1;
    }

    int result = fprintf(f, "%d, %f, %ld\n", data->id, data->value, data->ts);

    if(result > 0) {
        char write_msg_db[SIZE] = "";
        sprintf(write_msg_db, "Data insertion from sensor %d succeeded", data->id);
        write(fd, &write_msg_db, SIZE);
    } else {
        char write_msg_db[SIZE] = "";
        sprintf(write_msg_db, "Data insertion from sensor %d not succeeded: error", data->id);
        write(fd, &write_msg_db, SIZE);
    }

    return result;

}