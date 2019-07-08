#ifndef DC_GATEWAY_H
#define DC_GATEWAY_H

#include <stdint.h>
#include <stdlib.h>

#include <dc/account.h>

#include <curl/curl.h>

struct dc_gateway_;
typedef struct dc_gateway_ *dc_gateway_t;

typedef enum {
    GATEWAY_OPCODE_HEARTBEAT = 1,
    GATEWAY_OPCODE_IDENTIFY = 2,
    GATEWAY_OPCODE_HELLO = 10,
} dc_gateway_opcode_t;

dc_gateway_t dc_gateway_new(void);

void dc_gateway_set_login(dc_gateway_t gw, dc_account_t login);

/**
 * Set all required CURL handles. The object will delete the easy handle
 * and make sure it is removed from the multi handle upon unref. Do not
 * free the multi handle before you remove the gateway.
 */
void dc_gateway_set_curl(dc_gateway_t gw, CURLM *multi, CURL *easy);

CURL *dc_gateway_curl(dc_gateway_t gw);

/**
 * Returns a CURL slist that lasts as long as the handle itself lasts
 */
struct curl_slist * dc_gateway_slist(dc_gateway_t gw);

/**
 * To be used as a WRITEFUNCTION for a curl handle. Don't forget to set the
 * gateway handle as a WRITEDATA too, otherwise this will have no effect.
 */
size_t dc_gateway_writefunc(char *ptr, size_t sz, size_t nmemb, void *data);

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

#endif
