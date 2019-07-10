#include <dc/session.h>
#include "internal.h"

struct dc_session_
{
    dc_refable_t ref;

    dc_loop_t loop;
    dc_api_t api;
    dc_account_t login;
    dc_gateway_t gateway;
};

static void dc_session_free(dc_session_t s)
{
    return_if_true(s == NULL,);

    dc_session_logout(s);

    dc_unref(s->api);
    dc_unref(s->loop);

    free(s);
}

static void dc_session_handler(dc_gateway_t gw, dc_event_t e, void *p)
{
    dc_session_t s = (dc_session_t)p;

    char *str = NULL;
    str = json_dumps(dc_event_payload(e), 0);
    FILE *f = fopen("events.txt", "a+");
    fprintf(f, "%p: %s: %s\n", s, dc_event_type(e), str);
    free(str);
    fclose(f);
}

dc_session_t dc_session_new(dc_loop_t loop)
{
    return_if_true(loop == NULL, NULL);

    dc_session_t s = calloc(1, sizeof(struct dc_session_));
    return_if_true(s == NULL, NULL);

    s->ref.cleanup = (dc_cleanup_t)dc_session_free;

    s->loop = dc_ref(loop);

    s->api = dc_api_new();
    if (s->api == NULL) {
        dc_session_free(s);
        return NULL;
    }

    dc_loop_add_api(s->loop, s->api);

    return dc_ref(s);
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
