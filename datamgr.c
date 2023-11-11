/**
 * \author Lander Van Loock
 */

#define SET_MAX_TEMP 30
#define SET_MIN_TEMP 15
#include "datamgr.h"
#include "lib/dplist.h"


void datamgr_parse_sensor_files(FILE *fp_sensor_map, FILE *fp_sensor_data) {
    dplist_t * list = dpl_create();
}

void datamgr_free() {

}

uint16_t datamgr_get_room_id(sensor_id_t sensor_id) {

}

sensor_value_t datamgr_get_avg(sensor_id_t sensor_id) {

}

time_t datamgr_get_last_modified(sensor_id_t sensor_id) {

}

int datamgr_get_total_sensors() {

}
