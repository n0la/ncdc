#include <ncdc/ncdc.h>

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

wchar_t* wcsndup(const wchar_t* string, size_t maxlen)
{
    size_t n = wcsnlen(string, maxlen) + 1;
    wchar_t* r = calloc(n, sizeof(wchar_t));
    return r == NULL ? NULL : wmemcpy(r, string, n);
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

wchar_t **w_tokenise(wchar_t const *w)
{
    GPtrArray *array = g_ptr_array_new();
    wchar_t const *item = w;
    wchar_t *dup = NULL;
    size_t len = 0, origlen = 0;

    while ((dup = w_next_tok(item)) != NULL) {
        len = origlen = wcslen(dup);

        if (*dup == '"') {
            memmove(dup, dup+1, sizeof(wchar_t)*(len-1));
            --len;
        }

        if (len > 0 && dup[len-1] == '"') {
            dup[len-1] = '\0';
            --len;
        }

        g_ptr_array_add(array, dup);
        item += origlen;
    }

    g_ptr_array_add(array, NULL);

    return (wchar_t**)g_ptr_array_free(array, FALSE);
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

wchar_t *w_next_tok(wchar_t const *w)
{
    bool quotes = false;
    wchar_t const *start = NULL;

    /* skip first white spaces if there are any
     */
    for (; *w != '\0' && iswspace(*w); w++)
        ;

    if (*w == '\0') {
        return NULL;
    }

    start = w;
    quotes = (*w == '"');

    do {
        if (iswspace(*w) && !quotes) {
            --w;
            break;
        }

        if (*w == '"' && *(w-1) != '\\' && quotes) {
            break;
        }

        if (*w == '\0') {
            break;
        }

        ++w;
    } while (1);

    return wcsndup(start, (w - start));
}
