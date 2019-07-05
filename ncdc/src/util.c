#include <ncdc/ncdc.h>

wchar_t *util_readkey(int e, WINDOW *win)
{
    wint_t esc[7] = {0};
    int i = 0;

    return_if_true(e != KEY_ESCAPE, NULL);

    esc[0] = e;
    for (i = 1; i < 6; i++) {
        if (wget_wch(win, esc+i) == ERR) {
            return NULL;
        }
    }

    return wcsdup((wchar_t const *)esc);
}

wchar_t const *w_next_word(wchar_t const *w, ssize_t len)
{
    size_t i = 0;

    if (len < 0) {
        len = wcslen(w);
    }

    for (; !iswspace(w[i]) && i < len; i++)
        ;

    return w+i;
}

int aswprintf(wchar_t **buffer, wchar_t const *fmt, ...)
{
    size_t sz = 0;
    FILE *f = NULL;
    va_list lst;

    f = open_wmemstream(buffer, &sz);
    if (f == NULL) {
        return -1;
    }

    va_start(lst, fmt);
    vfwprintf(f, fmt, lst);
    va_end(lst);

    fclose(f);
    return sz;
}

wchar_t* wcsndup(wchar_t const* string, size_t maxlen)
{
    wchar_t* r = calloc(maxlen+1, sizeof(wchar_t));
    return_if_true(r == NULL, NULL);
    return wmemcpy(r, string, maxlen);
}

size_t w_strlenv(wchar_t **s)
{
    size_t i = 0;
    for (; s[i] != NULL; i++)
        ;
    return i;
}

void w_strfreev(wchar_t **s)
{
    size_t i = 0;

    return_if_true(s == NULL,);

    for (; s[i] != NULL; i++) {
        free(s[i]);
    }

    free(s);
}

wchar_t **w_tokenise(wchar_t const *str)
{
    wchar_t const *p = NULL, *start_of_word = NULL;
    wint_t c;
    GPtrArray *array = g_ptr_array_new();
    enum states { DULL, IN_WORD, IN_STRING } state = DULL;

    for (p = str; *p != '\0'; p++) {
        c = (wint_t) *p;
        switch (state) {
        case DULL:
        {
            if (iswspace(c)) {
                continue;
            }
            if (c == '"') {
                state = IN_STRING;
                start_of_word = p + 1;
                continue;
            }
            state = IN_WORD;
            start_of_word = p;
        } continue;

        case IN_STRING:
        {
            if (c == '"') {
                size_t len = (p - 2 - start_of_word);
                wchar_t *s = wcsndup(start_of_word, len);
                g_ptr_array_add(array, s);
                state = DULL;
            }
        } continue;

        case IN_WORD:
        {
            if (iswspace(c)) {
                size_t len = (p - start_of_word);
                wchar_t *s = wcsndup(start_of_word, len);
                g_ptr_array_add(array, s);
                state = DULL;
            }
        } continue;
        }
    }

    if (state != DULL) {
        size_t len = (p - start_of_word);
        wchar_t *s = wcsndup(start_of_word, len);
        g_ptr_array_add(array, s);
    }

    g_ptr_array_add(array, NULL);

    return (wchar_t**)g_ptr_array_free(array, FALSE);
}

wchar_t *w_joinv(wchar_t const **v, size_t len)
{
    wchar_t *buf = NULL;
    size_t buflen = 0;
    FILE *f = open_wmemstream(&buf, &buflen);
    size_t i = 0;

    return_if_true(f == NULL, NULL);

    for (i = 0; i < len; i++) {
        fwprintf(f, L"%ls", v[i]);
        if (i < (len-1)) {
            fputwc(' ', f);
        }
    }

    fclose(f);
    return buf;
}

char *w_convert(wchar_t const *w)
{
    size_t sz = 0;
    char *ptr = NULL;

    sz = wcstombs(NULL, w, 0);

    ptr = calloc(sz+1, sizeof(char));
    return_if_true(ptr == NULL, NULL);

    wcstombs(ptr, w, sz);
    return ptr;
}
