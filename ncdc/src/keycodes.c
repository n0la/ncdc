#include <ncdc/keycodes.h>

#include <ncdc/mainwindow.h>
#include <ncdc/input.h>
#include <ncdc/treeview.h>

ncdc_keybinding_t *
ncdc_find_keybinding(ncdc_keybinding_t *keys, wchar_t const *key, size_t l)
{
    size_t i = 0;

    for (i = 0; keys[i].name != NULL; i++) {
        if ((l == 1 && key[0] == keys[i].key[0]) ||
            wcscmp(key, keys[i].key) == 0) {
            return keys+i;
        }
    }

    return NULL;
}

ncdc_keybinding_t keys_guilds[] = {
    /* CTRL+KEY_UP
     */
    NCDC_BINDING(L"\x1B[1;5A",  L"previous-item", ncdc_treeview_previous),
    NCDC_BINDING(L"\x1BOA",     L"previous-item", ncdc_treeview_previous),
    /* CTRL+KEY_DOWN
     */
    NCDC_BINDING(L"\x1B[1;5B",  L"next-item", ncdc_treeview_next),
    NCDC_BINDING(L"\x1BOB",     L"next-item", ncdc_treeview_next),
    /* CTRL+KEY_RIGHT
     */
    NCDC_BINDING(L"\x1B[1;5C",  L"expand-item", ncdc_treeview_expand),
    NCDC_BINDING(L"\x1BOC",     L"expand-item", ncdc_treeview_expand),
    /* CTRL+KEY_LEFT
     */
    NCDC_BINDING(L"\x1B[1;5D",  L"collapse-item", ncdc_treeview_collapse),
    NCDC_BINDING(L"\x1BOD",     L"collapse-item", ncdc_treeview_collapse),
    NCDC_BINDEND()
};

ncdc_keybinding_t keys_chat[] = {
    NCDC_BINDEND()
};

ncdc_keybinding_t keys_global[] = {
    /* ALT+KEY_RIGHT
     */
    NCDC_BINDING(L"\x1B[1;3C",  L"right-window", ncdc_mainwindow_rightview),
    /* ALT+KEY_LEFT
     */
    NCDC_BINDING(L"\x1B[1;3D",  L"left-window",  ncdc_mainwindow_leftview),
    /* F1
     */
    NCDC_BINDING(L"\x1BOP",     L"select-input", ncdc_mainwindow_switch_input),
    /* F2
     */
    NCDC_BINDING(L"\x1BOQ",     L"select-guilds", ncdc_mainwindow_switch_guilds),
    /* F3
     */
    NCDC_BINDING(L"\x1BOR",     L"select-chat", ncdc_mainwindow_switch_chat),

    NCDC_BINDEND()
};

ncdc_keybinding_t keys_emacs[] = {
    /* key left
     */
    NCDC_BINDING(L"\x1BOD",      L"backward", ncdc_input_backward),
    /* key right
     */
    NCDC_BINDING(L"\x1BOC",      L"forward",  ncdc_input_forward),
    /* CTRL+F
     */
    NCDC_BINDING(L"\x06",        L"forward",  ncdc_input_forward),
    /* CTRL+B
     */
    NCDC_BINDING(L"\x02",        L"backward", ncdc_input_backward),
    /* CTRL+K
     */
    NCDC_BINDING(L"\x0B",        L"kill-right", ncdc_input_kill_right),
    /* CTRL+U
     */
    NCDC_BINDING(L"\x15",        L"kill-left", ncdc_input_kill_left),
    /* CTRL+W
     */
    NCDC_BINDING(L"\x17",        L"kill-word-left", ncdc_input_kill_word_left),
    /* CTRL+D
     */
    NCDC_BINDING(L"\x04",        L"delete",   ncdc_input_delete),
    /* BACKSPACE
     */
    NCDC_BINDING(L"\x7F",        L"delete",   ncdc_input_delete_backward),
    NCDC_BINDEND()
};
