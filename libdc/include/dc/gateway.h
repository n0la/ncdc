#ifndef DC_GATEWAY_H
#define DC_GATEWAY_H

#include <stdint.h>
#include <stdlib.h>

#include <dc/account.h>

#include <curl/curl.h>

struct dc_gateway_;
typedef struct dc_gateway_ *dc_gateway_t;

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
 * Process the queue of data that came from the websocket.
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
