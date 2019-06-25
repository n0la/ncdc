#ifndef DC_REFABLE_H
#define DC_REFABLE_H

typedef void (*dc_cleanup_t)(void *);

typedef struct {
    int ref;
    dc_cleanup_t cleanup;
} dc_refable_t;

void *dc_ref(void *);
void dc_unref(void *);

#endif
