/**
 * \author Lander Van Loock
 */

#include <pthread.h>
#include <malloc.h>
#include <unistd.h>
#include "datamgr.h"
#include "lib/dplist.h"
#include "sbuffer.h"

dplist_t * nodes = NULL;

void * element_copy(void * element) {
    sensor_node * copy = malloc(sizeof (sensor_node));
    if(copy != NULL) {
        copy->sensor_id = ((sensor_node*)element)->sensor_id;
        copy->room_id = ((sensor_node*)element)->room_id;
        copy->running_avg = ((sensor_node*)element)->running_avg;
        copy->last_modified = ((sensor_node*)element)->last_modified;
        copy->data_counter = ((sensor_node*)element)->data_counter;
        return (void *) copy;
    }
    return NULL;
}

void element_free(void ** element) {
    free(*element);
    *element = NULL;
}

int element_compare(void * x, void * y) {
    return ((((sensor_node *)x)->sensor_id < ((sensor_node *)y)->sensor_id) ? -1 : (((sensor_node *)x)->sensor_id == ((sensor_node *)y)->sensor_id) ? 0 : 1);
}

void* dataManager(void* param) {
    thread_param * params = param;
    int fd_write_data = params->fd_write;
    sbuffer_t * buffer = params->buffer;

    FILE * map = fopen("room_sensor.map", "r");
    datamgr_parse_sensor_files(map, buffer, fd_write_data);

    datamgr_free();

    pthread_exit(0);
}

void datamgr_parse_sensor_files(FILE *fp_sensor_map, sbuffer_t * buffer, int fd) {
    nodes =  dpl_create(element_copy, element_free, element_compare);

    uint16_t room_id;
    uint16_t sensor_id;
    double temperature;
    time_t timestamp;
    int index = 0;
    sensor_node * node = NULL;

    while(fscanf(fp_sensor_map, "%hd %hd", &room_id, &sensor_id) == 2) {
        sensor_node * sensor = (sensor_node *)malloc(sizeof(sensor_node));
        sensor->sensor_id = sensor_id;
        sensor->room_id = room_id;
        sensor->data_counter = 0;
        sensor->running_avg = -6000; // value when no average is calculated, so less than RUN_AVG_LENGTH values are in list
        dpl_insert_at_index(nodes, sensor, index, false);
        ++index;
    }
    fclose(fp_sensor_map);

    sensor_data_t * data = malloc(sizeof(sensor_data_t));
    int result = sbuffer_read(buffer, data);

    while (result != 1) {
        sensor_id = data->id;
        temperature = data->value;
        timestamp = data->ts;

        node = get_node_by_sensor_id(sensor_id);
        if (node != NULL) {
            node->sensor_data[node->data_counter] = temperature;
            node->last_modified = timestamp;

            if (node->running_avg != -6000 || node->data_counter == RUN_AVG_LENGTH - 1) {
                int n;
                double sum = 0;
                for (n = 0; n < RUN_AVG_LENGTH; ++n)
                    sum += node->sensor_data[n];
                node->running_avg = sum / RUN_AVG_LENGTH;
                if (node->running_avg > SET_MAX_TEMP) {
                    char write_msg[SIZE] = "";
                    sprintf(write_msg, "Sensor node %d reports it’s too cold (avg temp = %f)", node->sensor_id, node->running_avg);
                    write(fd, &write_msg, SIZE);
                } else if (node->running_avg < SET_MIN_TEMP) {
                    char write_msg[SIZE] = "";
                    sprintf(write_msg, "Sensor node %d reports it’s too hot (avg temp = %f)", node->sensor_id, node->running_avg);
                    write(fd, &write_msg, SIZE);
                }
            }

            if (node->data_counter == RUN_AVG_LENGTH - 1)
                node->data_counter = 0;
            else
                ++node->data_counter;
        } else {
            char write_msg[SIZE] = "";
            sprintf(write_msg, "Received sensor data with invalid sensor node ID %d", sensor_id);
            write(fd, &write_msg, SIZE);
        }
        result = sbuffer_read(buffer, data);
    }
    free(data);
}

void datamgr_free() {
    if(nodes != NULL) {
        dpl_free(&nodes, true);
    }
}

sensor_node * get_node_by_sensor_id(sensor_id_t sensor_id) {
    if(nodes != NULL) {
        sensor_node * node = (sensor_node *)malloc(sizeof(sensor_node));
        node->sensor_id = sensor_id;
        int index = dpl_get_index_of_element(nodes, node);
        free(node);
        if(index < 0) {
            return NULL;
        }
        return (sensor_node * )dpl_get_element_at_index(nodes, index);
    }
    return NULL;
}