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

void ncdc_treeitem_add(ncdc_treeitem_t i, ncdc_treeitem_t c)
{
    return_if_true(i == NULL || c == NULL,);
    g_ptr_array_add(i->children, dc_ref(c));
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

    if (t->content != NULL) {
        size_t len = wcslen(t->content);

        mvwaddwstr(win, l, c, t->content);
        off = ((len + c) / cols) + 1;
    }

    for (i = 0; i < t->children->len; i++) {
        ncdc_treeitem_t child = g_ptr_array_index(t->children, i);
        ret = ncdc_treeitem_render(child, win, lines, cols,
                                   l + off, c + 1
            );
        off += ret;
    }

    return off;
}

struct ncdc_treeview_
{
    dc_refable_t ref;

    ncdc_treeitem_t root;
};

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

ncdc_treeitem_t ncdc_treeview_root(ncdc_treeview_t t)
{
    return_if_true(t == NULL, NULL);
    return t->root;
}
