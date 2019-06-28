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

char *read_char(FILE *stream)
{
    uint8_t str[7] = {0};
    int len = 0, i = 0;

    /* check if we need more
     */
    str[0] = (uint8_t)fgetc(stream);
    len = g_utf8_skip[str[0]];

    for (i = 1; i < len; i++) {
        str[i] = (uint8_t)fgetc(stream);
    }
    str[len] = '\0';

    return strdup((char const *)str);
}

int strwidth(char const *string)
{
    size_t needed = mbstowcs(NULL, string, 0) + 1;
    wchar_t *wcstring = calloc(needed, sizeof(wchar_t));
    size_t ret = 0;

    return_if_true(wcstring == NULL, -1);

    ret = mbstowcs(wcstring, string, needed);
    if (ret == (size_t)-1) {
        free(wcstring);
        return -1;
    }

    int width = wcswidth(wcstring, needed);
    free(wcstring);

    return width;
}
