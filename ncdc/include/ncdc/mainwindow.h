#ifndef NCDC_MAINWINDOW_H
#define NCDC_MAINWINDOW_H

#include <ncdc/ncdc.h>
#include <stdarg.h>

struct ncdc_mainwindow_;
typedef struct ncdc_mainwindow_ *ncdc_mainwindow_t;

ncdc_mainwindow_t ncdc_mainwindow_new(void);

/* holy shit stains I am lazy
 */
#define LOG(n, ...) ncdc_mainwindow_log(n, __VA_ARGS__)
void ncdc_mainwindow_log(ncdc_mainwindow_t w, wchar_t const *fmt, ...);

GPtrArray *ncdc_mainwindow_views(ncdc_mainwindow_t n);
dc_channel_t ncdc_mainwindow_current_channel(ncdc_mainwindow_t n);
void ncdc_mainwindow_switchview(ncdc_mainwindow_t n, int idx);

void ncdc_mainwindow_refresh(ncdc_mainwindow_t n);
void ncdc_mainwindow_input_ready(ncdc_mainwindow_t n);

void ncdc_mainwindow_rightview(ncdc_mainwindow_t n);
void ncdc_mainwindow_leftview(ncdc_mainwindow_t n);

#endif
