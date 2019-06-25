#ifndef DC_GUILD_H
#define DC_GUILD_H

/* Discords version of groups or chat servers
 */
struct dc_guild_;
typedef struct dc_guild_ *dc_guild_t;

dc_guild_t dc_guild_new(void);

char const *dc_guild_name(dc_guild_t d);
void dc_guild_set_name(dc_guild_t d, char const *val);

char const *dc_guild_id(dc_guild_t d);
void dc_guild_set_id(dc_guild_t d, char const *val);

#endif
