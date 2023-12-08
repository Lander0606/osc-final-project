#include "sbuffer.h"

typedef struct {
    int port;
    int max_conn;
    int fd_write;
    sbuffer_t *buffer;
} conn_param;

void* listenToSocket(void *param);
void* connectionManager(void *param);