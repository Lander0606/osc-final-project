#include <stdio.h>
#include <unistd.h>
#include "sensor_db.h"
#include "logger.h"

// Define pipe constants
#define SIZE 1
#define READ_END 0
#define WRITE_END 1

// Define the non-printable ASCII characters for insert and end message
#define INSERT_MSG 17
#define OPEN_MSG 18
#define CLOSE_MSG 19
#define OPEN_ERR_MSG 20
#define INSERT_ERR_MSG 21
#define CLOSE_ERR_MSG 22

int fd[2];
bool child_created = false;

int create_child();

int create_child() {

    if (pipe(fd) == -1){
        printf("Error while creating pipe\n");
        return -1;
    }

    child_created = true;
    pid_t pid;
    pid = fork();

    if (pid < 0) { // Child process creation error
        printf("Error while creating child process\n");
        child_created = false;
        return -1;
    }

    if (pid > 0) { // Parent process code
        close(fd[READ_END]);
    }

    else { // Child process code

        char read_msg;
        close(fd[WRITE_END]);
        create_log_process();
        read(fd[READ_END], &read_msg, SIZE);

        while(read_msg != CLOSE_MSG) {

            if(read_msg == INSERT_MSG)
                write_to_log_process("Data inserted.");
            else if(read_msg == INSERT_ERR_MSG)
                write_to_log_process("Error while inserting data");
            else if(read_msg == OPEN_MSG)
                write_to_log_process("Data file opened.");
            else if(read_msg == OPEN_ERR_MSG)
                write_to_log_process("Error while opening file: database file already opened");
            else if(read_msg == CLOSE_ERR_MSG)
                write_to_log_process("Error while closing file.");

            read(fd[READ_END], &read_msg, SIZE);
        }

        write_to_log_process("Data file closed.");
        end_log_process();
        close(fd[READ_END]);
        exit(EXIT_SUCCESS);
    }

    return 0;

}

FILE * open_db(char * filename, bool append) {

    if(child_created) {
        char write_msg = OPEN_ERR_MSG;
        write(fd[WRITE_END], &write_msg, SIZE);
        return NULL;
    } else if (filename == NULL) {
        printf("Error while opening file: invalid filename pointer\n");
        return NULL;
    }

    FILE * fp;

    if(append)
        fp = fopen(filename, "a");
    else
        fp = fopen(filename, "w");

    if(fp != NULL) {
        int result = create_child();
        if(result < 0) {
            fclose(fp);
            return NULL;
        }
        char write_msg = OPEN_MSG;
        write(fd[WRITE_END], &write_msg, SIZE);
    } else {
        printf("Error while opening file: file open error\n");
    }
    return fp;
}

int insert_sensor(FILE * f, sensor_id_t id, sensor_value_t value, sensor_ts_t ts) {

    if(f == NULL && child_created) {
        char write_msg = INSERT_ERR_MSG;
        write(fd[WRITE_END], &write_msg, SIZE);
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
        write(fd[WRITE_END], &write_msg, SIZE);
    } else {
        char write_msg = INSERT_ERR_MSG;
        write(fd[WRITE_END], &write_msg, SIZE);
    }

    return result;

}

int close_db(FILE * f) {

    if(f == NULL && child_created) {
        char write_msg = CLOSE_ERR_MSG;
        write(fd[WRITE_END], &write_msg, SIZE);
        return -1;
    } else if (f == NULL) {
        printf("Error while closing file: invalid file pointer\n");
        return -1;
    }

    int result = fclose(f);

    if(result == 0) {
        char write_msg = CLOSE_MSG;
        write(fd[WRITE_END], &write_msg, SIZE);
        close(fd[WRITE_END]);
    } else if(child_created) {
        char write_msg = CLOSE_ERR_MSG;
        write(fd[WRITE_END], &write_msg, SIZE);
    } else {
        printf("Error while closing file: database file not opened yet\n");
    }

    return result;

}