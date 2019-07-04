#ifndef DC_CHANNEL_H
#define DC_CHANNEL_H

#include <stdint.h>
#include <jansson.h>

/**
 * A discord channel. Exactly what it says on the tin. A place where one
 * or more guardsmen can exchange Slaaneshi heresy without their commissars
 * finding out about it. What's important here is that there are channel
 * types, and that each channel has various recipients.
 */

typedef enum {
    /* standard text channel in a guild
     */
    CHANNEL_TYPE_GUILD_TEXT = 0,

    /* A direct message channel for 1:1 communication
     */
    CHANNEL_TYPE_DM_TEXT,

    /* A guild voice channel
     */
    CHANNEL_TYPE_GUILD_VOICE,

    /* Group direct message channel 1:N communication
     */
    CHANNEL_TYPE_GROUP_DM,

    /* Category within a GUILD
     */
    CHANNEL_TYPE_GUILD_CATEGORY,

    /* News channel
     */
    CHANNEL_TYPE_GUILD_NEWS,

    /* Guild store, no idea what this is
     */
    CHANNEL_TYPE_GUILD_STORE,
} dc_channel_type_t;

struct dc_channel_;
typedef struct dc_channel_ *dc_channel_t;

dc_channel_t dc_channel_new(void);
dc_channel_t dc_channel_from_json(json_t *j);

dc_channel_type_t dc_channel_type(dc_channel_t c);
void dc_channel_set_type(dc_channel_t c, dc_channel_type_t t);

#endif
