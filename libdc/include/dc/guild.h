#ifndef DC_GUILD_H
#define DC_GUILD_H

#include <dc/channel.h>

#include <jansson.h>
#include <stdint.h>

/* Discords version of groups or chat servers
 */
struct dc_guild_;
typedef struct dc_guild_ *dc_guild_t;

dc_guild_t dc_guild_new(void);
dc_guild_t dc_guild_from_json(json_t *j);

size_t dc_guild_channels(dc_guild_t d);
dc_channel_t dc_guild_nth_channel(dc_guild_t d, size_t idx);
dc_channel_t dc_guild_channel_by_name(dc_guild_t g, char const *name);

char const *dc_guild_name(dc_guild_t d);
void dc_guild_set_name(dc_guild_t d, char const *val);

char const *dc_guild_id(dc_guild_t d);
void dc_guild_set_id(dc_guild_t d, char const *val);

#endif
