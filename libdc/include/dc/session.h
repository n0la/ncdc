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
 * Queue API. If you enable queuing the session will keep the events from the
 * web socket around for you to handle. Please note that all internal states
 * will already have been upgraded, and changed once you pull these events
 * from the queue. Also note that if you enable queuing but never pull these
 * events from the queue they will pile up, and use a lot of memory.
 *
 * If you disable queuing then the queue is deleted, and no more events are
 * placed into the queue.
 *
 * dc_session_pop_event() will remove an event from the queue for you to handle.
 * You will have to call dc_unref() on it yourself to cleanup any internal data
 * of the event. It will return NULL if no event is in the queue.
 */
void dc_session_enable_queue(dc_session_t s, bool enable);
dc_event_t dc_session_pop_event(dc_session_t s);

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
