#ifndef DC_SESSION_H
#define DC_SESSION_H

#include <stdint.h>

#include <dc/api.h>
#include <dc/loop.h>
#include <dc/account.h>
#include <dc/channel.h>
#include <dc/gateway.h>
#include <dc/guild.h>

/**
 * A session object will contain all information gathered after a user
 * authenticated to the services. It is the "god emperor object" of libdiscord,
 * that stores, caches, and provides all information after a log in. From
 * available channel objects, account objects of friends, and guild mates,
 * to a list of available guilds.
 *
 * If you consider writing a fully fledged client, or a very sophisticated
 * bot, you should consider using this session object in your application.
 *
 * Please note that the session object attempts to use as little API calls as
 * necessary, relying heavily on data provided by the gateway. So it may take
 * a bit before appropriate information (i.e. friends) have been loaded.
 */

struct dc_session_;
typedef struct dc_session_ *dc_session_t;

/**
 * Creates a new session that will attach itself to the given loop.
 */
dc_session_t dc_session_new(dc_loop_t loop);

/**
 * Logs the given user out, and clears all internal data.
 */
bool dc_session_logout(dc_session_t s);

/**
 * Logs the given user into the system, starts a websocket and begins to
 * collect information about the given login user (channels, guilds,
 * friends).
 */
bool dc_session_login(dc_session_t s, dc_account_t login);

/**
 * Returns true if the session is ready, i.e. a login has been performed
 * and the READY event has been parsed from the websocket.
 */
bool dc_session_is_ready(dc_session_t s);

bool dc_session_has_token(dc_session_t s);

/**
 * Returns the currently logged in user. Which is often called "@me" in
 * Discord API.
 */
dc_account_t dc_session_me(dc_session_t s);

/**
 * Return the API handle in use by the session. Do not unref the reference
 * and if you need it for something else, dc_ref() it yourself.
 */
dc_api_t dc_session_api(dc_session_t s);

/**
 * access to the internal account cache
 */
void dc_session_add_account(dc_session_t s, dc_account_t u);
void dc_session_add_account_new(dc_session_t s, dc_account_t u);
dc_account_t dc_session_account_fullname(dc_session_t s, char const *f);

/**
 * Adds a new channel to the internal cache. _new does the same, but doesn't
 * increase the reference count.
 */
void dc_session_add_channel(dc_session_t s, dc_channel_t u);
void dc_session_add_channel_new(dc_session_t s, dc_channel_t u);

dc_channel_t dc_session_channel_by_id(dc_session_t s, char const *snowflake);

/**
 * Creates a new channel, or returns an existing channel if a channel with
 * these recipients already exists.
 * Warning: You must dc_ref() the return object if you need it beyond the
 * life-span of the session.
 */
dc_channel_t dc_session_make_channel(dc_session_t s, dc_account_t *r,
                                     size_t n);

/**
 * Finds a channel object by that match the given recipients.
 * Warning: You must dc_ref() the return object if you need it beyond the
 * life-span of the session.
 */
dc_channel_t dc_session_channel_recipients(dc_session_t s,
                                           dc_account_t *r, size_t sz);

/**
 * Add a guild to be managed by this session. _new does the same, but doesn't
 * ref the given reference.
 */
void dc_session_add_guild(dc_session_t s, dc_guild_t g);
void dc_session_add_guild_new(dc_session_t s, dc_guild_t g);
GHashTable *dc_session_guilds(dc_session_t s);
dc_guild_t dc_session_guild_by_name(dc_session_t s, char const *name);

/**
 * comparision functions for sorting, and finding
 */
bool dc_session_equal_me(dc_session_t s, dc_account_t a);
bool dc_session_equal_me_fullname(dc_session_t s, char const *a);

#endif
