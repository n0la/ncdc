/*
 * Part of ncdc - a discord client for the console
 * Copyright (C) 2019 Florian Stinglmayr <fstinglmayr@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef NCDC_API_H
#define NCDC_API_H

#include <dc/apisync.h>
#include <dc/account.h>
#include <dc/guild.h>
#include <dc/channel.h>
#include <dc/gateway.h>

#include <stdbool.h>

#include <jansson.h>
#include <event.h>
#include <glib.h>

#define DC_API_USER_STATUS_ONLINE    "online"
#define DC_API_USER_STATUS_IDLE      "idle"
#define DC_API_USER_STATUS_DND       "dnd"
#define DC_API_USER_STATUS_INVISIBLE "invisible"

struct dc_api_;
typedef struct dc_api_ *dc_api_t;

dc_api_t dc_api_new(void);

void dc_api_set_curl_multi(dc_api_t api, CURLM *curl);
void dc_api_set_event_base(dc_api_t api, struct event_base *base);

/* call this function in case the MULTI has told us that some
 * transfer has finished.
 */
void dc_api_signal(dc_api_t api, CURL *easy, int code);

/* internal curl stuff
 */
bool dc_api_error(json_t *j, int *code, char const **message);
dc_api_sync_t dc_api_call(dc_api_t api, char const *token,
                          char const *verb, char const *method,
                          json_t *j);
json_t *dc_api_call_sync(dc_api_t api, char const *token,
                         char const *verb, char const *method,
                         json_t *j);

/**
 * Authenticate a given user account. The user account should have
 * email, and password set. If the auth succeeds the account will have
 * a login token, and will from now on be the "login account". You
 * will have to pass that account to various other functions for
 * authentication.
 */
bool dc_api_authenticate(dc_api_t api, dc_account_t account);

/**
 * Log the account in. Will first call dc_api_authenticate(), then
 * dc_api_get_userinfo(), then dc_api_get_friends(). If
 * any of these steps fail, it returns false.
 */
bool dc_api_login(dc_api_t api, dc_account_t account);

/**
 * Inverse of dc_api_authenticate(). Logs the given user out, destroying the
 * login token in the process.
 */
bool dc_api_logout(dc_api_t api, dc_account_t account);

/**
 * Retrieve basic user information for the given account. The first
 * parameter is the user account holding login info, while the second
 * is the account you wish to retrieve information about. If you wish
 * to retrieve information about the login user, simply pass the account
 * twice.
 */
bool dc_api_get_userinfo(dc_api_t api, dc_account_t login,
                         dc_account_t user);

/**
 * Fetch a list of guilds fro the specified login user. Warning if you
 * unref the pointer array, you will also unref all the dc_guild_t objects.
 */
bool dc_api_get_userguilds(dc_api_t api, dc_account_t login,
                           GPtrArray **guilds);

/**
 * Set the online status of the currently logged in user "login". "status" must
 * be one of the valid string macros defined in this header, or the function
 * fails.
 */
bool dc_api_set_user_status(dc_api_t api, dc_account_t login,
                            char const *status);

/**
 * Create a 1:1 or 1:N DM channel with the given recipients. The recipients must
 * have their ID (snowflake) set. Returns the new channel, complete with ID, and
 * all in "channel". Note that the "login" user is automatically added to the DM
 * session, so it is not needed to add him to recipients.
 */
bool dc_api_create_channel(dc_api_t api, dc_account_t login,
                           dc_account_t *recipients, size_t nrecp,
                           dc_channel_t *channel);

/**
 * Fetch 50 messages for the given channel.
 */
bool dc_api_get_messages(dc_api_t api, dc_account_t login, dc_channel_t c);

/**
 * post a message to the given channel
 */
bool dc_api_post_message(dc_api_t api, dc_account_t login,
                         dc_channel_t c, dc_message_t m);

/**
 * "ack" a channel, meaning that you have read it its contents. You must provide
 * a message, so that discord knows which message was the last you read.
 */
bool dc_api_channel_ack(dc_api_t api, dc_account_t login,
                        dc_channel_t c, dc_message_t msg);

/**
 * Fetch a list of friends of the login account "login". The friends are stored
 * within the login object.
 */
bool dc_api_get_friends(dc_api_t api, dc_account_t login);

/**
 * Add a given account as a friend to the friends list
 */
bool dc_api_add_friend(dc_api_t api, dc_account_t login, dc_account_t friend);

/**
 * Remove a given account as a friend to the friends list. Warning: The
 * account ID (aka account snowflake) is required to perform this operation,
 * so you cannot just do dc_account_from_fullname(). Suggestion: use an object
 * from the actual friends list of the account login.
 */
bool dc_api_remove_friend(dc_api_t api, dc_account_t login,
                          dc_account_t friend);

/**
 * Accepts someone who has sent a friend request to you, as a friend. Warning:
 * The object "friend" requires an account ID (aka snowflake) for this method
 * to work. You should take this object perhaps from the "login"'s friend list.
 */
bool dc_api_accept_friend(dc_api_t api, dc_account_t login,
                          dc_account_t friend);

#endif
