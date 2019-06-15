#include <ncdc/refable.h>

void *ncdc_ref(void *arg)
{
    ncdc_refable_t *ptr = NULL;

    return_if_true(arg == NULL,NULL);

    ptr = (ncdc_refable_t *)arg;
    ++ptr->ref;

    return arg;
}

void ncdc_unref(void *arg)
{
    ncdc_refable_t *ptr = NULL;

    return_if_true(arg == NULL,);

    ptr = (ncdc_refable_t *)arg;
    if ((--ptr->ref) <= 0 && ptr->cleanup != NULL) {
        ptr->cleanup(arg);
    }
}
