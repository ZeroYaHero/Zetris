#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// This is not my code: https://stackoverflow.com/questions/6127503/shuffle-array-in-c
// TODO: malloc really is not necessary since I am always using this for the same reason.
static inline void shuffle(void *array, size_t n, size_t size) {
    char* tmp = malloc(size * sizeof(char)); // This had to change as the source I believe was using 
    char *arr = array;
    size_t stride = size * sizeof(char);

    if (n > 1) {
        size_t i;
        for (i = 0; i < n - 1; ++i) {
            size_t rnd = (size_t) rand();
            size_t j = i + rnd / (RAND_MAX / (n - i) + 1);

            memcpy(tmp, arr + j * stride, size);
            memcpy(arr + j * stride, arr + i * stride, size);
            memcpy(arr + i * stride, tmp, size);
        }
    }
    free(tmp); // Of course need to free memory
}

#endif //UTIL_H
