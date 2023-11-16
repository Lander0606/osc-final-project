#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

#include <sys/types.h>

#include "sensor_db.h"
#include "logger.h"
#include "config.h"

int main()
{
    // First test: correct use of sensor_db functions
    printf("***** TEST 1: *****\n");
    FILE *f = open_db(NULL, true);

    sleep(1);
    sensor_id_t id = 1;
    sensor_value_t v = 0.001;
    sensor_ts_t ts = time(NULL);
    insert_sensor(f, id, v, ts);

    id = 2;
    v = 0.002;
    ts = time(NULL);
    insert_sensor(f, id, v, ts);
    id = 3;
    v = 0.003;
    ts = time(NULL);
    insert_sensor(f, id, v, ts);
    sleep(5);
    insert_sensor(f, 4, v, ts);

    close_db(f);

    // Second test: invalid database file name
    printf("***** TEST 2: *****\n");
    FILE *f1 = open_db(NULL, true);
    insert_sensor(f1, id, v, ts);
    close_db(f1);

    // Third test: no file opened when inserting
    printf("***** TEST 3: *****\n");
    insert_sensor(f1, id, v, ts);

    // Fourth test: close a non-opened file
    printf("***** TEST 4: *****\n");
    close_db(f1);

    // Fifth test: opening a database twice and closing both
    printf("***** TEST 5: *****\n");
    FILE * f2 = open_db("sensor_db.csv", true);
    FILE * f3 = open_db("sensor_db.csv", true);
    close_db(f2);
    close_db(f3);

    return 0;
}
