#include <ncdc/treeview.h>
#include <ncdc/ncdc.h>

struct ncdc_treeitem_
{
    dc_refable_t ref;

    /* content
     */
    wchar_t *content;

    /* collapsed?
     */
    bool collapsed;

    /* children
     */
    GPtrArray *children;

    /* user defined data
     */
    void *tag;

    /* highlight current item?
     */
    bool highlight;

    /* parent
     */
    struct ncdc_treeitem_ *parent;
};

struct ncdc_treeview_
{
    dc_refable_t ref;

    /* root element
     */
    ncdc_treeitem_t root;

    /* currently selected item
     */
    ncdc_treeitem_t current;
};

static void ncdc_treeitem_free(ncdc_treeitem_t t)
{
    return_if_true(t == NULL,);

    free(t->content);
    t->content = NULL;

    if (t->children != NULL) {
        g_ptr_array_unref(t->children);
        t->children = NULL;
    }

    free(t);
}

ncdc_treeitem_t ncdc_treeitem_new(void)
{
    ncdc_treeitem_t t = calloc(1, sizeof(struct ncdc_treeitem_));
    return_if_true(t == NULL, NULL);

    t->ref.cleanup = (dc_cleanup_t)ncdc_treeitem_free;

    t->children = g_ptr_array_new_with_free_func((GDestroyNotify)dc_unref);
    if (t->children == NULL) {
        free(t);
        return NULL;
    }

    return dc_ref(t);
}

ncdc_treeitem_t ncdc_treeitem_new_string(wchar_t const *s)
{
    ncdc_treeitem_t t = ncdc_treeitem_new();
    return_if_true(t == NULL, NULL);

    t->content = wcsdup(s);

    return t;
}

size_t ncdc_treeitem_size(ncdc_treeitem_t i)
{
    return_if_true(i == NULL || i->children == NULL, 0);
    return i->children->len;
}

void ncdc_treeitem_add(ncdc_treeitem_t i, ncdc_treeitem_t c)
{
    return_if_true(i == NULL || c == NULL,);
    g_ptr_array_add(i->children, dc_ref(c));
    c->parent = i;
}

void ncdc_treeitem_remove(ncdc_treeitem_t i, ncdc_treeitem_t c)
{
    return_if_true(i == NULL || c == NULL,);
    g_ptr_array_remove(i->children, c);
}

void ncdc_treeitem_clear(ncdc_treeitem_t i)
{
    return_if_true(i == NULL || i->children == NULL,);
    return_if_true(i->children->len == 0,);
    g_ptr_array_remove_range(i->children, 0, i->children->len);
}

void *ncdc_treeitem_tag(ncdc_treeitem_t i)
{
    return_if_true(i == NULL, NULL);
    return i->tag;
}

void ncdc_treeitem_set_tag(ncdc_treeitem_t i, void *t)
{
    return_if_true(i == NULL,);
    i->tag = t;
}

ncdc_treeitem_t ncdc_treeitem_parent(ncdc_treeitem_t i)
{
    return_if_true(i == NULL, NULL);
    return i->parent;
}

wchar_t const *ncdc_treeitem_label(ncdc_treeitem_t i)
{
    return_if_true(i == NULL, NULL);
    return i->content;
}

void ncdc_treeitem_set_label(ncdc_treeitem_t i, wchar_t const *s)
{
    return_if_true(i == NULL || s == NULL,);
    free(i->content);
    i->content = wcsdup(s);
}

static int
ncdc_treeitem_render(ncdc_treeitem_t t, WINDOW *win,
                     int lines, int cols, int l, int c)
{
    size_t i = 0, off = 0;
    int ret = 0;
    wchar_t *data = NULL;
    size_t len = 0;

    if (t->content != NULL) {
        if (t->highlight) {
            wattron(win, COLOR_PAIR(ncdc_colour_treehighlight));
        }

        if (t->children->len > 0) {
            aswprintf(&data, L"[%c] %ls",
                      (t->collapsed ? '-' : '+'),
                      t->content
                );
        } else {
            data = wcsdup(t->content);
        }
        len = wcslen(data);

        mvwaddwstr(win, l, c, data);
        off = ((len + c) / cols) + 1;

        free(data);
        data = NULL;

        if (t->highlight) {
            wattroff(win, COLOR_PAIR(ncdc_colour_treehighlight));
        }
    }

    if (!t->collapsed) {
        for (i = 0; i < t->children->len; i++) {
            ncdc_treeitem_t child = g_ptr_array_index(t->children, i);
            ret = ncdc_treeitem_render(child, win, lines, cols,
                                       l + off, c + 1
                );
            off += ret;
        }
    }

    return off;
}

