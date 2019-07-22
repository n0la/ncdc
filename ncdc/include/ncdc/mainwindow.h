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

#ifndef NCDC_MAINWINDOW_H
#define NCDC_MAINWINDOW_H

#include <ncdc/ncdc.h>
#include <ncdc/textview.h>
#include <stdarg.h>

struct ncdc_mainwindow_;
typedef struct ncdc_mainwindow_ *ncdc_mainwindow_t;

ncdc_mainwindow_t ncdc_mainwindow_new(void);

/* holy shit stains I am lazy
 */
#define LOG(n, ...) ncdc_mainwindow_log(n, __VA_ARGS__)
void ncdc_mainwindow_log(ncdc_mainwindow_t w, wchar_t const *fmt, ...);

GPtrArray *ncdc_mainwindow_views(ncdc_mainwindow_t n);
void ncdc_mainwindow_close_view(ncdc_mainwindow_t n, int i);
dc_channel_t ncdc_mainwindow_current_channel(ncdc_mainwindow_t n);
void ncdc_mainwindow_switch_view(ncdc_mainwindow_t n, ncdc_textview_t v);
ncdc_textview_t ncdc_mainwindow_channel_view(ncdc_mainwindow_t n,
                                             dc_channel_t c);
ncdc_textview_t ncdc_mainwindow_switch_or_add(ncdc_mainwindow_t n,
                                              dc_channel_t c);

void ncdc_mainwindow_refresh(ncdc_mainwindow_t n);
void ncdc_mainwindow_input_ready(ncdc_mainwindow_t n);

void ncdc_mainwindow_rightview(ncdc_mainwindow_t n);
void ncdc_mainwindow_leftview(ncdc_mainwindow_t n);
void ncdc_mainwindow_switch_guilds(ncdc_mainwindow_t n);
void ncdc_mainwindow_switch_input(ncdc_mainwindow_t n);
void ncdc_mainwindow_switch_chat(ncdc_mainwindow_t n);

void ncdc_mainwindow_update_guilds(ncdc_mainwindow_t n);

#endif
