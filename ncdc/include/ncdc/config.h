#ifndef NCDC_CONFIG_H
#define NCDC_CONFIG_H

#include <ncdc/ncdc.h>

struct ncdc_config_;
typedef struct ncdc_config_ *ncdc_config_t;

ncdc_config_t ncdc_config_new(void);

dc_account_t ncdc_config_account(ncdc_config_t c, char const *name);

#endif
