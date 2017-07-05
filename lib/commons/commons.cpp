#include "commons.h"

void *checked_free(void *ptr) {
    if (ptr != NULL) {
        free(ptr);
    }
    return NULL;
}

void *checked_delete(void *ptr) {
    if (ptr != NULL) {
        delete ptr;
    }
    return NULL;
}