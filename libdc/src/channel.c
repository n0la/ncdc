#include <dc/channel.h>

#include "internal.h"

struct dc_channel_
{
    dc_refable_t ref;

    dc_channel_type_t type;

    /* snowflake of the channel
     */
    char *id;

    /* Guild ID this channel belongs to, may be NULL
     */
    char *guild_id;

    /* Name of the channel
     */
    char *name;

    /* Slaneeshi filth?
     */
    bool nsfw;

    /* ID of last message in the channel
     */
    char *last_message_id;

    /* list of recipients, array of dc_account_t
     */
    GPtrArray *recipients;

    /* Snowflake of the owner
     */
    char *owner_id;

    /* ID of the parent channel or bot
     */
    char *parent_id;

    /*  application ID of the group DM creator if it is bot-created
     */
    char *application_id;
};

static void dc_channel_free(dc_channel_t c)
{
    return_if_true(c == NULL,);

    free(c->id);
    free(c->guild_id);
    free(c->name);
    free(c->last_message_id);
    free(c->owner_id);
    free(c->parent_id);
    free(c->application_id);

    if (c->recipients != NULL) {
        g_ptr_array_unref(c->recipients);
        c->recipients = NULL;
    }

    free(c);
}

dc_channel_t dc_channel_new(void)
{
    dc_channel_t c = calloc(1, sizeof(struct dc_channel_));
    return_if_true(c == NULL, NULL);

    c->ref.cleanup = (dc_cleanup_t)dc_channel_free;

    c->recipients = g_ptr_array_new_with_free_func(
        (GDestroyNotify)dc_unref
        );

    return dc_ref(c);
}

dc_channel_t dc_channel_from_json(json_t *j)
{
    json_t *v = NULL;

    dc_channel_t c = dc_channel_new();

    goto_if_true(!json_is_object(j), error);

    v = json_object_get(j, "id");
    goto_if_true(v == NULL || !json_is_string(v), error);
    c->id = strdup(json_string_value(v));

    v = json_object_get(j, "type");
    goto_if_true(v == NULL || !json_is_integer(v), error);
    c->type = json_integer_value(v);

    v = json_object_get(j, "guild_id");
    if (v != NULL && json_is_string(v)) {
        c->guild_id = strdup(json_string_value(v));
    }

    v = json_object_get(j, "name");
    if (v == NULL && json_is_string(v)) {
        c->name = strdup(json_string_value(v));
    }

    v = json_object_get(j, "nsfw");
    if (v != NULL && json_is_boolean(v)) {
        c->nsfw = json_boolean_value(v);
    }

    v = json_object_get(j, "last_message_id");
    if (v != NULL && json_is_string(v)) {
        c->last_message_id = strdup(json_string_value(v));
    }

    v = json_object_get(j, "owner_id");
    if (v != NULL && json_is_string(v)) {
        c->owner_id = strdup(json_string_value(v));
    }

    v = json_object_get(j, "parent_id");
    if (v != NULL && json_is_string(v)) {
        c->parent_id = strdup(json_string_value(v));
    }

    v = json_object_get(j, "application_id");
    if (v != NULL && json_is_string(v)) {
        c->application_id = strdup(json_string_value(v));
    }

    v = json_object_get(j, "recipients");
    if (v != NULL && json_is_array(v)) {
        json_t *i = NULL;
        size_t idx = 0;

        json_array_foreach(v, idx, i) {
            dc_account_t a = dc_account_from_json(i);
            if (a != NULL) {
                g_ptr_array_add(c->recipients, a);
            }
        }
    }

    return c;

error:

    dc_unref(c);
    return NULL;
}

json_t *dc_channel_to_json(dc_channel_t c)
{
    json_t *j = NULL;

    return_if_true(c->id == NULL, NULL);

    j = json_object();
    return_if_true(j == NULL, NULL);

    /* I was so close in making a J_SET() macro for my lazy ass.
     */
    json_object_set_new(j, "id", json_string(c->id));
    json_object_set_new(j, "type", json_integer(c->type));

    /* TODO: tribool to see if it was actually set, or assume "false"
     *       is a sane default and continue on.
     */
    json_object_set_new(j, "nsfw", json_boolean(c->nsfw));

    if (c->guild_id != NULL) {
        json_object_set_new(j, "guild_id", json_string(c->guild_id));
    }

    if (c->name != NULL) {
        json_object_set_new(j, "name", json_string(c->name));
    }

    if (c->last_message_id != NULL) {
        json_object_set_new(j, "last_message_id",
                            json_string(c->last_message_id));
    }

    if (c->owner_id != NULL) {
        json_object_set_new(j, "owner_id", json_string(c->owner_id));
    }

    if (c->parent_id != NULL) {
        json_object_set_new(j, "parent_id", json_string(c->parent_id));
    }

    if (c->application_id != NULL) {
        json_object_set_new(j, "application_id",
                            json_string(c->application_id));
    }

    if (c->recipients != NULL && c->recipients->len > 0) {
        size_t i = 0;
        json_t *arr = json_array();

        for (i = 0; i < c->recipients->len; i++) {
            dc_account_t acc = g_ptr_array_index(c->recipients, i);
            json_t *a = dc_account_to_json(acc);
            if (a != NULL) {
                json_array_append_new(arr, a);
            }
        }

        json_object_set_new(j, "recipients", arr);
    }

    return j;
}

dc_channel_type_t dc_channel_type(dc_channel_t c)
{
    return_if_true(c == NULL, -1);
    return c->type;
}

void dc_channel_set_type(dc_channel_t c, dc_channel_type_t t)
{
    return_if_true(c == NULL,);
    c->type = t;
}
