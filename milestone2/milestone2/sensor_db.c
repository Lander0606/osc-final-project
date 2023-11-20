#include <stdio.h>
#include <unistd.h>
#include "sensor_db.h"
#include "logger.h"

// Define message pipe constants
#define SIZE 1

// Define the non-printable ASCII characters for insert and end message
#define INSERT_MSG 17
#define OPEN_MSG 18
#define CLOSE_MSG 19
#define OPEN_ERR_MSG 20
#define INSERT_ERR_MSG 21
#define CLOSE_ERR_MSG 22
#define END_MSG 23

bool child_created = false;
int fd_write;
FILE * fp;

FILE * open_db(char * filename, bool append) {

    if(child_created) {
        return fp;
    } else if (filename == NULL) {
        printf("Error while opening file: invalid filename pointer\n");
        return NULL;
    }

    fd_write = create_log_process();
    if(fd_write < 0) {
        printf("Error while opening file: couldn't create child\n");
        return NULL;
    }
    child_created = true;

    if(append)
        fp = fopen(filename, "a");
    else
        fp = fopen(filename, "w");

    if(fp != NULL) {
        char write_msg = OPEN_MSG;
        write(fd_write, &write_msg, SIZE);
    } else {
        char write_msg = END_MSG;
        write(fd_write, &write_msg, SIZE);
        close(fd_write);
        child_created = false;
        printf("Error while opening file: file open error\n");
    }
    return fp;
}

int insert_sensor(FILE * f, sensor_id_t id, sensor_value_t value, sensor_ts_t ts) {

    if(f == NULL && child_created) {
        char write_msg = INSERT_ERR_MSG;
        write(fd_write, &write_msg, SIZE);
        return -1;
    } else if(f == NULL) {
        printf("Error while inserting: invalid file pointer\n");
        printf("Error while inserting: database file not opened yet\n");
        return -1;
    } else if(!child_created) {
        printf("Error while inserting: database file not opened yet\n");
        return -1;
    }

    int result = fprintf(f, "%d, %f, %ld\n", id, value, ts);
    if(result > 0) {
        char write_msg = INSERT_MSG;
        write(fd_write, &write_msg, SIZE);
    } else {
        char write_msg = INSERT_ERR_MSG;
        write(fd_write, &write_msg, SIZE);
    }

    return result;

}

int close_db(FILE * f) {

    if(f == NULL && child_created) {
        char write_msg = CLOSE_ERR_MSG;
        write(fd_write, &write_msg, SIZE);
        return -1;
    } else if (f == NULL) {
        printf("Error while closing file: invalid file pointer\n");
        return -1;
    } else if (!child_created) {
        printf("Error while closing file: file already closed\n");
        return -1;
    }

    int result = fclose(f);

    if(result == 0) {
        char write_msg = CLOSE_MSG;
        write(fd_write, &write_msg, SIZE);
        close(fd_write);
        child_created = false;
        fp = NULL;
    } else if(child_created) {
        char write_msg = CLOSE_ERR_MSG;
        write(fd_write, &write_msg, SIZE);
    } else {
        printf("Error while closing file: database file not opened yet\n");
    }

    return result;

}