#include "commons.h"

void *checked_free(void *ptr) {
    if (ptr != NULL) {
        free(ptr);
    }
    return NULL;
}