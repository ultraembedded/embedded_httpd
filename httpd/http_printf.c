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
#include "http_printf.h"

#if HTTPD_OPT_USE_CUSTOM_PRINTF

//----------------------------------------------------
// Config
//----------------------------------------------------
#define PRINTF_DEC_PRINT
#define PRINTF_HEX_PRINT
#define PRINTF_STR_PRINT
#define PRINTF_CHR_PRINT
#define PRINTF_ENABLE_PADDING

//----------------------------------------------------
// Structures
//----------------------------------------------------
typedef struct 
{
    int  len;
    long num1;
    long num2;
    char pad_character;
    int  do_padding;
    int  left_flag;
    int  uppercase;
    int  max_len;
} tprintf_params;

//----------------------------------------------------
// vbuf_putchar: vbuf_printf output function (directs
// output to either function or buffer.
//----------------------------------------------------
static void vbuf_putchar(struct vbuf *buf, char c)
{
    // Function is target
    if (buf->function)
        buf->function(buf->function_arg, c);
    // Buffer is target
    else if (buf->buffer)
    {
        if (buf->offset < buf->max_length)
            buf->buffer[buf->offset++] = c;
    }
}
//----------------------------------------------------
// printf_padding: Add padding to output buffer
//----------------------------------------------------
#if defined PRINTF_DEC_PRINT || defined PRINTF_HEX_PRINT || defined PRINTF_STR_PRINT
#if defined PRINTF_ENABLE_PADDING
static void printf_padding( struct vbuf *buf, const int l_flag, tprintf_params *par)
{
    int i;

    if (par->do_padding && l_flag && (par->len < par->num1))
        for (i=par->len; i<par->num1; i++)
            vbuf_putchar(buf, par->pad_character);
}
#endif
#endif
//----------------------------------------------------
// printf_outs: Add string to output buffer
//----------------------------------------------------
#ifdef PRINTF_STR_PRINT
static void printf_outs( struct vbuf *buf, char* lp, tprintf_params *par)
{
    // Pad left
    par->len = httpd_strlen( lp);
#ifdef PRINTF_ENABLE_PADDING
    printf_padding(buf, !(par->left_flag), par);
#endif

    while (*lp && (par->num2)--)
        vbuf_putchar(buf, *lp++);

    // Pad right
    par->len = httpd_strlen( lp);
#ifdef PRINTF_ENABLE_PADDING
    printf_padding(buf, par->left_flag, par);
#endif
}
#endif
//----------------------------------------------------
// printf_outnum: Add number to output buffer
//----------------------------------------------------
#if defined PRINTF_DEC_PRINT || defined PRINTF_HEX_PRINT
static void printf_outnum( struct vbuf *buf, const long n, const long base, tprintf_params *par)
{
    char*         cp;
    int           negative;
    char          outbuf[32];
    const char    udigits[] = "0123456789ABCDEF";
    const char    ldigits[] = "0123456789abcdef";
    unsigned long num;
    int           count;
    const char *  digits = par->uppercase ? udigits : ldigits;

    // Negative number
    if (base == 10 && n < 0L)
    {
        negative = 1;
        num = -(n);
    }
    else
    {
        num = (n);
        negative = 0;
    }

    // Create number (in reverse)
    cp = outbuf;

    // Dec
    if (base == 10)
    {    
        do
        {
            *cp++ = digits[(int)(num % 10)];
        }
        while ((num /= 10) > 0);
    }
    // Hex
    else
    {
        do 
        {
            *cp++ = digits[(int)(num % 16)];
        }
        while ((num /= 16) > 0);
    }

    if (negative)
        *cp++ = '-';
    *cp-- = 0;

    // Pad
    par->len = httpd_strlen(outbuf);
#ifdef PRINTF_ENABLE_PADDING
    printf_padding(buf, !(par->left_flag), par);
#endif
    count = 0;
    while (cp >= outbuf && count++ < par->max_len)
        vbuf_putchar(buf, *cp--);
#ifdef PRINTF_ENABLE_PADDING
    printf_padding(buf, par->left_flag, par);
#endif
}
#endif
//----------------------------------------------------
// printf_getnum: Decode number in format string
//----------------------------------------------------
static long printf_getnum( char** linep)
{
    long n;
    char* cp;

    n = 0;
    cp = *linep;
    while (((*cp) >= '0' && (*cp) <= '9'))
        n = n*10 + ((*cp++) - '0');
    *linep = cp;
    return n;
}
//----------------------------------------------------
// vbuf_printf: Buffered printf function
//----------------------------------------------------
int vbuf_printf(struct vbuf *buf, const char* ctrl1, va_list argp)
{
    int long_flag;
    int dot_flag;
    int res = 0;

    tprintf_params par;

    char ch;
    char* ctrl = (char*)ctrl1;

    for ( ; *ctrl; ctrl++) 
    {
        // Format request found?
        if (*ctrl != '%') 
        {
            vbuf_putchar(buf, *ctrl);
            continue;
        }

        // Create request
        dot_flag   = long_flag = par.left_flag = par.do_padding = 0;
        par.pad_character = ' ';
        par.num2          = 32767;
        par.max_len       = 10;

 try_next:
        ch = *(++ctrl);

        if ((ch >= '0' && ch <= '9')) 
        {
            if (dot_flag)
                par.num2 = printf_getnum(&ctrl);
            else {
                if (ch == '0')
                    par.pad_character = '0';

                par.num1 = printf_getnum(&ctrl);
                par.do_padding = 1;
            }
            ctrl--;
            goto try_next;
        }

        par.uppercase = (ch >= 'A' && ch <= 'Z') ? 1 : 0;

        switch ((par.uppercase ? ch + 32: ch)) 
        {
            case '%':
                vbuf_putchar(buf, '%');
                continue;

            case '-':
                par.left_flag = 1;
                break;

            case '.':
                dot_flag = 1;
                break;

            case 'l':
                long_flag = 1;
                break;

#ifdef PRINTF_DEC_PRINT
            case 'd':
                if (long_flag || ch == 'D') 
                {
                    printf_outnum(buf, va_arg(argp, long), 10L, &par);
                    continue;
                }
                else
                {
                    printf_outnum(buf, va_arg(argp, int), 10L, &par);
                    continue;
                }
#endif
#ifdef PRINTF_HEX_PRINT
            case 'x':
            case 'p':
                if (long_flag || ch == 'D') 
                {
                    par.max_len = sizeof(long) * 2;
                    printf_outnum(buf, (long)va_arg(argp, long), 16L, &par);
                }
                else
                {
                    par.max_len = sizeof(int) * 2;
                    printf_outnum(buf, (long)va_arg(argp, int), 16L, &par);
                }
                continue;
#endif
#ifdef PRINTF_STR_PRINT
            case 's':
                printf_outs(buf, va_arg( argp, char*), &par);
                continue;
#endif
#ifdef PRINTF_CHR_PRINT
            case 'c':
                vbuf_putchar(buf, va_arg( argp, int));
                continue;
#endif
            case '\\':
                switch (*ctrl)
                {
                    case 'a':
                        vbuf_putchar(buf, '\a');
                        break;
                    case 'r':
                        vbuf_putchar(buf, '\r');
                        break;
                    case 'n':
                        vbuf_putchar(buf, '\n');
                        break;
                    default:
                        vbuf_putchar(buf, *ctrl);
                        break;
                }
                ctrl++;
                break;

            default:
                continue;
        }
        goto try_next;
    }

    return res;
}
//----------------------------------------------------
// http_vsprintf: 
//----------------------------------------------------
int http_vsprintf(char *s, const char *format, va_list arg)
{
    struct vbuf buf;

    if (!s || !format)
        return 0;

    // Setup buffer to be target
    buf.function = 0;
    buf.buffer = s;
    buf.offset = 0;
    buf.max_length = 32768; // default

    vbuf_printf(&buf, format, arg);

    // Null terminate at end of string
    buf.buffer[buf.offset] = 0;

    return buf.offset;
}
//----------------------------------------------------
// http_vsnprintf: 
//----------------------------------------------------
int http_vsnprintf( char *s, int maxlen, const char *format, va_list arg)
{
    struct vbuf buf;

    if (!s || !format || !maxlen)
        return 0;

    // Setup buffer to be target
    buf.function = 0;
    buf.buffer = s;
    buf.offset = 0;
    buf.max_length = maxlen;

    vbuf_printf(&buf, format, arg);

    // Null terminate at end of string
    buf.buffer[buf.offset] = 0;

    return buf.offset;
}
//----------------------------------------------------
// http_sprintf: 
//----------------------------------------------------
int http_sprintf(char *s, const char *format, ...)
{
    va_list argp;
    struct vbuf buf;

    if (!s || !format)
        return 0;

    va_start( argp, format);

    // Setup buffer to be target
    buf.function = 0;
    buf.buffer = s;
    buf.offset = 0;
    buf.max_length = 32768; // default

    vbuf_printf(&buf, format, argp);

    // Null terminate at end of string
    buf.buffer[buf.offset] = 0;

    va_end( argp);

    return buf.offset;
}
//----------------------------------------------------
// http_snprintf: 
//----------------------------------------------------
int http_snprintf(char *s, int maxlen, const char *format, ...)
{
    va_list argp;
    struct vbuf buf;

    if (!maxlen || !s || !format)
        return 0;

    va_start( argp, format);

    // Setup buffer to be target
    buf.function = 0;
    buf.buffer = s;
    buf.offset = 0;
    buf.max_length = maxlen;

    vbuf_printf(&buf, format, argp);

    // Null terminate
    if (buf.offset < buf.max_length)
        buf.buffer[buf.offset] = 0;
    else
        buf.buffer[buf.max_length-1] = 0;

    va_end( argp);

    return buf.offset;
}
//----------------------------------------------------
// http_xsnprintf: Custom printf to output func
//----------------------------------------------------
int http_xsnprintf(FP_OUTBUF_PUTC outfunc, void *outfunc_arg, const char *format, va_list arg)
{
    struct vbuf buf;

    if (!format)
        return 0;

    // Output to user provided function
    buf.function     = outfunc;
    buf.function_arg = outfunc_arg;
    buf.buffer       = NULL;
    buf.offset       = 0;
    buf.max_length   = 32768; // default

    vbuf_printf(&buf, format, arg);

    return buf.offset;
}
#endif
