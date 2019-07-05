#include <ncdc/keycodes.h>

#include <ncdc/mainwindow.h>
#include <ncdc/input.h>

ncdc_keybinding_t *
ncdc_find_keybinding(ncdc_keybinding_t *keys, wchar_t const *key, size_t l)
{
    size_t i = 0;

    for (i = 0; keys[i].name != NULL; i++) {
        if ((l == sizeof(wchar_t) && key[0] == keys[i].key[0]) ||
            wcscmp(key, keys[i].key) == 0) {
            return keys+i;
        }
    }

    return NULL;
}

ncdc_keybinding_t keys_mainwin[] = {
    /* ALT+KEY_RIGHT
     */
    NCDC_BINDING(L"\x1B[1;3C",     L"right-window", ncdc_mainwindow_rightview),
    /* ALT+KEY_LEFT
     */
    NCDC_BINDING(L"\x1B[1;3D",     L"left-window",  ncdc_mainwindow_leftview),
    NCDC_BINDEND()
};

ncdc_keybinding_t keys_emacs[] = {
    /* key left
     */
    NCDC_BINDCUR(KEY_LEFT,         L"backward", ncdc_input_backward),
    /* key right
     */
    NCDC_BINDCUR(KEY_RIGHT,        L"forward",  ncdc_input_forward),
    /* CTRL+F
     */
    NCDC_BINDING(L"\x06",          L"forward",  ncdc_input_forward),
    /* CTRL+B
     */
    NCDC_BINDING(L"\x02",          L"backward", ncdc_input_backward),
    /* CTRL+D
     */
    NCDC_BINDING(L"\x04",          L"delete",   ncdc_input_delete),
    NCDC_BINDCUR(KEY_BACKSPACE,    L"delete",   ncdc_input_delete_backward),
    NCDC_BINDEND()
};
