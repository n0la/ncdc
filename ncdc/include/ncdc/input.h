#ifndef NCDC_INPUT_H
#define NCDC_INPUT_H

#include <ncdc/ncdc.h>
#include <ncdc/keycodes.h>

struct ncdc_input_;
typedef struct ncdc_input_ *ncdc_input_t;

typedef bool (*ncdc_input_callback_t)(ncdc_input_t p, wchar_t const *str,
                                      size_t len, void *data);

ncdc_input_t ncdc_input_new(void);

void ncdc_input_feed(ncdc_input_t input, wchar_t const *c, size_t sz);
int ncdc_input_cursor(ncdc_input_t input);
char const *ncdc_input_buffer(ncdc_input_t input);
void ncdc_input_draw(ncdc_input_t input, WINDOW *win);

void ncdc_input_set_callback(ncdc_input_t i, ncdc_input_callback_t c, void *a);

/* keybinding functions
 */
void ncdc_input_backward(ncdc_input_t i);
void ncdc_input_forward(ncdc_input_t i);
void ncdc_input_delete(ncdc_input_t input);
void ncdc_input_delete_backward(ncdc_input_t input);

#endif
