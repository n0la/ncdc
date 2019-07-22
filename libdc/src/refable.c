/*
 * Part of ncdc - a discord client for the console
 * Copyright (C) 2019 Florian Stinglmayr <fstinglmayr@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

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
