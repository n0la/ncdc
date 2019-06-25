#include <dc/guild.h>
#include <dc/refable.h>

#include "internal.h"

struct dc_guild_
{
    dc_refable_t ref;

    char *name;
    char *id;
};

static void dc_guild_free(dc_guild_t ptr)
{
    free(ptr->name);
    free(ptr->id);

    free(ptr);
}

dc_guild_t dc_guild_new(void)
{
    dc_guild_t p = calloc(1, sizeof(struct dc_guild_));
    return_if_true(p == NULL, NULL);

    p->ref.cleanup = (dc_cleanup_t)dc_guild_free;

    return p;
}

char const *dc_guild_name(dc_guild_t d)
{
    return_if_true(d == NULL, NULL);
    return d->name;
}

void dc_guild_set_name(dc_guild_t d, char const *val)
{
    return_if_true(d == NULL || val == NULL,);
    free(d->name);
    d->name = strdup(val);
}

char const *dc_guild_id(dc_guild_t d)
{
    return_if_true(d == NULL, NULL);
    return d->id;
}

void dc_guild_set_id(dc_guild_t d, char const *val)
{
    return_if_true(d == NULL || val == NULL,);
    free(d->id);
    d->id = strdup(val);
}
