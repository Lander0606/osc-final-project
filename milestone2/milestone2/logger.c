#include <stdio.h>
#include <time.h>
#include <string.h>
#include "logger.h"

FILE * file;
uint seq_num = 0;

int write_to_log_process(char *msg) {
    time_t curtime;
    time(&curtime);
    char * timestring = ctime(&curtime);
    timestring[strlen(timestring)-1] = '\0';
    int result = fprintf(file, "%d - %s - %s\n", seq_num, timestring, msg);
    ++seq_num;
    return result;
}

int create_log_process() {
    file = fopen("gateway.log", "a");
    return 0;
}

int end_log_process() {
    fclose(file);
    return 0;
}