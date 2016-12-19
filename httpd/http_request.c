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
#include "http_request.h"

//-------------------------------------------------------
// Defines:
//-------------------------------------------------------
#define HTTP_LINE_GET               "GET "
#define HTTP_LINE_POST              "POST "
#define HTTP_LINE_CONTENT_LEN       "Content-Length: "
#define HTTP_LINE_AUTH_BASIC        "Authorization: Basic "

//-------------------------------------------------------
// Locals:
//-------------------------------------------------------
static int string_startswith(char *line, const char *token);
static char* string_copyuntil(char *line, char delimiter, char *target, int max_len);
static unsigned int string_atoi(const char *str);

//-------------------------------------------------------
// http_request_init:
//-------------------------------------------------------
void http_request_init(struct http_request *request)
{
    // Init defaults
    request->request_type = HTTP_REQ_NONE;
    request->path[0] = '\0';
    request->content_length = 0;

#if HTTP_OPT_SUPPORT_AUTH
    request->auth_type = HTTP_AUTH_NONE;
    request->auth_string[0] = '\0';    
#endif

#if HTTP_OPT_MAX_ARGS_LENGTH != 0
    request->args[0] = '\0';
#endif
}
//-------------------------------------------------------
// http_request_process_line:
//-------------------------------------------------------
void http_request_process_line(struct http_request *request, char* line)
{
    // GET string
    if (string_startswith(line, HTTP_LINE_GET))
    {
        line += httpd_strlen(HTTP_LINE_GET);

        // Example: "GET /index.cgi?args HTTP/1.1"
        request->request_type = HTTP_REQ_GET;

        // Extract main URL path
        line = string_copyuntil(line, '?', request->path, HTTP_OPT_MAX_PATH_LENGTH);
        if (line)
        {
        #if HTTP_OPT_MAX_ARGS_LENGTH != 0
            // GET args are present
            if (*line == '?')
                line = string_copyuntil(line + 1, ' ', request->args, HTTP_OPT_MAX_ARGS_LENGTH);
        #endif
        }
    }
    // POST string
    else if (string_startswith(line, HTTP_LINE_POST))
    {
        line += httpd_strlen(HTTP_LINE_POST);

        // Example: "POST /index.cgi?args HTTP/1.1"
        request->request_type = HTTP_REQ_POST;

        // Extract main URL path
        line = string_copyuntil(line, '?', request->path, HTTP_OPT_MAX_PATH_LENGTH);
        if (line)
        {
        #if HTTP_OPT_MAX_ARGS_LENGTH != 0
            // POST get args are present
            if (*line == '?')
                line = string_copyuntil(line + 1, ' ', request->args, HTTP_OPT_MAX_ARGS_LENGTH);
        #endif
        }
    }
    // Content Length
    else if (string_startswith(line, HTTP_LINE_CONTENT_LEN))
    {
        line += httpd_strlen(HTTP_LINE_CONTENT_LEN);
        request->content_length = string_atoi(line);
    }
    // Authorization
    else if (string_startswith(line, HTTP_LINE_AUTH_BASIC))
    {
#if HTTP_OPT_SUPPORT_AUTH
        line += httpd_strlen(HTTP_LINE_AUTH_BASIC);

        request->auth_type = HTTP_AUTH_BASIC;
        string_copyuntil(line, 0, request->auth_string, HTTP_OPT_MAX_AUTH_STR);
#endif
    }
}
//-----------------------------------------------------------------
// string_startswith:
//-----------------------------------------------------------------
static int string_startswith(char *line, const char *token)
{
    const char *s1 = token;
    char *s2 = line;

    // Compare entire 
    while (*s1)
        if (*s1++ != *s2++)
            return 0;

    return 1;
}
//-----------------------------------------------------------------
// string_copyuntil: Copy up until the delimiter is found, end if
// string is reached, or out of space.
// Also treats whitespace as end of string.
// Return: Next position in line or NULL if end reached
//-----------------------------------------------------------------
static char* string_copyuntil(char *line, char delimiter, char *target, int max_len)
{
    char *d = target;
    
    if (!max_len)
        return line;

    while (*line && *line != delimiter && *line != ' ')
    {
        // One less for NULL termination
        if (max_len-1)
        {
            *d++ = *line++;
            max_len--;
        }
        // Even if we run out of buffer space, skip past data
        else
            line++;
    }

    // Null terminate
    *d = 0;

    // More data remains?
    if (*line)
        return line;
    else
        return 0;
}
//-----------------------------------------------------------------
// string_atoi:
//-----------------------------------------------------------------
static unsigned int string_atoi(const char *str)
{
    unsigned int val = 0;

    while ('0' <= *str && *str <= '9') 
    {
        val *= 10;
        val += *str++ - '0';
    }

    return val;
}
