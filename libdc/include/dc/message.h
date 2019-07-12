#ifndef DC_MESSAGE_H
#define DC_MESSAGE_H

#include <stdint.h>
#include <jansson.h>

#include <dc/account.h>

struct dc_message_;
typedef struct dc_message_ *dc_message_t;

dc_message_t dc_message_new(void);
dc_message_t dc_message_new_content(char const *s, int len);
dc_message_t dc_message_from_json(json_t *j);
json_t *dc_message_to_json(dc_message_t m);

char const *dc_message_id(dc_message_t m);
char const *dc_message_channel_id(dc_message_t m);
char const *dc_message_timestamp(dc_message_t m);
char const *dc_message_content(dc_message_t m);
dc_account_t dc_message_author(dc_message_t m);

int dc_message_compare(dc_message_t *a, dc_message_t *b);

#endif
