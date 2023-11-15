#include "sensor_db.h"

FILE * open_db(char * filename, bool append) {
    FILE *fp;
    if(append)
        fp = fopen(filename, "a");
    else
        fp = fopen(filename, "w");

    return fp;
}

int insert_sensor(FILE * f, sensor_id_t id, sensor_value_t value, sensor_ts_t ts) {
    return fprintf(f, "%d, %f, %ld\n", id, value, ts);
}

int close_db(FILE * f) {
    return fclose(f);
}