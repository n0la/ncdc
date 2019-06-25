#include <dc/refable.h>

#include "internal.h"

void *dc_ref(void *arg)
{
    dc_refable_t *ptr = NULL;

    return_if_true(arg == NULL,NULL);

    ptr = (dc_refable_t *)arg;
    ++ptr->ref;

    return arg;
}

void dc_unref(void *arg)
{
    dc_refable_t *ptr = NULL;

    return_if_true(arg == NULL,);

    ptr = (dc_refable_t *)arg;
    if ((--ptr->ref) <= 0 && ptr->cleanup != NULL) {
        ptr->cleanup(arg);
    }
}
