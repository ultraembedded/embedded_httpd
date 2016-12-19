//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//                             Embedded HTTP Server
//                                    V0.2
//                              Ultra-Embedded.com
//                            Copyright 2010 - 2016
//
//                         Email: admin@ultra-embedded.com
//
//                                License: GPL
//   If you would like a version with a more permissive license for use in
//   closed source commercial applications please contact me for details.
//-----------------------------------------------------------------------------
//
// This file is part of Embedded HTTP Server.
//
// Embedded HTTP Server is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Embedded HTTP Server is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Embedded HTTP Server; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#include "http_options.h"
#include "httpd.h"
#include "http_output.h"
#include "http_auth.h"

//-------------------------------------------------------
// Defines:
//-------------------------------------------------------
#define HTTP_SRVR                "simplewebserver/1.0"
#define HTTP_PROTOCOL            "HTTP/1.0"

//-------------------------------------------------------
// Prototypes:
//-------------------------------------------------------
static void http_line(void *session, char* line);
static void http_request_end(void *session);
static void http_data(void *session, char* data, int len);
static void http_process_request(struct http_session *session);

//-------------------------------------------------------
// System API:
//-------------------------------------------------------
static struct http_sysapi httpd_sysapi = { NULL, NULL, NULL, NULL, NULL };

