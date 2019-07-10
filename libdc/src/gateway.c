#include <dc/gateway.h>
#include "internal.h"
#include <jansson.h>

struct dc_gateway_
{
    dc_refable_t ref;

    GPtrArray *ops;
    GPtrArray *out;
    GByteArray *buffer;

    CURL *easy;

    dc_account_t login;

    dc_gateway_event_callback_t callback;
    void *callback_data;

    uint64_t heartbeat_interval;
    time_t last_heartbeat;
};

static void dc_gateway_free(dc_gateway_t g)
{
    return_if_true(g == NULL,);

    if (g->ops != NULL) {
        g_ptr_array_unref(g->ops);
        g->ops = NULL;
    }

    if (g->out != NULL) {
        g_ptr_array_unref(g->out);
        g->out = NULL;
    }

    if (g->buffer != NULL) {
        g_byte_array_unref(g->buffer);
        g->buffer = NULL;
    }

    if (g->easy != NULL) {
        curl_easy_cleanup(g->easy);
        g->easy = NULL;
    }

    dc_unref(g->login);

    free(g);
}

dc_gateway_t dc_gateway_new(void)
{
    dc_gateway_t g = calloc(1, sizeof(struct dc_gateway_));
    return_if_true(g == NULL, NULL);

    g->ref.cleanup = (dc_cleanup_t)dc_gateway_free;

    g->ops = g_ptr_array_new_with_free_func((GDestroyNotify)json_decref);
    goto_if_true(g->ops == NULL, error);

    g->out = g_ptr_array_new_with_free_func((GDestroyNotify)json_decref);
    goto_if_true(g->out == NULL, error);

    g->buffer = g_byte_array_new();
    goto_if_true(g->buffer == NULL, error);

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

void dc_gateway_set_callback(dc_gateway_t gw, dc_gateway_event_callback_t c,
                             void *userdata)
{
    return_if_true(gw == NULL,);

    gw->callback = c;
    gw->callback_data = userdata;
}

bool dc_gateway_connect(dc_gateway_t gw)
{
    return_if_true(gw == NULL || gw->easy != NULL, true);

    char header[1000] = {0};
    size_t outlen = 0;
    int r = 0;

    gw->easy = curl_easy_init();
    goto_if_true(gw->easy == NULL, error);

    /* I had already introduced libcurl in a combination with libevent for all
     * the low level API stuff (i.e. POST/PUT/DELETE), and at the time of writing
     * the websocket code it was too late to rip it out, and replace with something
     * else (e.g. libwebsockets).
     *
     * CURL has no inbuilt way to handle websockets (yet), and thus we have to do
     * it ourselves by using CONNECT_ONLY. It works, but it is obviously a crutch.
     */

    curl_easy_setopt(gw->easy, CURLOPT_URL, DISCORD_GATEWAY);
    curl_easy_setopt(gw->easy, CURLOPT_FRESH_CONNECT, 1L);
    curl_easy_setopt(gw->easy, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
    curl_easy_setopt(gw->easy, CURLOPT_CONNECT_ONLY, 1L);

    goto_if_true(curl_easy_perform(gw->easy) != CURLE_OK, error);

    snprintf(header, sizeof(header)-1,
             "GET %s HTTP/1.1\r\n"
             "Host: %s\r\n"
             "User-Agent: %s\r\n"
             "Pragma: no-cache\r\n"
             "Cache-Control: no-cache\r\n"
             "Sec-WebSocket-Key: cbYK1Jm6cpk3Rua\r\n"
             "Sec-WebSocket-Version: 13\r\n"
             "Upgrade: websocket\r\n"
             "\r\n",
             DISCORD_GATEWAY_URL,
             DISCORD_GATEWAY_HOST,
             DISCORD_USERAGENT
        );

    r = curl_easy_send(gw->easy, header, strlen(header), &outlen);
    goto_if_true(r != CURLE_OK || outlen != strlen(header), error);

    do {
        r = curl_easy_recv(gw->easy, header, sizeof(header), &outlen);
        if (r == CURLE_OK && outlen > 0) {
            break;
        }
        if (r != CURLE_AGAIN) {
            goto error;
        }
    } while (true);

    goto_if_true(strstr(header, "HTTP/1.1 101") == NULL, error);

    return true;

error:

    curl_easy_cleanup(gw->easy);
    gw->easy = NULL;

    return false;
}

void dc_gateway_disconnect(dc_gateway_t gw)
{
    return_if_true(gw == NULL || gw->easy == NULL,);

    curl_easy_cleanup(gw->easy);
    gw->easy = NULL;
}

bool dc_gateway_connected(dc_gateway_t gw)
{
    return_if_true(gw == NULL || gw->easy == NULL, false);
    return true;
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
    dc_gateway_queue(gw, GATEWAY_OPCODE_PING, NULL);
    gw->last_heartbeat = time(NULL);
}

static void dc_gateway_queue_identify(dc_gateway_t gw)
{
    json_t *j = json_object(), *dev = json_object();
    char const *token = dc_account_token(gw->login);

    if (j == NULL || dev == NULL) {
        json_decref(j);
        json_decref(dev);
        return;
    }

    json_object_set_new(dev, "$os", json_string("linux"));
    json_object_set_new(dev, "$browser", json_string("libdc"));
    json_object_set_new(dev, "$device", json_string("libdc"));

    json_object_set_new(j, "token", json_string(token));
    json_object_set_new(j, "properties", dev);

    dc_gateway_queue(gw, GATEWAY_OPCODE_IDENTIFY, j);
}

static bool dc_gateway_handle_hello(dc_gateway_t gw, char const *s, json_t *d)
{
    json_t *val = NULL;

    val = json_object_get(d, "heartbeat_interval");
    return_if_true(val == NULL || !json_is_integer(val), false);

    /* send an identify first
     */
    dc_gateway_queue_identify(gw);

    gw->heartbeat_interval = json_integer_value(val);
    dc_gateway_queue_heartbeat(gw);

    return true;
}

static bool dc_gateway_handle_update(dc_gateway_t gw, char const *s, json_t *d)
{
    /* TODO
     */
    return true;
}

static bool dc_gateway_handle_event(dc_gateway_t gw, char const *s, json_t *d)
{
    dc_event_t e = dc_event_new(s, d);

    if (gw->callback != NULL && e != NULL) {
        gw->callback(gw, e, gw->callback_data);
    }

    dc_unref(e);

    return true;
}

static bool dc_gateway_handle_op(dc_gateway_t gw, json_t *j)
{
    json_t *val = NULL;
    dc_gateway_opcode_t op = 0;
    char const *s = NULL;

    val = json_object_get(j, "op");
    return_if_true(val == NULL || !json_is_integer(val), false);
    op = (dc_gateway_opcode_t)json_integer_value(val);

    val = json_object_get(j, "t");
    if (val != NULL && json_is_string(val)) {
        s = json_string_value(val);
    }

    val = json_object_get(j, "d");
    return_if_true(val == NULL || !json_is_object(val), false);

    switch (op) {
    case GATEWAY_OPCODE_EVENT: dc_gateway_handle_event(gw, s, val); break;
    case GATEWAY_OPCODE_HELLO: dc_gateway_handle_hello(gw, s, val); break;
    case GATEWAY_OPCODE_UPDATE: dc_gateway_handle_update(gw, s, val); break;
    case GATEWAY_OPCODE_PONG: break;
    default: break;
    }

    return true;
}

static void dc_gateway_process_read(dc_gateway_t gw)
{
    int ret = 0;
    char buf[100] = {0};
    size_t outlen = 0;

    return_if_true(gw->easy == NULL,);

    do {
        ret = curl_easy_recv(gw->easy, buf, sizeof(buf), &outlen);
        if (ret == CURLE_OK && outlen > 0) {
            g_byte_array_append(gw->buffer, (uint8_t const*)buf, outlen);
        }
    } while (ret == CURLE_OK && outlen > 0);
}

static void dc_gateway_process_frame(dc_gateway_t gw)
{
    size_t ret = 0;
    uint8_t *data = NULL;
    size_t datalen = 0;
    uint8_t type = 0;

    ret = dc_gateway_parseframe(gw->buffer->data, gw->buffer->len,
                                &type, &data, &datalen
        );
    return_if_true(ret == 0,);

    g_byte_array_remove_range(gw->buffer, 0, ret);

    switch (type) {
    case GATEWAY_FRAME_TEXT_DATA:
    {
        json_t *j = NULL;

        j = json_loadb((char const*)data, datalen,
                       JSON_DISABLE_EOF_CHECK, NULL);
        if (j != NULL) {
            g_ptr_array_add(gw->ops, j);
        }
    } break;

    case GATEWAY_FRAME_DISCONNECT:
    {
        dc_gateway_disconnect(gw);
    } break;

    }

    free(data);
    data = NULL;
    datalen = 0;
}

static void dc_gateway_process_in(dc_gateway_t gw)
{
    while (gw->ops->len > 0) {
        json_t *j = g_ptr_array_index(gw->ops, 0);
        dc_gateway_handle_op(gw, j);
        g_ptr_array_remove_index(gw->ops, 0);
    }
}

static bool dc_gateway_process_out(dc_gateway_t gw, json_t *j)
{
    char *str = NULL;
    uint8_t *mask = NULL;
    size_t outlen = 0, outlen2 = 0;
    int ret = 0;
    bool r = false;

    str = json_dumps(j, JSON_COMPACT);
    goto_if_true(str == NULL, cleanup);

    mask = dc_gateway_makeframe((uint8_t const *)str, strlen(str),
                                GATEWAY_FRAME_TEXT_DATA,
                                &outlen
        );
    goto_if_true(mask == NULL, cleanup);

    ret = curl_easy_send(gw->easy, mask, outlen, &outlen2);
    goto_if_true(ret != CURLE_OK || outlen2 != outlen, cleanup);

    r = true;

cleanup:

    free(str);
    str = NULL;

    free(mask);
    mask = NULL;

    return r;
}

void dc_gateway_process(dc_gateway_t gw)
{
    time_t diff = 0;

    if (!dc_gateway_connected(gw)) {
        return;
    }

    if (gw->heartbeat_interval > 0) {
        diff = time(NULL) - gw->last_heartbeat;
        if (diff >= (gw->heartbeat_interval / 1000)) {
            dc_gateway_queue_heartbeat(gw);
        }
    }

    dc_gateway_process_read(gw);

    if (gw->buffer->len > 0) {
        dc_gateway_process_frame(gw);
        if (!dc_gateway_connected(gw)) {
            return;
        }
    }

    if (gw->ops->len > 0) {
        dc_gateway_process_in(gw);
    }

    while (gw->out->len > 0) {
        json_t *j = g_ptr_array_index(gw->out, 0);
        dc_gateway_process_out(gw, j);
        g_ptr_array_remove_index(gw->out, 0);
    }
}
