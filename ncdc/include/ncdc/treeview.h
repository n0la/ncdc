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

size_t ncdc_treeitem_size(ncdc_treeitem_t i);
void ncdc_treeitem_clear(ncdc_treeitem_t i);
void ncdc_treeitem_add(ncdc_treeitem_t i, ncdc_treeitem_t c);
void ncdc_treeitem_remove(ncdc_treeitem_t i, ncdc_treeitem_t c);

struct ncdc_treeview_;
typedef struct ncdc_treeview_ *ncdc_treeview_t;

ncdc_treeview_t ncdc_treeview_new(void);
ncdc_treeitem_t ncdc_treeview_root(ncdc_treeview_t t);
void ncdc_treeview_render(ncdc_treeview_t t, WINDOW *w, int lines, int cols);

/* move the cursor around, and collapse/expand items
 */
void ncdc_treeview_previous(ncdc_treeview_t t);
void ncdc_treeview_next(ncdc_treeview_t t);
void ncdc_treeview_collapse(ncdc_treeview_t t);
void ncdc_treeview_expand(ncdc_treeview_t t);

#endif
