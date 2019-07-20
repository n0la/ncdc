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

#ifndef DC_API_ASYNC_H
#define DC_API_ASYNC_H

#include <curl/curl.h>

#include <stdio.h>
#include <stdbool.h>

struct dc_api_sync_;
typedef struct dc_api_sync_ *dc_api_sync_t;

dc_api_sync_t dc_api_sync_new(CURLM *curl, CURL *easy);

FILE *dc_api_sync_stream(dc_api_sync_t sync);
char const *dc_api_sync_data(dc_api_sync_t sync);
size_t dc_api_sync_datalen(dc_api_sync_t sync);
int dc_api_sync_code(dc_api_sync_t sync);
struct curl_slist *dc_api_sync_list(dc_api_sync_t sync);

bool dc_api_sync_wait(dc_api_sync_t sync);
void dc_api_sync_finish(dc_api_sync_t sync, int code);

#endif
