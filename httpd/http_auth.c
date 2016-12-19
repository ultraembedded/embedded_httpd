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
#include "http_auth.h"
#include "httpd.h"

#if HTTP_OPT_SUPPORT_AUTH

//-------------------------------------------------------
// Prototypes:
//-------------------------------------------------------
static int base64_decode( const char *input, int input_length, char *target, int max_length );

//-------------------------------------------------------
// http_auth_validate:
//-------------------------------------------------------
int http_auth_validate(struct http_session *session)
{
    char auth[HTTP_OPT_MAX_USERNAME+HTTP_OPT_MAX_PASSWORD+1+1];

    int res = base64_decode(session->request.auth_string, (int)httpd_strlen(session->request.auth_string), auth, sizeof(auth)-1);
    if (res > 0)
    {
        char *user;
        char *pass;
        int user_len;
        int pass_len;
        int length = res;

        // Null terminate result
        auth[res] = 0;
        
        user = auth;
        pass = httpd_strchr(auth, ':');

        if (!pass)
            return 0;

        // Null terminate user string & skip delimiter
        *pass++ = 0;

        // Calculate lengths
        user_len = (int)httpd_strlen(user);
        pass_len = (int)httpd_strlen(pass);

        // Verify lengths match
        if (httpd_strlen(session->http_username) != user_len || httpd_strlen(session->http_password) != pass_len )
            return 0;

        // Check username
        if (httpd_strnicmp(user, session->http_username, httpd_strlen(session->http_username)))
            return 0;

        // Check password
        if (httpd_strnicmp(pass, session->http_password, httpd_strlen(session->http_password)))
            return 0;

        return 1;
    }
    else
        return 0;
}
//-------------------------------------------------------
// base64_is_valid_char: Valid base64 encoding character?
//-------------------------------------------------------
static int base64_is_valid_char(char c)
{
    if ((c >= 'A' && c <= 'Z') || 
        (c >= 'a' && c <= 'z') ||
        (c >= '0' && c <= '9') || 
        (c == '+')             ||
        (c == '/')             || 
        (c == '=')) 
        return 1;
    else
        return 0;
}
//-------------------------------------------------------
// base64_decode_char: Decode a base64 character
//-------------------------------------------------------
static unsigned char base64_decode_char(char c) 
{  
    if (c >= 'A' && c <= 'Z') 
        return c - 'A';
    else if (c >= 'a' && c <= 'z') 
        return c - 'a' + 26;
    else if (c >= '0' && c <= '9') 
        return c - '0' + 52;
    else if (c == '+')             
        return 62;
    else 
        return 63;
}
//-------------------------------------------------------
// base64_decode: Decode a base64 encoded string
//-------------------------------------------------------
static int base64_decode( const char *input, int input_length, char *target, int max_length )
{
    unsigned char *p = target;
    int k; 
    int out_length = 0;

    for(k=0; k<input_length; k+=4) 
    {
        char c1='A', c2='A', c3='A', c4='A';
        unsigned char b1=0, b2=0, b3=0, b4=0;

        // Fail if non base64 character found
        if(!base64_is_valid_char(input[k]))
            return 0;

        c1= input[k];

        if(k+1<input_length)
            c2= input[k+1];

        if(k+2<input_length)
            c3= input[k+2];

        if(k+3<input_length)
            c4= input[k+3];

        b1 = base64_decode_char(c1);
        b2 = base64_decode_char(c2);
        b3 = base64_decode_char(c3);
        b4 = base64_decode_char(c4);

        if (max_length)
        {
            *p++=((b1<<2)|(b2>>4) );
            out_length++;
            max_length--;
        }

        if(c3 != '=' && max_length)
        {
            *p++=(((b2&0xf)<<4)|(b3>>2) );
            out_length++;
            max_length--;
        }

        if(c4 != '=' && max_length)
        {
            *p++=(((b3&0x3)<<6)|b4 );    
            out_length++;
            max_length--;
        }
    }

    return out_length;
}

#endif
