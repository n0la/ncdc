#ifndef NCDC_KEYCODES_H
#define NCDC_KEYCODES_H

#include <ncdc/ncdc.h>
#include <ncdc/input.h>

typedef void (*ncdc_keybinding_t)(void *p);

typedef struct {
    wchar_t key[10];
    wchar_t const *name;
    ncdc_keybinding_t handler;
} ncdc_input_keybinding_t;

#define NCDC_BINDCUR(k, n, f) { .key = { k, '\0' }, .name = n, .handler = (ncdc_keybinding_t) f }
#define NCDC_BINDING(k, n, f) { .key = k, .name = n, .handler = (ncdc_keybinding_t) f }

extern ncdc_input_keybinding_t emacs[];

#endif
