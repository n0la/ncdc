#ifndef DC_SESSION_H
#define DC_SESSION_H

#include <stdint.h>

#include <dc/api.h>
#include <dc/loop.h>
#include <dc/account.h>
#include <dc/channel.h>
#include <dc/gateway.h>

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

bool dc_session_has_token(dc_session_t s);

/**
 * Returns the currently logged in user. Which is often called "@me" in
 * Discord API.
 */
dc_account_t dc_session_me(dc_session_t s);

/**
 * access to the internal account cache
 */
void dc_session_add_account(dc_session_t s, dc_account_t u);

/**
 * access to the internal channel cache
 */
void dc_session_add_channel(dc_session_t s, dc_channel_t u);

/**
 * comparision functions for sorting, and finding
 */
bool dc_session_equal_me(dc_session_t s, dc_account_t a);
bool dc_session_equal_me_fullname(dc_session_t s, char const *a);

#endif
