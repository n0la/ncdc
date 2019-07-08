#include <dc/gateway.h>
#include "internal.h"
#include <jansson.h>

struct dc_gateway_
{
    dc_refable_t ref;

    GPtrArray *ops;
    GPtrArray *out;
    GByteArray *buffer;

    CURLM *multi;
    CURL *easy;
    struct curl_slist *slist;

    dc_account_t login;

    uint64_t heartbeat_interval;
    time_t last_heartbeat;
};

static void dc_gateway_free(dc_gateway_t g)
{
    return_if_true(g == NULL,);

    if (g->buffer != NULL) {
        g_byte_array_unref(g->buffer);
        g->buffer = NULL;
    }

    if (g->ops != NULL) {
        g_ptr_array_unref(g->ops);
        g->ops = NULL;
    }

    if (g->out != NULL) {
        g_ptr_array_unref(g->out);
        g->out = NULL;
    }

    if (g->easy != NULL) {
        curl_multi_remove_handle(g->multi, g->easy);
        curl_easy_cleanup(g->easy);
        g->easy = NULL;
    }

    if (g->slist != NULL) {
        curl_slist_free_all(g->slist);
        g->slist = NULL;
    }

    dc_unref(g->login);

    free(g);
}

dc_gateway_t dc_gateway_new(void)
{
    dc_gateway_t g = calloc(1, sizeof(struct dc_gateway_));
    return_if_true(g == NULL, NULL);

    g->ref.cleanup = (dc_cleanup_t)dc_gateway_free;

    g->buffer = g_byte_array_new();
    goto_if_true(g->buffer == NULL, error);

    g->ops = g_ptr_array_new_with_free_func((GDestroyNotify)json_decref);
    goto_if_true(g->ops == NULL, error);

    g->out = g_ptr_array_new_with_free_func((GDestroyNotify)json_decref);
    goto_if_true(g->out == NULL, error);

    g->slist = curl_slist_append(NULL, "");
    goto_if_true(g->slist == NULL, error);

    return dc_ref(g);

error:

    dc_gateway_free(g);
    return NULL;
}

void dc_gateway_set_login(dc_gateway_t gw, dc_account_t login)
{
    return_if_true(gw == NULL,);
    gw->login = dc_ref(login);
}

void dc_gateway_set_curl(dc_gateway_t gw, CURLM *multi, CURL *easy)
{
    return_if_true(gw == NULL,);
    gw->multi = multi;
    gw->easy = easy;
}

CURL *dc_gateway_curl(dc_gateway_t gw)
{
    return_if_true(gw == NULL, NULL);
    return gw->easy;
}

struct curl_slist * dc_gateway_slist(dc_gateway_t gw)
{
    return_if_true(gw == NULL, NULL);
    return gw->slist;
}

size_t dc_gateway_writefunc(char *ptr, size_t sz, size_t nmemb, void *data)
{
    dc_gateway_t g = (dc_gateway_t)data;
    json_t *j = NULL;
    size_t i = 0;

    FILE *f = fopen("websocket.txt", "a+");
    fprintf(f, ">> ");
    fwrite(ptr, sz, nmemb, f);
    fprintf(f, "\n");
    fclose(f);

    for (i = 0; *ptr != '{' && i < (sz *nmemb); ptr++, i++)
        ;

    if (i < (sz * nmemb)) {
        j = json_loadb(ptr, (sz*nmemb) - i, JSON_DISABLE_EOF_CHECK, NULL);
        if (j != NULL) {
            g_ptr_array_add(g->ops, j);
        }
    }

    return sz * nmemb;
}

static json_t *dc_gateway_answer(dc_gateway_t gw)
{
    json_t *j = NULL;
    char const *token = NULL;

    j = json_object();
    return_if_true(j == NULL, NULL);

    token = dc_account_token(gw->login);
    if (token != NULL) {
        json_object_set_new(j, "token", json_string(token));
    }

    return j;
}

