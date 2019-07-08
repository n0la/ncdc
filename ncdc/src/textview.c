#include <ncdc/textview.h>
#include <ncdc/ncdc.h>

struct ncdc_textview_
{
    dc_refable_t ref;

    GPtrArray *par;
    wchar_t *title;

    dc_account_t account;
    dc_channel_t channel;
};

static void ncdc_textview_free(ncdc_textview_t v)
{
    return_if_true(v == NULL,);

    free(v->title);
    v->title = NULL;

    dc_unref(v->account);
    dc_unref(v->channel);

    if (v->par != NULL) {
        g_ptr_array_unref(v->par);
    }

    free(v);
}

ncdc_textview_t ncdc_textview_new(void)
{
    ncdc_textview_t p = calloc(1, sizeof(struct ncdc_textview_));
    return_if_true(p == NULL, NULL);

    p->ref.cleanup = (dc_cleanup_t)ncdc_textview_free;

    p->par = g_ptr_array_new_with_free_func(free);
    if (p->par == NULL) {
        free(p);
        return NULL;
    }

    return dc_ref(p);
}

static void ncdc_textview_maketitle(ncdc_textview_t v)
{
    size_t i = 0;
    wchar_t *buf = NULL;
    size_t buflen = 0;
    FILE *f = open_wmemstream(&buf, &buflen);
    size_t rlen = dc_channel_recipients(v->channel);

    for (i = 0; i < rlen; i++) {
        dc_account_t r = dc_channel_nthrecipient(v->channel, i);
        fwprintf(f, L"%s", dc_account_fullname(r));
        if (i < (rlen-1)) {
            fwprintf(f, L",");
        }
    }

    fclose(f);

    free(v->title);
    v->title = buf;
}

wchar_t const *ncdc_textview_title(ncdc_textview_t v)
{
    if (v->title == NULL && v->channel != NULL) {
        ncdc_textview_maketitle(v);
    }

    return v->title;
}

void ncdc_textview_set_title(ncdc_textview_t v, wchar_t const *w)
{
    free(v->title);
    v->title = wcsdup(w);
}

dc_account_t ncdc_textview_account(ncdc_textview_t v)
{
    return_if_true(v == NULL, NULL);
    return v->account;
}

void ncdc_textview_set_account(ncdc_textview_t v, dc_account_t a)
{
    return_if_true(v == NULL || a == NULL,);
    dc_unref(v->account);
    v->account = dc_ref(a);
}

dc_channel_t ncdc_textview_channel(ncdc_textview_t v)
{
    return_if_true(v == NULL, NULL);
    return v->channel;
}

void ncdc_textview_set_channel(ncdc_textview_t v, dc_channel_t a)
{
    return_if_true(v == NULL || a == NULL,);
    dc_unref(v->channel);
    v->channel = dc_ref(a);
}

void ncdc_textview_append(ncdc_textview_t v, wchar_t const *w)
{
    return_if_true(v == NULL || w == NULL,);

    wchar_t const *p = w;
    wchar_t const *last = w;

    while ((p = wcschr(p, '\n')) != NULL) {
        wchar_t *dup = wcsndup(p, p - last);
        if (dup != NULL) {
            g_ptr_array_add(v->par, dup);
            last = p;
        }
    }

    g_ptr_array_add(v->par, wcsdup(last));
}

wchar_t const *ncdc_textview_nthline(ncdc_textview_t v, size_t idx)
{
    return_if_true(v == NULL, NULL);
    return_if_true(idx >= v->par->len, NULL);
    return g_ptr_array_index(v->par, idx);
}

void ncdc_textview_render(ncdc_textview_t v, WINDOW *win, int lines, int cols)
{
    ssize_t i = 0, needed_lines = 0, atline = 0;

    werase(win);

    if (v->par == NULL || v->par->len == 0) {
        return;
    }

    for (i = v->par->len-1; i >= 0; i--) {
        wchar_t const *w = ncdc_textview_nthline(v, i);
        size_t sz = wcslen(w);

        needed_lines += (sz / cols);
        if ((sz / cols) == 0) {
            needed_lines += 1;
        }
        atline = (lines - needed_lines);
        mvwaddwstr(win, atline, 0, ncdc_textview_nthline(v, i));

        if (needed_lines >= lines) {
            break;
        }
    }
}
