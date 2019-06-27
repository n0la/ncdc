#include <ncdc/input.h>
#include <ncdc/ncdc.h>

struct ncdc_input_
{
    dc_refable_t ref;

    GArray *buffer;
    int cursor;
    ncdc_input_keybinding_t *keys;

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

    p->keys = emacs;

    return p;
}

static ncdc_keybinding_t has_binding(ncdc_input_t in, wchar_t key)
{
    char const *k = keyname(key);
    size_t i = 0;

    for (; in->keys[i].name != NULL; i++) {
        if (strcmp(k, in->keys[i].key) == 0) {
            return in->keys[i].handler;
        }
    }

    return NULL;
}

void ncdc_input_feed(ncdc_input_t input, wchar_t c)
{
    return_if_true(input == NULL,);
    ncdc_keybinding_t handler = NULL;

    if (c == '\r') {
        ncdc_input_enter(input);
    } else if ((handler = has_binding(input, c)) != NULL) {
        handler(input);
    } else if (iswprint(c)) {
        g_array_insert_vals(input->buffer, input->cursor, &c, 1);
        input->cursor += wcswidth(&c, 1);
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
