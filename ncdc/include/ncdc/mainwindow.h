#ifndef NCDC_MAINWINDOW_H
#define NCDC_MAINWINDOW_H

#include <ncdc/ncdc.h>

struct ncdc_mainwindow_;
typedef struct ncdc_mainwindow_ *ncdc_mainwindow_t;

ncdc_mainwindow_t ncdc_mainwindow_new(void);

void ncdc_mainwindow_refresh(ncdc_mainwindow_t n);
void ncdc_mainwindow_input_ready(ncdc_mainwindow_t n);

#endif
