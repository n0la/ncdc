#include <ncdc/config.h>
#include <confuse.h>

static cfg_opt_t account_opts[] = {
    CFG_STR("email", NULL, CFGF_NONE),
    CFG_STR("password", NULL, CFGF_NONE),
    CFG_END()
};

static cfg_opt_t opts[] = {
    CFG_SEC("account", account_opts, CFGF_TITLE|CFGF_MULTI),
    CFG_END()
};

struct ncdc_config_
{
    dc_refable_t ref;

    char *configpath;
    cfg_t *cfg;
};

static void ncdc_config_free(ncdc_config_t c)
{
    return_if_true(c == NULL,);

    cfg_free(c->cfg);
    free(c->configpath);
    free(c);
}

ncdc_config_t ncdc_config_new(void)
{
    ncdc_config_t c = calloc(1, sizeof(struct ncdc_config_));
    return_if_true(c == NULL, NULL);

    c->ref.cleanup = (dc_cleanup_t)ncdc_config_free;

    c->cfg = cfg_init(opts, CFGF_NONE);
    if (c->cfg == NULL) {
        free(c);
        return NULL;
    }

    asprintf(&c->configpath, "%s/config", ncdc_private_dir);
    if (cfg_parse(c->cfg, c->configpath) == CFG_PARSE_ERROR) {
        free(c->configpath);
        cfg_free(c->cfg);
        free(c);
        return NULL;
    }

    return c;
}

dc_account_t ncdc_config_account(ncdc_config_t c, char const *name)
{
    cfg_t *cfg = NULL;
    dc_account_t acc = NULL;
    size_t i = 0;

    return_if_true(c == NULL || name == NULL, NULL);

    for (i = 0; i < cfg_size(c->cfg, "account"); i++) {
        cfg = cfg_getnsec(c->cfg, "account", i);
        if (strcmp(cfg_title(cfg), name) == 0) {
            /* entry has been found
             */
            char const *email = cfg_getstr(cfg, "email");
            char const *password = cfg_getstr(cfg, "password");

            if (email == NULL || password == NULL) {
                continue;
            }

            acc = dc_account_new2(email, password);
            break;
        }
    }

    return acc;
}
