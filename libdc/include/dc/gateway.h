#ifndef DC_GATEWAY_H
#define DC_GATEWAY_H

#include <stdint.h>
#include <stdlib.h>

#include <dc/account.h>
#include <dc/event.h>

struct dc_gateway_;
typedef struct dc_gateway_ *dc_gateway_t;

/**
 * The event callback that will be called by the gateway when a new event
 * arrives. First parameter is the gateway responsible for sending this
 * event, second parameter is the event in question, third parameter is
 * user defined callback data.
 *
 * Note that the event will be allocated, and dc_unref()'d by the gateway,
 * so if you need the event around you need to dc_ref() it.
 */
typedef void (*dc_gateway_event_callback_t)(dc_gateway_t, dc_event_t, void*);

typedef enum {
    GATEWAY_OPCODE_EVENT = 0,
    GATEWAY_OPCODE_PING = 1,
    GATEWAY_OPCODE_IDENTIFY = 2,
    GATEWAY_OPCODE_UPDATE = 3,
    GATEWAY_OPCODE_HELLO = 10,
    GATEWAY_OPCODE_PONG = 11,
} dc_gateway_opcode_t;

typedef enum {
    GATEWAY_FRAME_TEXT_DATA = 129,
    GATEWAY_FRAME_DISCONNECT = 136,
    GATEWAY_FRAME_PING = 137,
    GATEWAY_FRAME_PONG = 138,
} dc_gateway_frames_t;

dc_gateway_t dc_gateway_new(void);

void dc_gateway_set_login(dc_gateway_t gw, dc_account_t login);

void dc_gateway_set_callback(dc_gateway_t gw, dc_gateway_event_callback_t c,
                             void *userdata);

/**
 * Connect the given gateway. Does nothing if the gateway is already
 * connected.
 */
bool dc_gateway_connect(dc_gateway_t gw);

/**
 * Cleans up the easy handle, and thus disconnects from the socket handle
 * immediately. After this call dc_gateway_connected() will return false.
 */
void dc_gateway_disconnect(dc_gateway_t gw);

/**
 * Returns true if the gateway is still connected.
 */
bool dc_gateway_connected(dc_gateway_t gw);

/**
 * Process the queue of data that came from the websocket. Since the
 * gateway handle is not part of whole event_base_loop() shebang, this
 * must be called individually. dc_loop_once() will do this for you, if
 * you opt to use dc_loop() (which you should).
 */
void dc_gateway_process(dc_gateway_t gw);

/**
 * utility function to make a websocket frame
 */
uint8_t *
dc_gateway_makeframe(uint8_t const *d, size_t data_len,
                     uint8_t type, size_t *outlen);

size_t
dc_gateway_parseframe(uint8_t const *data, size_t datalen,
                      uint8_t *type, uint8_t **outdata, size_t *outlen);

#endif
