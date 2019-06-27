#include <ncdc/input.h>

ncdc_input_keybinding_t emacs[] = {
    NCDC_BINDING("KEY_RIGHT",     "forward",  ncdc_input_forward),
    NCDC_BINDING("KEY_LEFT",      "backward", ncdc_input_backward),
    NCDC_BINDING("^F",            "forward",  ncdc_input_forward),
    NCDC_BINDING("^B",            "backward", ncdc_input_backward),
    NCDC_BINDING("^D",            "delete",   ncdc_input_delete),
    NCDC_BINDING("KEY_BACKSPACE", "delete",   ncdc_input_delete_backward),
    NCDC_BINDING(NULL, NULL, NULL)
};
