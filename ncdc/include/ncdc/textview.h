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

#ifndef NCDC_TEXTVIEW_H
#define NCDC_TEXTVIEW_H

#include <ncdc/ncdc.h>

struct ncdc_textview_;
typedef struct ncdc_textview_ *ncdc_textview_t;

ncdc_textview_t ncdc_textview_new(void);

dc_account_t ncdc_textview_account(ncdc_textview_t v);
void ncdc_textview_set_account(ncdc_textview_t v, dc_account_t a);

dc_channel_t ncdc_textview_channel(ncdc_textview_t v);
void ncdc_textview_set_channel(ncdc_textview_t v, dc_channel_t a);

wchar_t const *ncdc_textview_title(ncdc_textview_t v);
void ncdc_textview_set_title(ncdc_textview_t v, wchar_t const *w);

void ncdc_textview_append(ncdc_textview_t v, wchar_t const *w);
wchar_t const *ncdc_textview_nthline(ncdc_textview_t v, size_t i);
void ncdc_textview_render(ncdc_textview_t v, WINDOW *win, int lines, int cols);

#endif
