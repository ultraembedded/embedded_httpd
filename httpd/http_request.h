#ifndef __HTTP_REQUEST_H__
#define __HTTP_REQUEST_H__

#include "http_options.h"

//-------------------------------------------------------
// Types:
//-------------------------------------------------------

// Request type (GET or POST)
typedef enum eHttpRequestType { HTTP_REQ_NONE, HTTP_REQ_GET, HTTP_REQ_POST } tHttpRequestType;

// Auth type
typedef enum eHttpAuthType { HTTP_AUTH_NONE, HTTP_AUTH_BASIC } tHttpAuthType;

//-------------------------------------------------------
// Structures:
//-------------------------------------------------------
struct http_request
{
    // Request type
    tHttpRequestType    request_type;

    // Path of request
    char                path[HTTP_OPT_MAX_PATH_LENGTH];

#if HTTP_OPT_MAX_ARGS_LENGTH != 0
    // Request args
    char                args[HTTP_OPT_MAX_ARGS_LENGTH];
#endif

    // Length of attached data
    int                 content_length;

#if HTTP_OPT_SUPPORT_AUTH
    // Authorization
    tHttpAuthType       auth_type;
    char                auth_string[HTTP_OPT_MAX_AUTH_STR];
#endif
};

//-------------------------------------------------------
// Prototypes:
//-------------------------------------------------------
void http_request_init(struct http_request *request);
void http_request_process_line(struct http_request *request, char* line);

#endif