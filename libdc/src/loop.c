#include <dc/loop.h>
#include <dc/refable.h>

#include "internal.h"

struct dc_loop_
{
    dc_refable_t ref;

    struct event_base *base;
    struct event *timer;
    CURLM *multi;

    bool base_owner;
    bool multi_owner;

    GPtrArray *apis;
    GPtrArray *gateways;
};

static void dc_loop_free(dc_loop_t p)
{
    return_if_true(p == NULL,);

    if (p->timer != NULL) {
        evtimer_del(p->timer);
        event_free(p->timer);
        p->timer = NULL;
    }

    if (p->multi_owner && p->multi != NULL) {
        curl_multi_cleanup(p->multi);
        p->multi = NULL;
    }

    if (p->base_owner && p->base != NULL) {
        event_base_free(p->base);
        p->base = NULL;
    }

    if (p->apis != NULL) {
        g_ptr_array_unref(p->apis);
        p->apis = NULL;
    }

    if (p->gateways != NULL) {
        g_ptr_array_unref(p->gateways);
        p->gateways = NULL;
    }

    free(p);
}

static void socket_handler(int sock, short what, void *data)
{
    int unused = 0;
    dc_loop_t loop = (dc_loop_t)data;

    if ((what & EV_READ) == EV_READ) {
        curl_multi_socket_action(loop->multi, sock, CURL_CSELECT_IN, &unused);
    } else if ((what & EV_WRITE) == EV_WRITE) {
        curl_multi_socket_action(loop->multi, sock, CURL_CSELECT_OUT, &unused);
    }
}

static int
mcurl_handler(CURL *easy, curl_socket_t s, int what, void *userp, void *socketp)
{
    struct event *event = (struct event *)socketp;
    dc_loop_t loop = (dc_loop_t)userp;

    for (size_t i = 0; i < loop->gateways->len; i++) {
        dc_gateway_t gw = g_ptr_array_index(loop->gateways, i);

        if (dc_gateway_curl(gw) == easy) {
            printf("gateway event: %s: %s%s\n",
                   (what == CURL_POLL_REMOVE ? "remove" : "add"),
                   ((what & CURL_POLL_IN) ? "r" : ""),
                   ((what & CURL_POLL_OUT) ? "w": "")
                );
        }
    }

    if (what == CURL_POLL_REMOVE) {
        if (event != NULL) {
            event_del(event);
            event_free(event);
            curl_multi_assign(loop->multi, s, NULL);
        }
    } else {
        int stat =
            ((what & CURL_POLL_IN) ? EV_READ : 0) |
            ((what & CURL_POLL_OUT) ? EV_WRITE : 0) |
            EV_PERSIST
            ;

        if (event == NULL) {
            event = event_new(loop->base, s, stat, socket_handler, loop);
            if (event == NULL) {
                return 0;
            }
            curl_multi_assign(loop->multi, s, event);
        } else {
            event_del(event);
            event_assign(event, loop->base, s, stat, socket_handler,loop);
            event_add(event, NULL);
        }
    }

    return 0;
}

static void timer_handler(int sock, short what, void *data)
{
    int running = 0;
    dc_loop_t loop = (dc_loop_t)data;
    curl_multi_socket_action(loop->multi, CURL_SOCKET_TIMEOUT, 0, &running);
}

static int mcurl_timer(CURLM *curl, long timeout, void *ptr)
{
    int running = 0;
    struct timeval tm;
    dc_loop_t loop = (dc_loop_t)ptr;

    if (timeout == -1) {
        evtimer_del(loop->timer);
    } else if (timeout == 0) {
        curl_multi_socket_action(loop->multi, CURL_SOCKET_TIMEOUT, 0, &running);
    } else if (timeout > 0) {
        tm.tv_sec = timeout / 1000;
        tm.tv_usec = (timeout % 1000) * 1000;
        evtimer_add(loop->timer, &tm);
    }

    return 0;
}

