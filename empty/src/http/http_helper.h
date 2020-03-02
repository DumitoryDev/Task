#ifndef HTTPHELPER
#define HTTPHELPER

#include "../../include/pch.h"
#include "../../include/http_header.h"


typedef struct setting_http{

    const char *request;
    char * output; 
    
} setting_http, * psetting_http;

void send_http_get_request(psetting_http data);
void register_http(void);


#endif


