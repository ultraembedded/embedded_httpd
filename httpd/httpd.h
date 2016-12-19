#ifndef __HTTPD_H__
#define __HTTPD_H__

#include "http_options.h"
#include "http_request.h"
#include "http_input.h"

//-------------------------------------------------------
// Structures:
//-------------------------------------------------------

// HTTPD server session
struct http_session
{
    // Current socket identifier
    int                        socket;

    // Incoming command request buffer
    struct http_input_buffer   input_buffer;

    // Parsed HTTP request
    struct http_request        request;

    // Output buffer
    unsigned char              write_buffer[HTTP_OPT_OUTPUT_BUFFER_SIZE];
    int                        write_buf_count;

#if HTTP_OPT_SUPPORT_POST 
    // Current post data receive length
    int                        http_post_length;

#if HTTP_OPT_MAX_POST_DATA != 0
    // Post data target buffer
    char                       http_post_data[HTTP_OPT_MAX_POST_DATA];
#endif

    // Post data receive function
    void                      (*http_on_post_data)(void *session, char *str, int len);
#endif

    // Authorization Support
#if HTTP_OPT_SUPPORT_AUTH
    // Is login required?
    int                        http_use_auth;
    
    // Login details
    char                       http_username[HTTP_OPT_MAX_USERNAME];
    char                       http_password[HTTP_OPT_MAX_PASSWORD];
#endif
};

// System API used by HTTPD
struct http_sysapi
{
    // Required API
    void *     (*http_fopen) (char * name, char * mode);
    int        (*http_fread) (char * buf, unsigned size1, unsigned size2, void * file);
    int        (*http_fclose)(void * file);
    int        (*http_send) (int s, const char * buf, int len, int flags);

    // Optional API
    int        (*http_cgi) (struct http_session *p, char * path, char * args);
};

//-------------------------------------------------------
// Prototypes:
//-------------------------------------------------------

// Initialisation
void http_init( void *    (*_open) (char * name, char * mode),
                int       (*_read) (char * buf, unsigned size1, unsigned size2, void * file),
                int       (*_close)(void * file),
                int       (*_send) (int s, const char * buf, int len, int flags) );

// Attach CGI handler
void http_attach_cgi_handler( int (*cgi_handler) (struct http_session *p, char * path, char * args) );

// HTTP auth required
void http_set_auth(struct http_session *session, const char *user, const char *pass);

// Initialise session connection
void http_new_connection(struct http_session *p, int s);

// Process incoming data for a session
int http_process_data(struct http_session *p, char *buf, int len);

// HTTP output functions
int http_send_file(struct http_session *session, char *path);
int http_read_file(struct http_session *session, char *path, int (*reader)(struct http_session *p, char * str, int len));
int http_send_response(struct http_session *session, const char *title, const char *mime, char *buffer, int length);
int http_send_start(struct http_session *session, const char *title);
int http_send_html(struct http_session *session, const char *buf);
int http_send_end(struct http_session *session);
int http_send_redirect(struct http_session *session, char *location);

// Low level send data function
int http_send_data(int socket, char *str, int length);
 
#endif
