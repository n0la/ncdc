#include <dc/session.h>
#include "internal.h"

struct dc_session_
{
    dc_refable_t ref;

    dc_loop_t loop;
    dc_api_t api;
    dc_account_t login;
    dc_gateway_t gateway;

    GHashTable *accounts;
    GHashTable *channels;
};

/* event handlers
 */
typedef void (*dc_session_handler_t)(dc_session_t s, dc_event_t e);
static void dc_session_handle_ready(dc_session_t s, dc_event_t e);

static dc_session_handler_t handlers[DC_EVENT_TYPE_LAST] = {
    [DC_EVENT_TYPE_UNKNOWN] = NULL,
    [DC_EVENT_TYPE_READY] = dc_session_handle_ready,
};

static void dc_session_free(dc_session_t s)
{
    return_if_true(s == NULL,);

    if (s->accounts != NULL) {
        g_hash_table_unref(s->accounts);
        s->accounts = NULL;
    }

    if (s->channels != NULL) {
        g_hash_table_unref(s->channels);
        s->channels = NULL;
    }

    dc_session_logout(s);

    dc_unref(s->api);
    dc_unref(s->loop);

    free(s);
}

static void dc_session_handle_ready(dc_session_t s, dc_event_t e)
{
    json_t *r = dc_event_payload(e);
    json_t *user = NULL;
    json_t *relationships = NULL;
    size_t idx = 0;
    json_t *c = NULL;
    json_t *channels = NULL;

    /* retrieve user information about ourselves, including snowflake,
     * discriminator, and other things
     */
    user = json_object_get(r, "user");
    if (user != NULL && json_is_object(user)) {
        dc_account_load(s->login, user);
        dc_session_add_account(s, s->login);
    }

    /* load relationships, aka friends
     */
    relationships = json_object_get(r, "relationships");
    if (relationships != NULL && json_is_array(relationships)) {
        json_array_foreach(relationships, idx, c) {
            dc_account_t u = dc_account_from_relationship(user);

            if (u == NULL) {
                continue;
            }

            dc_account_add_friend(s->login, u);
            dc_session_add_account(s, u);
        }
    }

    /* load channels
     */
    channels = json_object_get(r, "private_channels");
    if (channels != NULL && json_is_array(channels)) {
        json_array_foreach(channels, idx, c) {
            dc_channel_t chan = dc_channel_from_json(c);
            if (chan == NULL) {
                continue;
            }

            dc_session_add_channel(s, chan);
        }
    }
}

static void dc_session_handler(dc_gateway_t gw, dc_event_t e, void *p)
{
    dc_session_t s = (dc_session_t)p;
    dc_session_handler_t h = handlers[dc_event_type_code(e)];

    if (h != NULL) {
        h(s, e);
    }

    char *str = NULL;
    str = json_dumps(dc_event_payload(e), 0);
    FILE *f = fopen("events.txt", "a+");
    fprintf(f, "%s: %s\n", dc_event_type(e), str);
    free(str);
    fclose(f);
}

dc_session_t dc_session_new(dc_loop_t loop)
{
    return_if_true(loop == NULL, NULL);

    dc_session_t s = calloc(1, sizeof(struct dc_session_));
    return_if_true(s == NULL, NULL);

    s->ref.cleanup = (dc_cleanup_t)dc_session_free;

    s->accounts = g_hash_table_new_full(g_str_hash, g_str_equal,
                                        free, dc_unref
        );
    goto_if_true(s->accounts == NULL, error);

    s->channels = g_hash_table_new_full(g_str_hash, g_str_equal,
                                        free, dc_unref
        );
    goto_if_true(s->channels == NULL, error);

    s->loop = dc_ref(loop);

    s->api = dc_api_new();
    goto_if_true(s->api == NULL, error);

    dc_loop_add_api(s->loop, s->api);

    return dc_ref(s);

error:

    dc_session_free(s);
    return NULL;
}

bool dc_session_logout(dc_session_t s)
{
    return_if_true(s == NULL, false);

    if (s->login != NULL) {
        if (dc_account_has_token(s->login)) {
            dc_api_logout(s->api, s->login);
        }
        dc_unref(s->login);
        s->login = NULL;
    }

    if (s->gateway != NULL) {
        dc_loop_remove_gateway(s->loop, s->gateway);
        dc_unref(s->gateway);
        s->gateway = NULL;
    }

    return true;
}

bool dc_session_login(dc_session_t s, dc_account_t login)
{
    return_if_true(s == NULL || login == NULL, false);

    if (s->login != NULL) {
        dc_session_logout(s);
    }

    s->login = dc_ref(login);
    if (!dc_account_has_token(login)) {
        if (!dc_api_authenticate(s->api, s->login)) {
            dc_unref(s->login);
            s->login = NULL;
            return false;
        }

        s->gateway = dc_gateway_new();
        if (s->gateway == NULL) {
            dc_session_logout(s);
            return false;
        }

        dc_gateway_set_callback(s->gateway, dc_session_handler, s);
        dc_gateway_set_login(s->gateway, s->login);
        dc_loop_add_gateway(s->loop, s->gateway);
    }

    return true;
}

bool dc_session_has_token(dc_session_t s)
{
    return_if_true(s == NULL || s->login == NULL, false);
    return dc_account_has_token(s->login);
}

dc_account_t dc_session_me(dc_session_t s)
{
    return_if_true(s == NULL, NULL);
    return s->login;
}

bool dc_session_equal_me(dc_session_t s, dc_account_t a)
{
    return_if_true(s == NULL || s->login == NULL || a == NULL, false);
    return (strcmp(dc_account_fullname(s->login),
                   dc_account_fullname(a)) == 0
        );
}

bool dc_session_equal_me_fullname(dc_session_t s, char const *a)
{
    return_if_true(s == NULL || s->login == NULL || a == NULL, false);
    return (strcmp(dc_account_fullname(s->login), a) == 0);
}

void dc_session_add_account(dc_session_t s, dc_account_t u)
{
    return_if_true(s == NULL || u == NULL,);
    return_if_true(dc_account_id(u) == NULL,);

    char const *id = dc_account_id(u);

    if (!g_hash_table_contains(s->accounts, id)) {
        g_hash_table_insert(s->accounts, strdup(id), dc_ref(u));
    }
}

void dc_session_add_channel(dc_session_t s, dc_channel_t u)
{
    return_if_true(s == NULL || u == NULL,);
    return_if_true(dc_channel_id(u) == NULL,);

    char const *id = dc_channel_id(u);

    if (!g_hash_table_contains(s->channels, id)) {
        g_hash_table_insert(s->channels, strdup(id), dc_ref(u));
        /* TODO: dedup for saving storage
         */
    }
}
