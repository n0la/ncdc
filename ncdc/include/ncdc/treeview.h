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

#ifndef NCDC_TREEVIEW_H
#define NCDC_TREEVIEW_H

#include <string.h>
#include <stdlib.h>
#include <ncdc/ncdc.h>

struct ncdc_treeitem_;
typedef struct ncdc_treeitem_ *ncdc_treeitem_t;

ncdc_treeitem_t ncdc_treeitem_new(void);
ncdc_treeitem_t ncdc_treeitem_new_string(wchar_t const *s);

wchar_t const *ncdc_treeitem_label(ncdc_treeitem_t i);
void ncdc_treeitem_set_label(ncdc_treeitem_t i, wchar_t const *s);

void *ncdc_treeitem_tag(ncdc_treeitem_t i);
void ncdc_treeitem_set_tag(ncdc_treeitem_t i, void *t);
ncdc_treeitem_t ncdc_treeitem_parent(ncdc_treeitem_t i);

size_t ncdc_treeitem_size(ncdc_treeitem_t i);
void ncdc_treeitem_clear(ncdc_treeitem_t i);
void ncdc_treeitem_add(ncdc_treeitem_t i, ncdc_treeitem_t c);
void ncdc_treeitem_remove(ncdc_treeitem_t i, ncdc_treeitem_t c);

struct ncdc_treeview_;
typedef struct ncdc_treeview_ *ncdc_treeview_t;

ncdc_treeview_t ncdc_treeview_new(void);
ncdc_treeitem_t ncdc_treeview_root(ncdc_treeview_t t);
ncdc_treeitem_t ncdc_treeview_current(ncdc_treeview_t t);
void ncdc_treeview_render(ncdc_treeview_t t, WINDOW *w, int lines, int cols);

/* move the cursor around, and collapse/expand items
 */
void ncdc_treeview_previous(ncdc_treeview_t t);
void ncdc_treeview_next(ncdc_treeview_t t);
void ncdc_treeview_collapse(ncdc_treeview_t t);
void ncdc_treeview_expand(ncdc_treeview_t t);

#endif
