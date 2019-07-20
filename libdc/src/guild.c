#include <dc/guild.h>
#include <dc/refable.h>

#include "internal.h"

struct dc_guild_
{
    dc_refable_t ref;

    char *name;
    char *id;

    GPtrArray *channels;
};

static void dc_guild_free(dc_guild_t ptr)
{
    free(ptr->name);
    free(ptr->id);

    if (ptr->channels != NULL) {
        g_ptr_array_unref(ptr->channels);
        ptr->channels = NULL;
    }

    free(ptr);
}

dc_guild_t dc_guild_new(void)
{
    dc_guild_t p = calloc(1, sizeof(struct dc_guild_));
    return_if_true(p == NULL, NULL);

    p->ref.cleanup = (dc_cleanup_t)dc_guild_free;

    p->channels = g_ptr_array_new_with_free_func((GDestroyNotify)dc_unref);
    if (p->channels == NULL) {
        free(p);
        return NULL;
    }

    return dc_ref(p);
}

dc_guild_t dc_guild_from_json(json_t *j)
{
    dc_guild_t g = dc_guild_new();
    json_t *val = NULL;
    json_t *c = NULL;
    size_t idx = 0;

    val = json_object_get(j, "name");
    goto_if_true(val == NULL || !json_is_string(val), error);
    g->name = strdup(json_string_value(val));

    val = json_object_get(j, "id");
    goto_if_true(val == NULL || !json_is_string(val), error);
    g->id = strdup(json_string_value(val));

    /* there is a ton of more information here, that we should look
     * to add, including "member_count", "owner_id", but for now "channels"
     * will do nicely
     */
    val = json_object_get(j, "channels");
    goto_if_true(val == NULL || !json_is_array(val), error);

    json_array_foreach(val, idx, c) {
        dc_channel_t chan = dc_channel_from_json(c);
        continue_if_true(chan == NULL);
        g_ptr_array_add(g->channels, chan);
    }

    return g;

error:

    dc_unref(g);
    return NULL;
}

size_t dc_guild_channels(dc_guild_t d)
{
    return_if_true(d == NULL || d->channels == NULL, 0);
    return d->channels->len;
}

dc_channel_t dc_guild_nth_channel(dc_guild_t d, size_t idx)
{
    return_if_true(d == NULL || d->channels == NULL, NULL);
    return_if_true(idx >= d->channels->len, NULL);
    return g_ptr_array_index(d->channels, idx);
}

dc_channel_t dc_guild_channel_by_name(dc_guild_t g, char const *name)
{
    return_if_true(g == NULL || name == NULL, NULL);
    size_t i = 0;

    for (i = 0; i < g->channels->len; i++) {
        dc_channel_t c = g_ptr_array_index(g->channels, i);
        if (strcmp(dc_channel_name(c), name) == 0) {
            return c;
        }
    }

    return NULL;
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
