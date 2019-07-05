#ifndef NCDC_KEYCODES_H
#define NCDC_KEYCODES_H

#include <ncdc/ncdc.h>

typedef void (*ncdc_keyhandler_t)(void *p);

typedef struct {
    wchar_t key[10];
    wchar_t const *name;
    ncdc_keyhandler_t handler;
} ncdc_keybinding_t;

#define NCDC_BINDCUR(k, n, f) { .key = { k, '\0' }, .name = n, .handler = (ncdc_keyhandler_t) f }
#define NCDC_BINDING(k, n, f) { .key = k, .name = n, .handler = (ncdc_keyhandler_t) f }
#define NCDC_BINDEND()        { .key = {0}, .name = NULL, .handler = NULL }

extern ncdc_keybinding_t keys_emacs[];
extern ncdc_keybinding_t keys_mainwin[];

ncdc_keybinding_t *ncdc_find_keybinding(ncdc_keybinding_t *keys,
                                        wchar_t const *key, size_t l);

#endif
