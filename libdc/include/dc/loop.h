#ifndef DC_LOOP_H
#define DC_LOOP_H

#include <dc/api.h>

#include <event.h>
#include <curl/curl.h>

#include <stdint.h>

struct dc_loop_;
typedef struct dc_loop_ *dc_loop_t;

/**
 * A simple CURLM <--> libevent2 loop and handler if you don't want
 * to bother rolling your own.
 */
dc_loop_t dc_loop_new(void);

/**
 * If you already have either the CURL multi handle, or the event
 * base handle, use this function. Either can be NULL. If both are
 * NULL both are allocated for you.
 */
dc_loop_t dc_loop_new_full(struct event_base *base, CURLM *multi);

/**
 * Returns the CURL multi handle in use by this loop.
 */
CURLM *dc_loop_curl(dc_loop_t l);

/**
 * Returns the event base used by this loop.
 */
struct event_base *dc_loop_event_base(dc_loop_t l);

/**
 * Add an API handle that this loop should feed.
 */
void dc_loop_add_api(dc_loop_t loop, dc_api_t api);

/**
 * Loop once, and process one message in the queues of the event
 * base, and one message from the queue of the CURL multi events.
 */
bool dc_loop_once(dc_loop_t l);

#endif
