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
#include "http_input.h"

//-------------------------------------------------------
// http_input_init:
//-------------------------------------------------------
void http_input_init( struct http_input_buffer *pbuf, void (*on_text)(void *session, char* line), void (*on_request_end)(void *session), void (*on_data)(void *session, char* buf, int length))
{
    // State of buffer
    pbuf->buffer_state = HTTPREQ_PROC_HEADER;

    // Text line buffering
    pbuf->count = 0;
    pbuf->buffer[0] = 0;

    // On text line call-back
    pbuf->on_text = on_text;

    // On end of HTTP request call-back (prior to possible data)
    pbuf->on_request_end = on_request_end;

    // On data call-back
    pbuf->on_data = on_data;
}
//-------------------------------------------------------
// http_input_process:
//-------------------------------------------------------
int http_input_process( void *session, struct http_input_buffer *pbuf, char *s, int length)
{
    int i;
    int count = length;

    for (i=0;i<length; i++, s++, count--)
    {
        // Process text to gather command lines
        if (pbuf->buffer_state == HTTPREQ_PROC_HEADER)
        {
            // Add in new character
            pbuf->buffer[pbuf->count] = *s;

            // End of line or run out of space
            if ((*s == '\n') || (pbuf->count == HTTP_OPT_INPUT_LINE_MAX-1))
            {
                // Null terminate at tail
                pbuf->buffer[pbuf->count] = '\0';

                // Call user callback for line received
                if (pbuf->count)
                    pbuf->on_text(session, pbuf->buffer);
                // In HTTP, 0 length line denotes start of data
                else
                {
                    // Call end of HTTP text call-back
                    pbuf->on_request_end(session);

                    // Switch to data reception mode
                    pbuf->buffer_state = HTTPREQ_PROC_DATA;
                }

                pbuf->count = 0;
            }
            // Ignore carriage returns
            else if (*s != '\r')
                pbuf->count++;
        }
        // Passing data to HTTP server app
        else
        {
            // Pass remainder to app
            pbuf->on_data(session, s, count);
            break;
        }
    }

    return 1;
}
