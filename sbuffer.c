#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "sbuffer.h"

typedef struct sbuffer_node {
    struct sbuffer_node *next;
    sensor_data_t data;
} sbuffer_node_t;

struct sbuffer {
    sbuffer_node_t *head;
    sbuffer_node_t *tail;
};

pthread_mutex_t mutex1; // Mutex for storage access
pthread_mutex_t mutex2; // Mutex to let data manager and storage manager read both
pthread_cond_t condvar; // Condition variable for when buffer is empty
pthread_cond_t condvar2; // Condition variable to read data twice

int condition = 0; // Variable for condvar 2

int sbuffer_init(sbuffer_t **buffer) {
    *buffer = malloc(sizeof(sbuffer_t));
    if (*buffer == NULL) return SBUFFER_FAILURE;
    (*buffer)->head = NULL;
    (*buffer)->tail = NULL;

    pthread_mutex_init(&mutex1, NULL);
    pthread_mutex_init(&mutex2, NULL);
    pthread_cond_init(&condvar, NULL);
    pthread_cond_init(&condvar2, NULL);

    return SBUFFER_SUCCESS;
}

int sbuffer_free(sbuffer_t **buffer) {
    sbuffer_node_t *dummy;
    if ((buffer == NULL) || (*buffer == NULL)) {
        return SBUFFER_FAILURE;
    }
    while ((*buffer)->head) {
        dummy = (*buffer)->head;
        (*buffer)->head = (*buffer)->head->next;
        free(dummy);
    }
    free(*buffer);
    *buffer = NULL;
    pthread_mutex_destroy(&mutex1);
    pthread_mutex_destroy(&mutex2);
    pthread_cond_destroy(&condvar);
    pthread_cond_destroy(&condvar2);
    return SBUFFER_SUCCESS;
}

int sbuffer_remove(sbuffer_t *buffer, sensor_data_t *data) {
    sbuffer_node_t *dummy;
    if (buffer == NULL) return SBUFFER_FAILURE;
    pthread_mutex_lock(&mutex2);
    while(condition == 0) {
        pthread_cond_wait(&condvar2, &mutex2);
    }
    pthread_mutex_lock(&mutex1);
    while (buffer->head == NULL) {
        pthread_cond_wait(&condvar, &mutex1);
    }
    *data = buffer->head->data;
    dummy = buffer->head;
    if (buffer->head->data.id == 0) {
        pthread_cond_signal(&condvar2);
        pthread_mutex_unlock(&mutex2);
        pthread_mutex_unlock(&mutex1);
        return SBUFFER_NO_DATA;
    }
    if (buffer->head == buffer->tail) // buffer has only one node
    {
        buffer->head = buffer->tail = NULL;
    } else  // buffer has many nodes empty
    {
        buffer->head = buffer->head->next;
    }
    condition = 0;
    pthread_cond_signal(&condvar2);
    pthread_mutex_unlock(&mutex2);
    pthread_mutex_unlock(&mutex1);
    free(dummy);
    return SBUFFER_SUCCESS;
}

int sbuffer_insert(sbuffer_t *buffer, sensor_data_t *data) {
    sbuffer_node_t *dummy;
    if (buffer == NULL) return SBUFFER_FAILURE;
    dummy = malloc(sizeof(sbuffer_node_t));
    if (dummy == NULL) return SBUFFER_FAILURE;
    dummy->data = *data;
    dummy->next = NULL;
    pthread_mutex_lock(&mutex1);
    if (buffer->tail == NULL) // buffer empty
    {
        buffer->head = buffer->tail = dummy;
    } else // buffer not empty
    {
        buffer->tail->next = dummy;
        buffer->tail = buffer->tail->next;
    }
    pthread_cond_signal(&condvar);
    pthread_mutex_unlock(&mutex1);
    return SBUFFER_SUCCESS;
}

int sbuffer_read(sbuffer_t *buffer, sensor_data_t *data) {
    if (buffer == NULL) return SBUFFER_FAILURE;
    pthread_mutex_lock(&mutex2);
    while(condition == 1) {
        pthread_cond_wait(&condvar2, &mutex2);
    }
    condition = 1;
    pthread_mutex_lock(&mutex1);
    while (buffer->head == NULL) {
        pthread_cond_wait(&condvar, &mutex1);
    }
    if (buffer->head->data.id == 0) {
        condition = 1;
        pthread_cond_signal(&condvar2);
        pthread_mutex_unlock(&mutex2);
        pthread_mutex_unlock(&mutex1);
        return SBUFFER_NO_DATA;
    }
    *data = buffer->head->data;
    pthread_cond_signal(&condvar2);
    pthread_mutex_unlock(&mutex2);
    pthread_mutex_unlock(&mutex1);
    return SBUFFER_SUCCESS;
}