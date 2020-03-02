#ifndef HTTPHELPER
#define HTTPHELPER

#include "../../include/pch.h"
#include "../../include/http_header.h"


typedef struct http_param{

    const char *url;
    const char *post_param;
    
} http_param, * phttp_param;

void register_http(void);


#endif


