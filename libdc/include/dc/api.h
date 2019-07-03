#ifndef NCDC_API_H
#define NCDC_API_H

#include <dc/apisync.h>
#include <dc/account.h>
#include <dc/guild.h>

#include <stdbool.h>

#include <jansson.h>
#include <event.h>
#include <glib.h>

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
 * Fetch a list of friends of the login account "login". The friends are stored
 * within the login object.
 */
bool dc_api_get_friends(dc_api_t api, dc_account_t login);

/**
 * Add a given account as a friend to the friends list
 */
bool dc_api_add_friend(dc_api_t api, dc_account_t login, dc_account_t friend);

#endif
