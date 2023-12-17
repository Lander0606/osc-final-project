#include <stdio.h>
#include <stdlib.h>
#include "config.h"

typedef struct {
    uint16_t sensor_id;
    uint16_t room_id;
    double running_avg;
    time_t last_modified;
    uint8_t data_counter;
    double sensor_data[RUN_AVG_LENGTH];
} sensor_node;

void* dataManager(void* param);

void * element_copy(void * element);
void element_free(void ** element);
int element_compare(void * x, void * y);

sensor_node * get_node_by_sensor_id(sensor_id_t sensor_id);

void datamgr_parse_sensor_files(FILE *fp_sensor_map, sbuffer_t * buffer, int fd);

void datamgr_free();