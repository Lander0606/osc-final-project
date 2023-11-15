/**
 * \author Lander Van Loock
 */

#define SET_MAX_TEMP 25
#define SET_MIN_TEMP 15

#include <assert.h>
#include "datamgr.h"
#include "lib/dplist.h"

typedef struct {
    uint16_t sensor_id;
    uint16_t room_id;
    double running_avg;
    time_t last_modified;
    uint8_t data_counter;
    double sensor_data[RUN_AVG_LENGTH];
} sensor_node;

void * element_copy(void * element);
void element_free(void ** element);
int element_compare(void * x, void * y);

sensor_node * get_node_by_sensor_id(sensor_id_t sensor_id);

dplist_t * nodes = NULL;

void * element_copy(void * element) {
    sensor_node * copy = malloc(sizeof (sensor_node));
    assert(copy != NULL);
    copy->sensor_id = ((sensor_node*)element)->sensor_id;
    copy->room_id = ((sensor_node*)element)->room_id;
    copy->running_avg = ((sensor_node*)element)->running_avg;
    copy->last_modified = ((sensor_node*)element)->last_modified;
    copy->data_counter = ((sensor_node*)element)->data_counter;
    return (void *) copy;
}

void element_free(void ** element) {
    free(*element);
    *element = NULL;
}

int element_compare(void * x, void * y) {
    return ((((sensor_node *)x)->sensor_id < ((sensor_node *)y)->sensor_id) ? -1 : (((sensor_node *)x)->sensor_id == ((sensor_node *)y)->sensor_id) ? 0 : 1);
}

void datamgr_parse_sensor_files(FILE *fp_sensor_map, FILE *fp_sensor_data) {
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

    while (!feof(fp_sensor_data)) {
        fread((&sensor_id), sizeof(sensor_id), 1, fp_sensor_data);
        fread((&temperature), sizeof(temperature), 1, fp_sensor_data);
        fread((&timestamp), sizeof(timestamp), 1, fp_sensor_data);

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
                if (node->running_avg > SET_MAX_TEMP)
                    printf("Info: Sensor %d in room %d: too hot!\n", node->sensor_id, node->room_id);
                else if (node->running_avg < SET_MIN_TEMP)
                    printf("Info: Sensor %d in room %d: too cold!\n", node->sensor_id, node->room_id);
            }


            if (node->data_counter == RUN_AVG_LENGTH - 1)
                node->data_counter = 0;
            else
                ++node->data_counter;
        }
    }

}

void datamgr_free() {
    assert(nodes != NULL);
    dpl_free(&nodes, false);
}

sensor_node * get_node_by_sensor_id(sensor_id_t sensor_id) {
    assert(nodes != NULL);
    sensor_node * node = (sensor_node *)malloc(sizeof(sensor_node));
    node->sensor_id = sensor_id;
    int index = dpl_get_index_of_element(nodes, node);
    free(node);
    if(index < 0) {
        printf("Error: No sensor in nodes list with id %d\n", sensor_id);
        return NULL;
    }
    return (sensor_node * )dpl_get_element_at_index(nodes, index);
}

uint16_t datamgr_get_room_id(sensor_id_t sensor_id) {
    return get_node_by_sensor_id(sensor_id)->room_id;
}

sensor_value_t datamgr_get_avg(sensor_id_t sensor_id) {
    return get_node_by_sensor_id(sensor_id)->running_avg;
}

time_t datamgr_get_last_modified(sensor_id_t sensor_id) {
    return get_node_by_sensor_id(sensor_id)->last_modified;
}

int datamgr_get_total_sensors() {
    assert(nodes != NULL);
    return dpl_size(nodes);
}
