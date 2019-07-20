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

#ifndef NCDC_INPUT_H
#define NCDC_INPUT_H

#include <ncdc/ncdc.h>
#include <ncdc/keycodes.h>

struct ncdc_input_;
typedef struct ncdc_input_ *ncdc_input_t;

typedef bool (*ncdc_input_callback_t)(ncdc_input_t p, wchar_t const *str,
                                      size_t len, void *data);

ncdc_input_t ncdc_input_new(void);

void ncdc_input_feed(ncdc_input_t input, wchar_t const *c, size_t sz);
int ncdc_input_cursor(ncdc_input_t input);
char const *ncdc_input_buffer(ncdc_input_t input);
void ncdc_input_draw(ncdc_input_t input, WINDOW *win);

void ncdc_input_set_callback(ncdc_input_t i, ncdc_input_callback_t c, void *a);

/* keybinding functions
 */
void ncdc_input_kill_right(ncdc_input_t input);
void ncdc_input_kill_left(ncdc_input_t input);
void ncdc_input_kill_word_left(ncdc_input_t input);
void ncdc_input_backward(ncdc_input_t i);
void ncdc_input_forward(ncdc_input_t i);
void ncdc_input_delete(ncdc_input_t input);
void ncdc_input_delete_backward(ncdc_input_t input);

#endif
