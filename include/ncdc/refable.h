#ifndef NCDC_REFABLE_H
#define NCDC_REFABLE_H

#include <ncdc/ncdc.h>

typedef void (*cleanup_t)(void *);

typedef struct {
    int ref;
    cleanup_t cleanup;
} ncdc_refable_t;

void *ncdc_ref(void *);
void ncdc_unref(void *);

#endif
