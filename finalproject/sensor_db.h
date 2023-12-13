#include <stdio.h>
#include <stdlib.h>
#include "config.h"

void* storageManager(void* param);
int insert_sensor(FILE * f, sensor_data_t * data, int fd);