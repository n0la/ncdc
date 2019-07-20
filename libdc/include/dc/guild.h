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
