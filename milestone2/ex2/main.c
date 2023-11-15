#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#define SIZE 25
#define READ_END 0
#define WRITE_END 1

int main(void){
    char wmsg[SIZE] = "GrEeTinGs";
    char rmsg[SIZE];
    pid_t pid;
    int fd[2]; // file descriptor

    if (pipe(fd) == -1) {
        printf("Pipe failed\n");
        return 1;
    }

    pid = fork();

    if (pid < 0) { // fork error
        printf("fork failed\n");
        return 1;
    }
    if (pid > 0) { // parent process
        close(fd[READ_END]);
        write(fd[WRITE_END], wmsg, strlen(wmsg)+1);
        close(fd[WRITE_END]);
    }
    else { // child process
        close(fd[WRITE_END]);
        read(fd[READ_END], rmsg, SIZE);
        int n;
        for(n=0; n < strlen(rmsg); ++n) {
            if(islower(rmsg[n]))
                rmsg[n]=toupper(rmsg[n]);
            else
                rmsg[n]=tolower(rmsg[n]);
        }
        printf("%s", rmsg);
        close(fd[READ_END]);
    }
    return 0;
}