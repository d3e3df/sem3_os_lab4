#include "shared.h"
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Использование: %s <fd> <имя_файла>\n", argv[0]);
        return 1;
    }

    int fd = atoi(argv[1]);
    char* filename = argv[2];

    FILE* out = fopen(filename, "w");
    if (out == NULL) {
        perror("fopen");
        return 1;
    }

    SharedData* shared = mmap(NULL, sizeof(SharedData), PROT_READ | PROT_WRITE,
                              MAP_SHARED, fd, 0);
    if (shared == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    while (1) {
        if (sem_wait(&shared->sem_to_child) == -1) {
            perror("sem_wait sem_to_child");
            break;
        }

        if (shared->is_done) {
            break;
        }

        double sum = 0.0;
        int count = 0;
        int invalid = 0;

        char local_buffer[MAX_STR_LEN];
        strncpy(local_buffer, shared->buffer, MAX_STR_LEN - 1);
        local_buffer[MAX_STR_LEN - 1] = '\0';

        char* token = strtok(local_buffer, " \t\n\r");

        while (token != NULL) {
            char* end;
            double val = strtod(token, &end);

            if (*end != '\0' || end == token) {
                invalid = 1;
                break;
            }

            sum += val;
            count++;
            token = strtok(NULL, " \t\n\r");
        }

        if (count == 0 || invalid) {
            fprintf(out, "Некорректный ввод\n");
            fflush(out);
            printf("[Child] Некорректный ввод\n");
            fflush(stdout);
        } else {
            fprintf(out, "Сумма %d чисел = %.6f\n", count, sum);
            fflush(out);
            printf("[Child] Сумма %d чисел = %.6f\n", count, sum);
            fflush(stdout);
        }

        sem_post(&shared->sem_to_parent);
    }

    fclose(out);
    munmap(shared, sizeof(SharedData));
    return 0;
}