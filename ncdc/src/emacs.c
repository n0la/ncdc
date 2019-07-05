#include <ncdc/keycodes.h>

ncdc_input_keybinding_t emacs[] = {
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
    NCDC_BINDING({0}, NULL, NULL)
};
