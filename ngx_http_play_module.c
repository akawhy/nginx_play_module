#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

#include "ddebug.h"

typedef struct {
    ngx_flag_t  enable;

    ngx_str_t   display_varname;
} ngx_http_play_loc_conf_t;


static void *ngx_http_play_create_loc_conf(ngx_conf_t *cf);

static char *ngx_conf_play(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static ngx_int_t ngx_http_play_handler(ngx_http_request_t *r);


static ngx_command_t ngx_http_play_commands[] = {
    {
        ngx_string("play"),
        NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
        ngx_conf_play,
        NGX_HTTP_LOC_CONF_OFFSET,
        0,
        NULL
    },
    {
        ngx_string("play_print_var"),
        NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
        ngx_conf_set_str_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_http_play_loc_conf_t, display_varname),
        NULL
    },
    ngx_null_command
};

static ngx_http_module_t ngx_http_play_module_ctx = {
    NULL,                              /* preconfiguration */
    //ngx_http_play_init,              /* postconfiguration */
    NULL,                              /* postconfiguration */

    NULL,                              /* create main configuration */
    NULL,                              /* init main configuration */

    NULL,                              /* create server configuration */
    NULL,                              /* merge server configuration */

    ngx_http_play_create_loc_conf,     /* create location configuration */
    NULL                               /* merge location configuration */
};

ngx_module_t  ngx_http_play_module = {
    NGX_MODULE_V1,
    &ngx_http_play_module_ctx,                      /* module context */
    ngx_http_play_commands,                         /* module directives */
    NGX_HTTP_MODULE,                                /* module type */
    NULL,                                           /* init master */
    NULL,                                           /* init module */
    NULL,                                           /* init process */
    NULL,                                           /* init thread */
    NULL,                                           /* exit thread */
    NULL,                                           /* exit process */
    NULL,                                           /* exit master */
    NGX_MODULE_V1_PADDING
};

static char *
ngx_conf_play(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_play_loc_conf_t *plcf;
    ngx_http_core_loc_conf_t *clcf;
    ngx_str_t                *value;

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    plcf = conf;
    
    value = cf->args->elts;

    ngx_conf_log_error(NGX_LOG_INFO, cf, 0, "play: %s", value[1].data);

    plcf->enable = 0;
    if (ngx_strncmp(value[1].data, "on", 2) == 0) {
        clcf->handler = ngx_http_play_handler;
        plcf->enable  = 1;
    }

    return NGX_CONF_OK;
}

/* the main function */
static ngx_int_t
ngx_http_play_handler(ngx_http_request_t *r)
{
    ngx_int_t    rc;
    ngx_buf_t   *b;
    ngx_chain_t  out;
    ngx_str_t    v;
    ngx_variable_value_t *var;

    ngx_http_play_loc_conf_t *plcf;

    plcf = ngx_http_get_module_loc_conf(r, ngx_http_play_module);
    
    /* here we go ;) */
    if (plcf->display_varname.len == 0) {
        return NGX_ERROR;
    }


    rc = ngx_http_discard_request_body(r);
    if (rc != NGX_OK) {
        return rc;
    }

    ngx_str_set(&r->headers_out.content_type, "text/html");

    b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));
    if (b == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    out.buf  = b;
    out.next = NULL;

    /*
     *
     * I want to change here ;)
     *
     * */

    v = plcf->display_varname;

    /* set the value */
    var = ngx_http_get_variable(r, &v, ngx_hash_key(v.data, v.len));
    if (var == NULL || var->not_found) {
        b->pos  = (u_char *)"not found";
        b->last = b->pos + 9;
    }
    else {
        b->pos  = var->data;
        b->last = var->data + var->len;
    }


    ///////////////////////////////////////////////////////////////////////////////////////////////

    b->memory = 1;
    b->last_buf = 1;

    r->headers_out.status = NGX_HTTP_OK;
    //r->headers_out.content_length_n = var.len;

    rc = ngx_http_send_header(r);
    return ngx_http_output_filter(r, &out);
}


static void *
ngx_http_play_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_play_loc_conf_t *plcf =  NULL;

    plcf = ngx_pcalloc(cf->pool, sizeof(ngx_http_play_loc_conf_t));

    if (plcf == NULL) {
        return NULL;
    }

    return plcf;
}