//-------------------------------------------------------
// http_send_data: Low level network send for http data
//-------------------------------------------------------
int http_send_data(int socket, char *str, int length)
{
    int res = 0;

    if (httpd_sysapi.http_send)
        res = httpd_sysapi.http_send(socket, str, length, 0);

    return res;
}
//-------------------------------------------------------
// http_get_mime_type:
//-------------------------------------------------------
static char *http_get_mime_type(char *name)
{
    // Find last '.'
    char *ext = httpd_strrchr(name, '.');

    if (!ext) 
        return NULL;

    if (httpd_stricmp(ext, ".html") == 0 || httpd_stricmp(ext, ".htm") == 0) return "text/html";
    if (httpd_stricmp(ext, ".shtml") == 0 || httpd_stricmp(ext, ".cgi") == 0) return "text/html";
    if (httpd_stricmp(ext, ".jpg") == 0 || httpd_stricmp(ext, ".jpeg") == 0) return "image/jpeg";
    if (httpd_stricmp(ext, ".gif") == 0) return "image/gif";
    if (httpd_stricmp(ext, ".png") == 0) return "image/png";
    if (httpd_stricmp(ext, ".css") == 0) return "text/css";
    if (httpd_stricmp(ext, ".au") == 0) return "audio/basic";
    if (httpd_stricmp(ext, ".wav") == 0) return "audio/wav";
    if (httpd_stricmp(ext, ".avi") == 0) return "video/x-msvideo";
    if (httpd_stricmp(ext, ".mpeg") == 0 || httpd_stricmp(ext, ".mpg") == 0) return "video/mpeg";
    if (httpd_stricmp(ext, ".mp3") == 0) return "audio/mpeg";

    return NULL;
}
//-------------------------------------------------------
// http_send_header:
//-------------------------------------------------------
static void http_send_header(struct http_session *session, int status, char *title, char *extra, char *mime, int length)
{
    http_output_printf(session, "%s %d %s\r\n", HTTP_PROTOCOL, status, title);
    http_output_printf(session, "Server: %s\r\n", HTTP_SRVR);
    if (extra) http_output_printf(session, "%s\r\n", extra);
#if HTTP_OPT_NO_CACHE_PRAGMA
    http_output_printf(session, "Pragma: no-cache\r\n");
#endif
    if (mime) http_output_printf(session, "Content-Type: %s\r\n", mime);
    if (length >= 0) http_output_printf(session, "Content-Length: %d\r\n", length);
    http_output_printf(session, "Connection: close\r\n");
    http_output_printf(session, "\r\n");
}
//-------------------------------------------------------
// http_send_error:
//-------------------------------------------------------
static void http_send_error(struct http_session *session, int status, char *title, char *extra, char *text)
{
    http_send_header(session, status, title, extra, "text/html", -1);
    http_output_printf(session, "<HTML><HEAD><TITLE>%d %s</TITLE></HEAD>\r\n", status, title);
    http_output_printf(session, "<BODY><H4>%d %s</H4>\r\n", status, title);
    http_output_printf(session, "%s\r\n", text);
    http_output_printf(session, "</BODY></HTML>\r\n");
}
//-------------------------------------------------------
// http_send_redirect:
//-------------------------------------------------------
int http_send_redirect(struct http_session *session, char *location)
{
    http_output_printf(session, "%s %d %s\r\n", HTTP_PROTOCOL, 303, "See Other");
    http_output_printf(session, "Location: %s\r\n", location);
    http_output_printf(session, "\r\n");
    return 1;
}
//-------------------------------------------------------
// http_send_auth_request:
//-------------------------------------------------------
#if HTTP_OPT_SUPPORT_AUTH
static void http_send_auth_request(struct http_session *session, char *realm)
{
    http_output_printf(session, "%s %d %s\r\n", HTTP_PROTOCOL, 401, "Login");
    http_output_printf(session, "Server: %s\r\n", HTTP_SRVR);
    http_output_printf(session, "WWW-Authenticate: Basic realm=\"%s\"\r\n", realm);
    http_output_printf(session, "Content-Type: text/html\r\n");
    http_output_printf(session, "Connection: close\r\n");
    http_output_printf(session, "\r\n");
}
#endif
//-------------------------------------------------------
// http_init:
//-------------------------------------------------------
void http_init( void *    (*_open) (char * name, char * mode),
                int       (*_read) (char * buf, unsigned size1, unsigned size2, void * file),
                int       (*_close)(void * file),
                int       (*_send) (int s, const char * buf, int len, int flags) )
{
    httpd_sysapi.http_fopen = _open;
    httpd_sysapi.http_fread = _read;
    httpd_sysapi.http_fclose = _close;
    httpd_sysapi.http_send = _send;
    httpd_sysapi.http_cgi = NULL;
}
//-------------------------------------------------------
// http_attach_cgi_handler:
//-------------------------------------------------------
void http_attach_cgi_handler( int (*cgi_handler) (struct http_session *p, char * path, char * args) )
{
    httpd_sysapi.http_cgi = cgi_handler;
}
//-------------------------------------------------------
// http_new_connection:
//-------------------------------------------------------
void http_new_connection(struct http_session *p, int s)
{
    // Init input buffer
    http_input_init(&p->input_buffer, http_line, http_request_end, http_data);

    // Init request parser
    http_request_init(&p->request);

    p->socket = s;

#if HTTP_OPT_SUPPORT_POST 
    #if HTTP_OPT_MAX_POST_DATA != 0
    p->http_post_data[0] = 0;
    #endif
    p->http_post_length = 0;
    p->http_on_post_data = 0;
#endif

#if HTTP_OPT_SUPPORT_AUTH

    // NOTE: We are resetting auth settings so everytime http_new_connection()
    // is called from webserver thread, any authentication setup functions must
    // also be called to re-establish the security settings...
    p->http_use_auth = 0;
#endif
}
//-------------------------------------------------------
// http_process_data: Process received data
// Returns: 0 = No more data required/expected
//            1 = More data required
//-------------------------------------------------------
int http_process_data(struct http_session *p, char *buf, int len)
{
    // Pass data to HTTP request buffer
    if (!http_input_process(p, &p->input_buffer, buf, len))
        p->socket = -1; // Error

    // Socket marked as close required
    if (p->socket == -1)
        return 0;
    // Else more data expected...
    else
        return 1;
}
//-------------------------------------------------------
// http_line: On receiving a line of a HTTP request
//-------------------------------------------------------
static void http_line(void *session, char* line)
{
    struct http_session *p = (struct http_session *)session;

    // Pass to request parser
    http_request_process_line(&p->request, line);
}
//-------------------------------------------------------
// http_request_end:
//-------------------------------------------------------
static void http_request_end(void *session)
{
    struct http_session *p = (struct http_session *)session;

    // If GET request (POST request called after data received)
    if (p->request.request_type != HTTP_REQ_POST)
    {
        http_process_request(p);
    
        // Mark close connection required
        p->socket = -1;
    }
}
//-------------------------------------------------------
// http_data:
//-------------------------------------------------------
static void http_data(void *session, char* data, int len)
{
    struct http_session *p = (struct http_session *)session;

#if HTTP_OPT_SUPPORT_POST 

    // This should not happen!
    if (p->request.request_type != HTTP_REQ_POST)
        return ;

    // Post data receive to function
    if (p->http_on_post_data)
    {
        // Call user callback
        p->http_on_post_data(p, data, len);
        p->http_post_length += len;

        // POST complete?
        if (p->http_post_length >= p->request.content_length)
        {
            http_process_request(p);

            // Mark close connection required
            p->socket = -1;
        }
    }
    else
    {
        while (len)
        {
        #if HTTP_OPT_MAX_POST_DATA != 0
            if (p->http_post_length < (HTTP_OPT_MAX_POST_DATA-1))
                p->http_post_data[p->http_post_length++] = *data++;

            len--;
        #else
            p->http_post_length += len;
            len = 0;
        #endif
        }

        // POST complete?
        if (p->http_post_length >= p->request.content_length || p->http_post_length == (HTTP_OPT_MAX_POST_DATA-1))
        {
        #if HTTP_OPT_MAX_POST_DATA != 0
            // Null terminate post data
            p->http_post_data[p->http_post_length] = 0;
        #endif

            http_process_request(p);

            // Mark close connection required
            p->socket = -1;
        }    
    }
#else
    // Else we dont support this method so let process_request
    // deal with the error message...

    http_process_request(p);

    // Mark close connection required
    p->socket = -1;
#endif
}
//-------------------------------------------------------
// http_process_request:
//-------------------------------------------------------
static void http_process_request(struct http_session *session)
{
    char *path;
    int res = 0;

    // Initialise buffering system
    http_output_init(session);

    // Require login?
#if HTTP_OPT_SUPPORT_AUTH
    if (session->http_use_auth)
    {
        // Try and validate current user details against session
        if (!http_auth_validate(session))
        {
            // Send request for auth
            http_send_auth_request(session, "website");
            http_output_flush(session);
            return ;
        }
    }
#endif

    // Verify that this is a valid request
#if HTTP_OPT_SUPPORT_POST
    if (session->request.request_type != HTTP_REQ_GET && session->request.request_type != HTTP_REQ_POST)
#else
    if (session->request.request_type != HTTP_REQ_GET)
#endif
    {
        http_send_error(session, 501, "Not supported", NULL, "Method is not supported.");
        http_output_flush(session);
        return ;
    }

    // Was the path just '/', redirect to index page
    if (session->request.path[0] == '/' && session->request.path[1] == '\0')
        path = HTTP_OPT_DEFAULT_FILE;
    else
        path = session->request.path;

    // Try using CGI handler to see if this is a virtual file first
    if (httpd_sysapi.http_cgi)
    {
    #if HTTP_OPT_MAX_ARGS_LENGTH != 0
        res = httpd_sysapi.http_cgi(session, path, session->request.args);
    #else
        res = httpd_sysapi.http_cgi(session, path, 0);
    #endif
    }

    // Virtual/CGI file was not found/sent
    if (!res)
    {
        // Try and open the path specified 
        res = http_send_file(session, path);
    }

    // File was not found/sent an no CGI handler was invoked
    if (!res)
    {
        // Otherwise send a 404 message
        http_send_error(session, 404, "Not Found", NULL, "File not found.");
    }

    // Flush output buffer
    http_output_flush(session);
}
//-------------------------------------------------------
// http_send_file:
//-------------------------------------------------------
int http_send_file(struct http_session *session, char *path)
{
    char file_data[HTTP_OPT_FILE_CHUNKSIZE];
    int n;
    void* file;

#if HTTP_OPT_USE_PATH
    char fullname[HTTP_OPT_MAX_PATH_LENGTH+sizeof(HTTP_OPT_FILE_PATH)+1];
#endif

    // No file API?
    if (!httpd_sysapi.http_fopen || !httpd_sysapi.http_fread || !httpd_sysapi.http_fclose)
        return 0;

    // Attach a directory path to the file request?
#if HTTP_OPT_USE_PATH
    httpd_strcpy(fullname, HTTP_OPT_FILE_PATH);
    httpd_strcat(fullname, path);
    path = fullname;
#else
    // Relative path - skip leading slash
    if (*path == '/')
        path ++;
#endif
    
    // Open file
    file = httpd_sysapi.http_fopen(path, "rb");

    // File open failed
    if (!file)
        return 0;
    // File opened
    else
    {
        // Send HTTP headers
        // NOTE: If you know the file length, pass in the length!
        http_send_header(session, 200, "OK", NULL, http_get_mime_type(path), -1);

        // Read in data & send via socket
        while ((n = httpd_sysapi.http_fread(file_data, 1, sizeof(file_data), file)) > 0) 
        {
            http_output_write(session, file_data, n);
        }

        // Close file
        httpd_sysapi.http_fclose(file);
        return 1;
    }
}
//-------------------------------------------------------
// http_read_file:
//-------------------------------------------------------
int http_read_file(struct http_session *session, char *path, int (*reader)(struct http_session *p, char * str, int len))
{
    char file_data[HTTP_OPT_FILE_CHUNKSIZE];
    int n;
    void* file;
#if HTTP_OPT_USE_PATH
    char fullname[HTTP_OPT_MAX_PATH_LENGTH+sizeof(HTTP_OPT_FILE_PATH)+1];
#endif

    // No file API?
    if (!httpd_sysapi.http_fopen || !httpd_sysapi.http_fread || !httpd_sysapi.http_fclose)
        return 0;

    // No reader function?
    if (!reader)
        return 0;

    // Attach a directory path to the file request?
#if HTTP_OPT_USE_PATH
    httpd_strcpy(fullname, HTTP_OPT_FILE_PATH);
    httpd_strcat(fullname, path);
    path = fullname;
#endif
    
    // Open file
    file = httpd_sysapi.http_fopen(path, "rb");

    // File open failed
    if (!file)
        return 0;
    // File opened
    else
    {
        // Send HTTP headers
        // NOTE: If you know the file length, pass in the length!
        http_send_header(session, 200, "OK", NULL, http_get_mime_type(path), -1);

        // Read in data & send via socket
        while ((n = httpd_sysapi.http_fread(file_data, 1, sizeof(file_data), file)) > 0) 
        {
            reader(session, file_data, n);
        }

        // Close file
        httpd_sysapi.http_fclose(file);
        return 1;
    }
}
//-------------------------------------------------------
// http_send_start:
//-------------------------------------------------------
int http_send_start(struct http_session *session, const char *title)
{
    http_output_init(session);
    http_send_header(session, 200, "OK", (char*)title, "text/html", -1);
    return 1;
}
//-------------------------------------------------------
// http_send_response:
//-------------------------------------------------------
int http_send_response(struct http_session *session, const char *title, const char *mime, char *buffer, int length)
{
    http_output_init(session);
    http_send_header(session, 200, "OK", (char*)title, (char*)mime, -1);
    http_output_write(session, buffer, length);
    http_output_flush(session);
    return 1;
}
//-------------------------------------------------------
// http_send_html:
//-------------------------------------------------------
int http_send_html(struct http_session *session, const char *buf)
{
    http_output_printf(session, buf);
    return 1;
}
//-------------------------------------------------------
// http_send_end:
//-------------------------------------------------------
int http_send_end(struct http_session *session)
{
    http_output_flush(session);
    return 1;
}
//-------------------------------------------------------
// http_set_auth:
//-------------------------------------------------------
void http_set_auth(struct http_session *session, const char *user, const char *pass)
{
#if HTTP_OPT_SUPPORT_AUTH
    if (user && pass)
    {
        httpd_strncpy(session->http_username, user, HTTP_OPT_MAX_USERNAME);
        httpd_strncpy(session->http_password, pass, HTTP_OPT_MAX_PASSWORD);

        session->http_username[HTTP_OPT_MAX_USERNAME-1] = 0;
        session->http_password[HTTP_OPT_MAX_PASSWORD-1] = 0;
        
        session->http_use_auth = 1;
    }
    else
        session->http_use_auth = 0;
#endif
}
