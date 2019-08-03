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

#ifndef NCDC_AUTOCOMPLETE_H
#define NCDC_AUTOCOMPLETE_H

#include <ncdc/ncdc.h>

struct ncdc_autocomplete_;
typedef struct ncdc_autocomplete_ *ncdc_autocomplete_t;

ncdc_autocomplete_t ncdc_autocomplete_new(void);

bool ncdc_autocomplete_prepare(ncdc_autocomplete_t a,
                               wchar_t const *s, ssize_t sz,
                               size_t pos);
void ncdc_autocomplete_completions(ncdc_autocomplete_t a,
                                   wchar_t **words, ssize_t num);
bool ncdc_autocomplete_complete(ncdc_autocomplete_t a, int *newpos);
void ncdc_autocomplete_reset(ncdc_autocomplete_t a);

int ncdc_autocomplete_word_index(ncdc_autocomplete_t a);
wchar_t const *ncdc_autocomplete_completed(ncdc_autocomplete_t a);

#endif
