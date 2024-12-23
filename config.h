/**
* \author Lander Van Loock
*/

#include <stdint.h>
#include <time.h>

#ifndef _CONFIG_H_
#define _CONFIG_H_

#ifndef RUN_AVG_LENGTH
#define RUN_AVG_LENGTH 5
#endif

#ifndef TIMEOUT
#define TIMEOUT 5
#endif

#ifndef SET_MAX_TEMP
#define SET_MAX_TEMP 20
#endif

#ifndef SET_MIN_TEMP
#define SET_MIN_TEMP 10
#endif

// Define message pipe constants
#define SIZE 128
#define WRITE_END 1
#define READ_END 0

typedef uint16_t sensor_id_t;
typedef double sensor_value_t;
typedef time_t sensor_ts_t;

typedef struct sbuffer sbuffer_t;

typedef struct {
    sensor_id_t id;
    sensor_value_t value;
    sensor_ts_t ts;
} sensor_data_t;

typedef struct {
    sbuffer_t *buffer;
    int fd_write;
} thread_param;

#endif /* _CONFIG_H_ */
