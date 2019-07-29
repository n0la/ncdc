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

#ifndef DC_MESSAGE_H
#define DC_MESSAGE_H

#include <stdint.h>
#include <jansson.h>
#include <time.h>

#include <dc/account.h>

struct dc_message_;
typedef struct dc_message_ *dc_message_t;

dc_message_t dc_message_new(void);
dc_message_t dc_message_new_content(char const *s, int len);
dc_message_t dc_message_from_json(json_t *j);
json_t *dc_message_to_json(dc_message_t m);

char const *dc_message_id(dc_message_t m);
char const *dc_message_channel_id(dc_message_t m);
char const *dc_message_timestamp(dc_message_t m);
char const *dc_message_content(dc_message_t m);
dc_account_t dc_message_author(dc_message_t m);
time_t dc_message_unix_timestamp(dc_message_t m);

int dc_message_compare(dc_message_t *a, dc_message_t *b);

#endif
