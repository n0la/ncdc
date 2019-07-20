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

#ifndef NCDC_KEYCODES_H
#define NCDC_KEYCODES_H

#include <ncdc/ncdc.h>

typedef void (*ncdc_keyhandler_t)(void *p);

typedef struct {
    wchar_t key[10];
    wchar_t const *name;
    ncdc_keyhandler_t handler;
} ncdc_keybinding_t;

#define NCDC_BINDCUR(k, n, f) { .key = { k, '\0' }, .name = n, .handler = (ncdc_keyhandler_t) f }
#define NCDC_BINDING(k, n, f) { .key = k, .name = n, .handler = (ncdc_keyhandler_t) f }
#define NCDC_BINDEND()        { .key = {0}, .name = NULL, .handler = NULL }

extern ncdc_keybinding_t keys_guilds[];
extern ncdc_keybinding_t keys_emacs[];
extern ncdc_keybinding_t keys_chat[];
extern ncdc_keybinding_t keys_global[];

ncdc_keybinding_t *ncdc_find_keybinding(ncdc_keybinding_t *keys,
                                        wchar_t const *key, size_t l);

#endif
