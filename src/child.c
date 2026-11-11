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
        char* p = shared->buffer;

        while (*p) {
            while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') {
                p++;
            }
            if (*p == '\0')
                break;

            char* end;
            double val = strtod(p, &end);
            if (end == p) {
                invalid = 1;
                while (*p && *p != ' ' && *p != '\t' && *p != '\n' &&
                       *p != '\r') {
                    p++;
                }
                continue;
            }

            sum += val;
            count++;
            p = end;
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