#ifndef __HTTP_INPUT_H__
#define __HTTP_INPUT_H__

#include "http_options.h"

//-------------------------------------------------------
// Defines:
//-------------------------------------------------------

// Request buffer state enum/typedef
typedef enum eHttpRequestState { HTTPREQ_PROC_HEADER, HTTPREQ_PROC_DATA } tHttpRequestState;

//-------------------------------------------------------
// Structures:
//-------------------------------------------------------
struct http_input_buffer
{
    // Request buffer state
    tHttpRequestState buffer_state;

    // Text line buffering
    char    buffer[HTTP_OPT_INPUT_LINE_MAX];
    int     count;

    // On text line call-back
    void (*on_text)(void *session, char* line);

    // On end of HTTP request call-back (prior to possible data)
    void (*on_request_end)(void *session);

    // On data call-back
    void (*on_data)(void *session, char* buf, int length);
};

//-------------------------------------------------------
// Prototypes:
//-------------------------------------------------------

// Initialise HTTP request parser
void http_input_init( struct http_input_buffer *pbuf, void (*on_text)(void *session, char* line), void (*on_request_end)(void *session), void (*on_data)(void *session, char* buf, int length));

// Process received HTTP data
int  http_input_process( void *session, struct http_input_buffer *pbuf, char *s, int length );

#endif