#include <dc/refable.h>

#include "internal.h"

void *dc_ref(void *arg)
{
    dc_refable_t *ptr = NULL;

    return_if_true(arg == NULL,NULL);

    ptr = (dc_refable_t *)arg;
    ++ptr->ref;

    if (ptr->debug) {
        FILE *F = fopen("refdebug.txt", "a+");
        fprintf(F, "libdc: ref inc: %p: %d\n", ptr, ptr->ref);
        fclose(F);
    }

    return arg;
}

void dc_unref(void *arg)
{
    dc_refable_t *ptr = NULL;

    return_if_true(arg == NULL,);

    ptr = (dc_refable_t *)arg;
    --ptr->ref;

    if (ptr->debug) {
        FILE *F = fopen("refdebug.txt", "a+");
        fprintf(F, "libdc: ref dec: %p: %d\n",
                ptr, ptr->ref
            );
        fclose(F);
    }

    if (ptr->ref <= 0 && ptr->cleanup != NULL) {
        ptr->cleanup(arg);

        if (ptr->debug) {
            FILE *F = fopen("refdebug.txt", "a+");
            fprintf(F, "libdc: ref dec: %p: %d: cleanup!\n",
                    ptr, ptr->ref
                );
            fclose(F);
        }
    }
}

void dc_ref_debug(void *arg)
{
    dc_refable_t *ptr = NULL;
    return_if_true(arg == NULL,);
    ptr = (dc_refable_t *)arg;
    ptr->debug = true;
}
