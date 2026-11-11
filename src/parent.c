#include "shared.h"
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void) {
    int fd = open("shared_memory_file", O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    if (ftruncate(fd, sizeof(SharedData)) == -1) {
        perror("ftruncate");
        close(fd);
        return 1;
    }

    SharedData* shared = mmap(NULL, sizeof(SharedData), PROT_READ | PROT_WRITE,
                              MAP_SHARED, fd, 0);
    if (shared == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    if (sem_init(&shared->sem_to_child, 1, 0) == -1 ||
        sem_init(&shared->sem_to_parent, 1, 0) == -1) {
        perror("sem_init");
        return 1;
    }
    shared->is_done = 0;

    char fileName[256];
    printf("Введите имя файла для записи результата: ");
    fflush(stdout);
    if (fgets(fileName, sizeof(fileName), stdin) == NULL)
        return 1;
    fileName[strcspn(fileName, "\n")] = 0;

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        char fd_str[10];
        sprintf(fd_str, "%d", fd);
        char* args[] = {"./child", fd_str, fileName, NULL};
        execv("./child", args);
        perror("execv");
        exit(1);
    }

    printf("\nВводите строки (числа через пробел). Ctrl+D - выход.\n");

    char input[256];
    while (1) {
        printf("> ");
        fflush(stdout);
        if (fgets(input, sizeof(input), stdin) == NULL)
            break;

        strncpy(shared->buffer, input, MAX_STR_LEN - 1);
        shared->buffer[MAX_STR_LEN - 1] = '\0';
        sem_post(&shared->sem_to_child);

        sem_wait(&shared->sem_to_parent);
    }

    shared->is_done = 1;
    sem_post(&shared->sem_to_child);

    int status;
    waitpid(pid, &status, 0);

    sem_destroy(&shared->sem_to_child);
    sem_destroy(&shared->sem_to_parent);
    munmap(shared, sizeof(SharedData));
    close(fd);
    unlink("shared_memory_file");

    printf("\nРабота завершена.\n");
    return 0;
}