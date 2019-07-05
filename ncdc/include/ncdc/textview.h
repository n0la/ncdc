#ifndef NCDC_TEXTVIEW_H
#define NCDC_TEXTVIEW_H

#include <ncdc/ncdc.h>

struct ncdc_textview_;
typedef struct ncdc_textview_ *ncdc_textview_t;

ncdc_textview_t ncdc_textview_new(void);

dc_account_t ncdc_textview_account(ncdc_textview_t v);
void ncdc_textview_set_account(ncdc_textview_t v, dc_account_t a);

dc_channel_t ncdc_textview_channel(ncdc_textview_t v);
void ncdc_textview_set_channel(ncdc_textview_t v, dc_channel_t a);

void ncdc_textview_append(ncdc_textview_t v, wchar_t const *w);
wchar_t const *ncdc_textview_nthline(ncdc_textview_t v, size_t i);
void ncdc_textview_render(ncdc_textview_t v, WINDOW *win, int lines, int cols);

#endif