static void ncdc_treeview_free(ncdc_treeview_t t)
{
    return_if_true(t == NULL,);

    dc_unref(t->root);
    t->root = NULL;

    free(t);
}

ncdc_treeview_t ncdc_treeview_new(void)
{
    ncdc_treeview_t t = calloc(1, sizeof(struct ncdc_treeview_));
    return_if_true(t == NULL, NULL);

    t->ref.cleanup = (dc_cleanup_t)ncdc_treeview_free;

    t->root = ncdc_treeitem_new();
    goto_if_true(t->root == NULL, error);

    t->current = t->root;

    return dc_ref(t);

error:

    dc_unref(t);
    return NULL;
}

void ncdc_treeview_render(ncdc_treeview_t t, WINDOW *w, int lines, int cols)
{
    werase(w);
    wmove(w, 0, 0);
    ncdc_treeitem_render(t->root, w, lines, cols, 0, 0);
}

ncdc_treeitem_t ncdc_treeview_current(ncdc_treeview_t t)
{
    return_if_true(t == NULL, NULL);
    return t->current;
}

ncdc_treeitem_t ncdc_treeview_root(ncdc_treeview_t t)
{
    return_if_true(t == NULL, NULL);
    return t->root;
}

void ncdc_treeview_previous(ncdc_treeview_t t)
{
    return_if_true(t == NULL,);

    ncdc_treeitem_t cur = t->current;
    ncdc_treeitem_t found = NULL, child = NULL;
    size_t i = 0;

    if (cur->parent != NULL) {
        for (i = 0; i < cur->parent->children->len; i++) {
            child = g_ptr_array_index(cur->parent->children, i);
            if (child == cur && i > 0) {
                found = g_ptr_array_index(cur->parent->children, i-1);
                break;
            }
        }
    }

    if (found == NULL) {
        cur = cur->parent;
    } else {
        cur = found;
    }

    if (cur == NULL) {
        cur = t->root;
    }

    t->current->highlight = false;
    cur->highlight = true;
    t->current = cur;
}

void ncdc_treeview_next(ncdc_treeview_t t)
{
    return_if_true(t == NULL,);

    ncdc_treeitem_t cur = t->current;
    ncdc_treeitem_t found = NULL;
    ncdc_treeitem_t child = NULL;
    size_t i = 0;

    if (cur->children->len == 0 || cur->collapsed) {
        while (cur != NULL && cur->parent != NULL && found == NULL) {

            found = NULL;
            for (i = 0; i < cur->parent->children->len; i++) {
                child = g_ptr_array_index(cur->parent->children, i);
                if (child == cur && i < cur->parent->children->len-1) {
                    found = g_ptr_array_index(cur->parent->children, i+1);
                    break;
                }
            }

            if (found == NULL) {
                cur = cur->parent;
            }
        }
        cur = found;
    } else if (cur->children->len > 0 && !cur->collapsed) {
        cur = g_ptr_array_index(cur->children, 0);
    }

    if (cur == NULL) {
        cur = t->root;
    }

    t->current->highlight = false;
    cur->highlight = true;
    t->current = cur;
}

void ncdc_treeview_collapse(ncdc_treeview_t t)
{
    return_if_true(t == NULL || t->current == NULL,);
    return_if_true(t->current == t->root,);
    t->current->collapsed = true;
}

void ncdc_treeview_expand(ncdc_treeview_t t)
{
    return_if_true(t == NULL || t->current == NULL,);
    return_if_true(t->current == t->root,);
    t->current->collapsed = false;
}
