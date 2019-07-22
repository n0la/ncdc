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

#include <ncdc/input.h>
#include <ncdc/ncdc.h>
#include <ncdc/keycodes.h>

struct ncdc_input_
{
    dc_refable_t ref;

    GArray *buffer;
    int cursor;
    ncdc_keybinding_t *keys;

    ncdc_input_callback_t callback;
    void *callback_arg;
};

static void ncdc_input_enter(ncdc_input_t p);

static void ncdc_input_free(ncdc_input_t p)
{
    g_array_unref(p->buffer);

    free(p);
}

ncdc_input_t ncdc_input_new(void)
{
    ncdc_input_t p = calloc(1, sizeof(struct ncdc_input_));
    return_if_true(p == NULL, NULL);

    p->ref.cleanup = (dc_cleanup_t)ncdc_input_free;

    p->buffer = g_array_new(TRUE, TRUE, sizeof(wchar_t));
    p->cursor = 0;

    p->keys = keys_emacs;

    return p;
}

void ncdc_input_feed(ncdc_input_t input, wchar_t const *c, size_t sz)
{
    return_if_true(input == NULL,);
    ncdc_keybinding_t *bind = NULL;

    if (c[0] == '\r') {
        ncdc_input_enter(input);
    } else if ((bind = ncdc_find_keybinding(input->keys, c, sz)) != NULL) {
        bind->handler(input);
    } else if (iswprint(c[0])) {
        g_array_insert_vals(input->buffer, input->cursor, &c[0], 1);
        input->cursor += wcswidth(&c[0], 1);
    }
}

void ncdc_input_set_callback(ncdc_input_t i, ncdc_input_callback_t c, void *a)
{
    return_if_true(i == NULL,);

    i->callback = c;
    i->callback_arg = a;
}

int ncdc_input_cursor(ncdc_input_t input)
{
    return_if_true(input == NULL, 0);
    return input->cursor;
}

char const *ncdc_input_buffer(ncdc_input_t input)
{
    return_if_true(input == NULL, NULL);
    return (char const *)input->buffer->data;
}

void ncdc_input_draw(ncdc_input_t input, WINDOW *win)
{
    werase(win);
    mvwaddwstr(win, 0, 0, (wchar_t const *)input->buffer->data);
    wmove(win, 0, input->cursor);
    wrefresh(win);
}

static void ncdc_input_enter(ncdc_input_t input)
{
    if (input->buffer->len == 0) {
        return;
    }

    if (input->callback != NULL) {
        bool ret = false;

        ret = input->callback(input,
                              (wchar_t const *)input->buffer->data,
                              input->buffer->len,
                              input->callback_arg
            );
        if (ret) {
            /* TODO: add to history
             */
        }
    }

    g_array_remove_range(input->buffer, 0, input->buffer->len);
    input->cursor = 0;
}

void ncdc_input_kill_word_left(ncdc_input_t input)
{
    ssize_t i = 0, j = 0;

    return_if_true(input->cursor == 0,);
    return_if_true(input->buffer->len == 0,);

    i = input->cursor;
    if (i == input->buffer->len) {
        --i;
    }

    for (; i >= 0 && iswspace(g_array_index(input->buffer, wchar_t, i)); i--, j++)
        ;
    for (; i >= 0 && !iswspace(g_array_index(input->buffer, wchar_t, i)); i--, j++)
        ;

    if (i < 0) {
        i = 0;
    }

    g_array_remove_range(input->buffer, i, j);
    input->cursor = i;
}

void ncdc_input_kill_left(ncdc_input_t input)
{
    return_if_true(input->cursor == 0,);
    g_array_remove_range(input->buffer, 0, input->cursor);
    input->cursor = 0;
}

void ncdc_input_kill_right(ncdc_input_t input)
{
    return_if_true(input->cursor == input->buffer->len,);
    g_array_remove_range(input->buffer, input->cursor,
                         input->buffer->len - input->cursor
        );
}

void ncdc_input_delete(ncdc_input_t input)
{
    return_if_true(input->cursor == input->buffer->len,);
    g_array_remove_index(input->buffer, input->cursor);
}

void ncdc_input_delete_backward(ncdc_input_t input)
{
    return_if_true(input->cursor == 0,);

    g_array_remove_index(input->buffer, input->cursor-1);
    --input->cursor;
}

void ncdc_input_backward(ncdc_input_t input)
{
    return_if_true(input->cursor == 0,);

    wchar_t c = g_array_index(input->buffer, wchar_t, input->cursor-1);
    int len = wcswidth(&c, 1);
    input->cursor -= len;
}

void ncdc_input_forward(ncdc_input_t input)
{
    return_if_true(input->cursor >= input->buffer->len,);

    wchar_t c = g_array_index(input->buffer, wchar_t, input->cursor);
    int len = wcswidth(&c, 1);
    input->cursor += len;
}
