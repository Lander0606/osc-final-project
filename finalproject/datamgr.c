#include <pthread.h>
#include "datamgr.h"

void* dataManager(void* param) {
    pthread_exit(0);
}