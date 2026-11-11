#ifndef SHARED_H
#define SHARED_H

#include <semaphore.h>

#define MAX_STR_LEN 1024

typedef struct {
    sem_t sem_to_child;
    sem_t sem_to_parent;
    int is_done;
    char buffer[MAX_STR_LEN];
} SharedData;

#endif