static void dc_gateway_queue(dc_gateway_t gw, int code, json_t *d)
{
    json_t *j = NULL;

    j = json_object();
    return_if_true(j == NULL,);

    if (d == NULL) {
        d = dc_gateway_answer(gw);
    }

    json_object_set_new(j, "t", json_null());
    json_object_set_new(j, "s", json_null());
    json_object_set_new(j, "d", d);
    json_object_set_new(j, "op", json_integer(code));

    g_ptr_array_add(gw->out, j);
}

static void dc_gateway_queue_heartbeat(dc_gateway_t gw)
{
    dc_gateway_queue(gw, GATEWAY_OPCODE_HEARTBEAT, NULL);
    gw->last_heartbeat = time(NULL);
}

static bool dc_gateway_handle_hello(dc_gateway_t gw, json_t *d)
{
    json_t *val = NULL;

    val = json_object_get(d, "heartbeat_interval");
    return_if_true(val == NULL || !json_is_integer(val), false);

    /* send an identify first
     */
    dc_gateway_queue(gw, GATEWAY_OPCODE_IDENTIFY, NULL);

    gw->heartbeat_interval = json_integer_value(val);
    dc_gateway_queue_heartbeat(gw);

    return true;
}

static bool dc_gateway_handle_op(dc_gateway_t gw, json_t *j)
{
    json_t *val = NULL;
    dc_gateway_opcode_t op = 0;

    val = json_object_get(j, "op");
    return_if_true(val == NULL || !json_is_integer(val), false);
    op = (dc_gateway_opcode_t)json_integer_value(val);

    val = json_object_get(j, "d");
    return_if_true(val == NULL || !json_is_object(val), false);

    switch (op) {
    case GATEWAY_OPCODE_HELLO: dc_gateway_handle_hello(gw, val); break;
    default: break;
    }

    return true;
}

#if 0
static void dc_gateway_process_read(dc_gateway_t gw)
{
    char buf[100] = {0};
    size_t read = 0;
    int ret = 0;
    FILE *f = NULL;
    json_t *j = NULL;
    size_t where = 0;

    ret = curl_easy_recv(gw->easy, &buf, sizeof(buf), &read);
    return_if_true(ret != CURLE_OK,);

    g_byte_array_append(gw->buffer, (uint8_t const*)buf, read);

    f = fmemopen(gw->buffer->data, gw->buffer->len, "r");
    return_if_true(f == NULL,);

    j = json_loadf(f, JSON_DISABLE_EOF_CHECK, NULL);
    where = ftell(f);

    fclose(f);
    f = NULL;

    if (j != NULL) {
        g_ptr_array_add(gw->ops, j);
        g_byte_array_remove_range(gw->buffer, 0, where);
    }
}
#endif

static void dc_gateway_process_in(dc_gateway_t gw)
{
    while (gw->ops->len > 0) {
        json_t *j = g_ptr_array_index(gw->ops, 0);
        dc_gateway_handle_op(gw, j);
        g_ptr_array_remove_index(gw->ops, 0);
    }
}

static void dc_gateway_process_out(dc_gateway_t gw)
{
    char *str = NULL;
    size_t slen = 0, outlen = 0, sent = 0;
    uint8_t *mask = NULL;

    while (gw->out->len > 0) {
        json_t *j = g_ptr_array_index(gw->out, 0);

        str = json_dumps(j, JSON_COMPACT);

        if (str != NULL) {
            slen = strlen(str);
            mask = dc_gateway_makeframe((uint8_t const *)str, slen, 0, &outlen);
            curl_easy_send(gw->easy, mask, outlen, &sent);

            FILE *f = fopen("websocket.txt", "a+");
            fprintf(f, "<< %s\n", str);
            fclose(f);
        }

        free(str);
        free(mask);

        g_ptr_array_remove_index(gw->out, 0);
    }
}

void dc_gateway_process(dc_gateway_t gw)
{
    time_t diff = 0;

    if (gw->heartbeat_interval > 0) {
        diff = time(NULL) - gw->last_heartbeat;
        if (diff >= (gw->heartbeat_interval / 1000)) {
            dc_gateway_queue_heartbeat(gw);
        }
    }

    //dc_gateway_process_read(gw);
    dc_gateway_process_in(gw);
    dc_gateway_process_out(gw);
}
