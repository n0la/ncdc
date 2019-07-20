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

#ifndef DC_CHANNEL_H
#define DC_CHANNEL_H

#include <stdint.h>
#include <jansson.h>

#include <dc/account.h>
#include <dc/message.h>

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
    CHANNEL_TYPE_DM = 1,

    /* A guild voice channel
     */
    CHANNEL_TYPE_GUILD_VOICE = 2,

    /* Group direct message channel 1:N communication
     */
    CHANNEL_TYPE_GROUP_DM = 3,

    /* Category within a GUILD
     */
    CHANNEL_TYPE_GUILD_CATEGORY = 4,

    /* News channel
     */
    CHANNEL_TYPE_GUILD_NEWS = 5,

    /* Guild store, no idea what this is
     */
    CHANNEL_TYPE_GUILD_STORE = 6,
} dc_channel_type_t;

struct dc_channel_;
typedef struct dc_channel_ *dc_channel_t;

dc_channel_t dc_channel_new(void);
dc_channel_t dc_channel_from_json(json_t *j);

char const *dc_channel_id(dc_channel_t c);
char const *dc_channel_name(dc_channel_t c);
char const *dc_channel_parent_id(dc_channel_t c);

dc_channel_type_t dc_channel_type(dc_channel_t c);
bool dc_channel_is_dm(dc_channel_t c);
void dc_channel_set_type(dc_channel_t c, dc_channel_type_t t);

size_t dc_channel_recipients(dc_channel_t c);
void dc_channel_add_recipient(dc_channel_t c, dc_account_t a);
dc_account_t dc_channel_nth_recipient(dc_channel_t c, size_t i);
bool dc_channel_has_recipient(dc_channel_t c, dc_account_t a);

size_t dc_channel_messages(dc_channel_t c);
dc_message_t dc_channel_nth_message(dc_channel_t c, size_t i);
void dc_channel_add_messages(dc_channel_t c, dc_message_t *m, size_t s);

bool dc_channel_compare(dc_channel_t a, dc_channel_t b);

bool dc_channel_has_new_messages(dc_channel_t c);
void dc_channel_mark_read(dc_channel_t c);

#endif
