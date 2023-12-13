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

    FILE * file = fopen("data.csv", "w");
    char open_msg[SIZE] = "A new data.csv file has been created.";
    write(fd_write, &open_msg, SIZE);

    sensor_data_t * data = malloc(sizeof(sensor_data_t));
    sbuffer_remove(buffer, data);

    while(data->id != 0) {
        file = fopen("data.csv", "a");
        insert_sensor(file, data, fd_write);
        fclose(file);

        sbuffer_remove(buffer, data);
    }

    fclose(file);
    char close_msg[SIZE] = "The data.csv file has been closed.";
    write(fd_write, &close_msg, SIZE);
    free(data);
    pthread_exit(0);
}

int insert_sensor(FILE * f, sensor_data_t * data, int fd) {

    if(f == NULL) {
        char write_msg[SIZE] = "Invalid file pointer in storage manager";
        write(fd, &write_msg, SIZE);
        return -1;
    }

    int result = fprintf(f, "%d, %f, %ld\n", data->id, data->value, data->ts);

    if(result > 0) {
        char write_msg[SIZE];
        sprintf(write_msg, "Data insertion from sensor %d succeeded", data->id);
        write(fd, &write_msg, SIZE);
    } else {
        char write_msg[SIZE];
        sprintf(write_msg, "Data insertion from sensor %d not succeeded: error", data->id);
        write(fd, &write_msg, SIZE);
    }

    return result;

}