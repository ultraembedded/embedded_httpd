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
#include "http_printf.h"

//-------------------------------------------------------
// http_output_init:
//-------------------------------------------------------
void http_output_init(struct http_session *session)
{
    session->write_buf_count = 0;
}
//-------------------------------------------------------
// http_output_putc:
//-------------------------------------------------------
static int http_output_putc(void *arg, char c)
{
    struct http_session *session = (struct http_session *)arg;

    session->write_buffer[session->write_buf_count++] = c;

    if (session->write_buf_count == HTTP_OPT_OUTPUT_BUFFER_SIZE)
    {
        http_send_data(session->socket, session->write_buffer, session->write_buf_count);
        session->write_buf_count = 0;
    }

    return 0;
}
//-------------------------------------------------------
// http_output_flush:
//-------------------------------------------------------
void http_output_flush(struct http_session *session)
{
    if (session->write_buf_count)
    {
        http_send_data(session->socket, session->write_buffer, session->write_buf_count);
        session->write_buf_count = 0;
    }
}
//-------------------------------------------------------
// http_output_printf:
//-------------------------------------------------------
int http_output_printf(struct http_session *session, const char *fmt, ...)
{
#if HTTPD_OPT_USE_CUSTOM_PRINTF
    va_list argp;
    int res;

    va_start( argp, fmt);
    res = http_xsnprintf(http_output_putc, session, fmt, argp);
    va_end( argp);

    return res;
#else
    char write_text[HTTPD_OPT_MAX_PRINTF_LEN];
    va_list argp;
    int res;
    int i;

    va_start( argp, fmt);

    // NOTE: should use vsnprintf for length checking!
    res = HTTPD_OPT_FUNC_VSNPRINTF(write_text, sizeof(write_text), fmt, argp);

    va_end( argp);

    for (i=0;i<res;i++)
        http_output_putc(session, write_text[i]);

    return res;
#endif
}
//-------------------------------------------------------
// http_output_write:
//-------------------------------------------------------
int http_output_write(struct http_session *session, const char *str, int len)
{
    int i;

    if (len == -1)
        len = (int)httpd_strlen(str);

    for (i=0;i<len;i++)
        http_output_putc(session, str[i]);

    return len;
}
