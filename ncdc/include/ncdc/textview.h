#ifndef NCDC_TEXTVIEW_H
#define NCDC_TEXTVIEW_H

#include <ncdc/ncdc.h>

struct ncdc_textview_;
typedef struct ncdc_textview_ *ncdc_textview_t;

ncdc_textview_t ncdc_textview_new(void);

void ncdc_textview_append(ncdc_textview_t v, wchar_t const *w);
wchar_t const *ncdc_textview_nthline(ncdc_textview_t v, size_t i);
void ncdc_textview_render(ncdc_textview_t v, WINDOW *win, int lines, int cols);

#endif
