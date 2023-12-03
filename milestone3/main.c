#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "sbuffer.h"

pthread_mutex_t mutex;
pthread_cond_t condvar;

void *writer_thread(void *argw);
void *reader_thread(void *argr);

int main(int argc, char *argv[]){

    sbuffer_t *buffer = NULL;
    sbuffer_init(&buffer);

    pthread_t tid[3];
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&condvar, NULL);

    pthread_create(&tid[0], &attr, writer_thread, buffer);
    pthread_create(&tid[1], &attr, reader_thread, buffer);
    pthread_create(&tid[2], &attr, reader_thread, buffer);

    pthread_join(tid[0], NULL);
    pthread_join(tid[1], NULL);
    pthread_join(tid[2], NULL);

    sbuffer_free(&buffer);

    return 0;
}

void *writer_thread(void *argw) {

    FILE * data = fopen("sensor_data", "rb");
    sensor_data_t *sens_data = malloc(sizeof(sensor_data_t));
    uint16_t sensor_id;
    double temperature;
    time_t timestamp;

    while (!feof(data)) {
        fread((&sensor_id), sizeof(sensor_id), 1, data);
        fread((&temperature), sizeof(temperature), 1, data);
        fread((&timestamp), sizeof(timestamp), 1, data);

        sens_data->id = sensor_id;
        sens_data->value = temperature;
        sens_data->ts = timestamp;

        // TODO: critical section
        sbuffer_insert(argw, sens_data);
        usleep(10000);
    }

    sensor_data_t *end_data = malloc(sizeof(sensor_data_t));
    end_data->id = 0;

    // TODO: critical section
    sbuffer_insert(argw, end_data);

    fclose(data);
    free(sens_data);
    free(end_data);
    pthread_exit(0);
}

void *reader_thread(void *argr) {
    int end = 0;

    while(end == 0) {
        
    }

    pthread_exit(0);
}