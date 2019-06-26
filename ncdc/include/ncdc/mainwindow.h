#ifndef NCDC_MAINWINDOW_H
#define NCDC_MAINWINDOW_H

#include <ncdc/ncdc.h>

struct ncdc_mainwindow_;
typedef struct ncdc_mainwindow_ *ncdc_mainwindow_t;

bool ncdc_mainwindow_init(void);

void ncdc_mainwindow_feed(int ch);
void ncdc_mainwindow_refresh(void);

#endif
