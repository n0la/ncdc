#include <dc/util.h>

#include "internal.h"

void dc_util_dump_json(json_t *j)
{
    return_if_true(j == NULL,);
    char *str = json_dumps(j, JSON_COMPACT);

    printf("%s\n", str);
    free(str);
}
