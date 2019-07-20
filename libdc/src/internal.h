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

#ifndef DC_INTERNAL_H
#define DC_INTERNAL_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#include <pthread.h>

#include <curl/curl.h>

#include <jansson.h>

#include <glib.h>

#include <event.h>
#include <event2/thread.h>

#include <dc/util.h>
#include <dc/refable.h>
#include <dc/account.h>

/* For internal debugging only. Please note that the client will then begin to
 * write text files everywhere filled with all sorts of private information. Use
 * with caution.
 */
/*#define DEBUG*/

#define return_if_true(v,r) do { if (v) return r; } while(0)
#define goto_if_true(v,l) do { if (v) goto l; } while(0)
#define continue_if_true(v) if (v) continue

#define TOKEN(l) (dc_account_token(l))

#define DISCORD_URL          "https://discordapp.com/api/v6"
#define DISCORD_GATEWAY_URL  "/?encoding=json&v=6"
#define DISCORD_GATEWAY_HOST "gateway.discord.gg"
#define DISCORD_GATEWAY      "https://" DISCORD_GATEWAY_HOST DISCORD_GATEWAY_URL

#define DISCORD_USERAGENT "Mozilla/5.0 (X11; Linux x86_64; rv:67.0) Gecko/20100101 Firefox/67.0"

#endif