dc_loop_t dc_loop_new(void)
{
    return dc_loop_new_full(NULL, NULL);
}

dc_loop_t dc_loop_new_full(struct event_base *base, CURLM *multi)
{
    dc_loop_t ptr = calloc(1, sizeof(struct dc_loop_));
    return_if_true(ptr == NULL, NULL);

    ptr->ref.cleanup = (dc_cleanup_t)dc_loop_free;

    if (base != NULL) {
        ptr->base = base;
        ptr->base_owner = false;
    } else {
        ptr->base = event_base_new();
        goto_if_true(ptr->base == NULL, fail);
        ptr->base_owner = true;
    }

    if (multi != NULL) {
        ptr->multi = multi;
        ptr->multi_owner = false;
    } else {
        ptr->multi = curl_multi_init();
        goto_if_true(ptr->multi == NULL, fail);
        ptr->multi_owner = true;
    }

    ptr->apis = g_ptr_array_new_with_free_func((GDestroyNotify)dc_unref);
    goto_if_true(ptr->apis == NULL, fail);

    ptr->gateways = g_ptr_array_new_with_free_func((GDestroyNotify)dc_unref);
    goto_if_true(ptr->gateways == NULL, fail);

    ptr->timer = evtimer_new(ptr->base, timer_handler, ptr);
    goto_if_true(ptr->timer == NULL, fail);

    curl_multi_setopt(ptr->multi, CURLMOPT_SOCKETDATA, ptr);
    curl_multi_setopt(ptr->multi, CURLMOPT_SOCKETFUNCTION, mcurl_handler);

    curl_multi_setopt(ptr->multi, CURLMOPT_TIMERDATA, ptr);
    curl_multi_setopt(ptr->multi, CURLMOPT_TIMERFUNCTION, mcurl_timer);

    return dc_ref(ptr);

fail:

    dc_loop_free(ptr);
    return NULL;
}

CURLM *dc_loop_curl(dc_loop_t l)
{
    return_if_true(l == NULL, NULL);
    return l->multi;
}

struct event_base *dc_loop_event_base(dc_loop_t l)
{
    return_if_true(l == NULL, NULL);
    return l->base;
}

void dc_loop_add_api(dc_loop_t l, dc_api_t a)
{
    return_if_true(l == NULL || a == NULL,);
    dc_api_t p = dc_ref(a);

    dc_api_set_event_base(p, l->base);
    dc_api_set_curl_multi(p, l->multi);

    g_ptr_array_add(l->apis, dc_ref(p));
}

void dc_loop_add_gateway(dc_loop_t l, dc_gateway_t gw)
{
    return_if_true(l == NULL || gw == NULL,);

    g_ptr_array_add(l->gateways, dc_ref(gw));
}

void dc_loop_abort(dc_loop_t l)
{
    return_if_true(l == NULL || l->base == NULL,);
    event_base_loopbreak(l->base);
}

bool dc_loop_once(dc_loop_t l)
{
    return_if_true(l == NULL, false);

    int ret = 0, remain = 0;
    struct CURLMsg *msg = NULL;
    size_t i = 0;

    ret = event_base_loop(l->base, EVLOOP_ONCE);
    if (ret < 0) {
        return false;
    }

    msg = curl_multi_info_read(l->multi, &remain);
    if (msg != NULL) {
        if (remain <= 0) {
            if (evtimer_pending(l->timer, NULL)) {
                evtimer_del(l->timer);
            }
        }
        if (msg->msg == CURLMSG_DONE) {
            for (i = 0; i < l->apis->len; i++) {
                dc_api_t api = g_ptr_array_index(l->apis, i);
                dc_api_signal(api, msg->easy_handle, msg->data.result);
            }
        }
    }

    for (i = 0; i < l->gateways->len; i++) {
        dc_gateway_t gw = g_ptr_array_index(l->gateways, i);
        dc_gateway_process(gw);
    }

    return true;
}
