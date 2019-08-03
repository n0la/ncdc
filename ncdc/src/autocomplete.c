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

#include <ncdc/autocomplete.h>

struct ncdc_autocomplete_
{
    dc_refable_t ref;

    wchar_t *buffer;
    wchar_t *word;
    wchar_t *completed;

    wchar_t **completions;
    size_t completions_size;
    size_t search_pos;

    size_t buffer_size;
    size_t pos;
    size_t word_pos;
    size_t start;
    size_t end;
};

static void ncdc_autocomplete_free(ncdc_autocomplete_t a)
{
    return_if_true(a == NULL,);

    free(a->buffer);
    a->buffer = NULL;

    free(a->completed);
    a->completed = NULL;

    free(a->word);
    a->word = NULL;

    w_strfreev(a->completions);
    a->completions = NULL;

    free(a);
}

ncdc_autocomplete_t ncdc_autocomplete_new(void)
{
    ncdc_autocomplete_t c = calloc(1, sizeof(struct ncdc_autocomplete_));
    return_if_true(c == NULL, NULL);

    c->ref.cleanup = (dc_cleanup_t)ncdc_autocomplete_free;

    return dc_ref(c);
}

void ncdc_autocomplete_reset(ncdc_autocomplete_t a)
{
    return_if_true(a == NULL,);

    free(a->buffer);
    a->buffer = NULL;

    free(a->word);
    a->word = NULL;

    free(a->completed);
    a->completed = NULL;

    w_strfreev(a->completions);
    a->completions = NULL;
    a->completions_size = 0;

    a->buffer_size = a->search_pos = a->pos = a->word_pos = a->start = a->end = 0;
}

bool ncdc_autocomplete_prepare(ncdc_autocomplete_t a,
                               wchar_t const *s, ssize_t sz,
                               size_t pos)
{
    return_if_true(a == NULL || s == NULL, false);

    size_t start = pos, end = pos;
    ssize_t i = 0, words = 0;
    bool inword = false;

    return_if_true(a->buffer != NULL && a->word != NULL, true);

    if (sz < 0) {
        sz = wcslen(s);
    }

    /* figure out the current word for autocompletion
     */
    for (start = pos; !iswspace(s[start]) && start > 0; start--)
        ;

    for (end = pos; !iswspace(s[end]) && pos < sz; end++)
        ;

    /* count words backwards, to figure out what position our
     * current word has in the string. this is important for
     * others because they might have positional arguments
     */
    if (start > 0) {
        for (i = start; i >= 0; i--) {
            if (iswspace(s[i])) {
            inword = false;
            } else if (!inword) {
                inword = true;
                ++words;
            }
        }
    }

    free(a->buffer);
    a->buffer = wcsndup(s, sz);
    a->buffer_size = sz;

    a->start = start;
    a->end = end;

    a->pos = pos;
    a->word_pos = words;

    a->word = wcsndup(s + start, end);

    return true;
}

void ncdc_autocomplete_completions(ncdc_autocomplete_t a,
                                   wchar_t **words, ssize_t num)
{
    return_if_true(a == NULL,);

    w_strfreev(a->completions);
    a->completions = NULL;
    a->completions_size = 0;

    if (words != NULL) {
        a->completions = w_strdupv(words, num);
        a->completions_size = w_strlenv(words);
    }
}

int ncdc_autocomplete_word_index(ncdc_autocomplete_t a)
{
    return_if_true(a == NULL, -1);
    return a->word_pos;
}

bool ncdc_autocomplete_complete(ncdc_autocomplete_t a, int *newpos)
{
    bool found = false;
    size_t wordlen = 0, foundlen = 0;
    wchar_t const *item = NULL;

    return_if_true(a == NULL, false);

    wordlen = wcslen(a->word);

    for (; a->search_pos < a->completions_size; a->search_pos++) {
        item = a->completions[a->search_pos];
        if (wcsncmp(a->word, item, wordlen) == 0) {
            found = true;
            break;
        }
    }

    ++a->search_pos;

    if (a->search_pos >= a->completions_size) {
        a->search_pos = 0; /* wrap around search */
    }

    if (!found) {
        return false;
    }

    free(a->completed);
    a->completed = NULL;

    foundlen = wcslen(item);

    a->completed = calloc(wcslen(a->buffer) + foundlen + 1, sizeof(wchar_t));
    return_if_true(a->completed == NULL, false);

    wcsncat(a->completed, a->buffer, a->start);
    wcscat(a->completed, item);
    wcscat(a->completed, a->buffer + a->end);

    if (newpos != NULL) {
        *newpos = a->start + foundlen;
    }

    return true;
}

wchar_t const *ncdc_autocomplete_completed(ncdc_autocomplete_t a)
{
    return_if_true(a == NULL, NULL);
    return a->completed;
}
