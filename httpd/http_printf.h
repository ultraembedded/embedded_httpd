#ifndef __HTTP_PRINTF_H__
#define __HTTP_PRINTF_H__

#include "http_options.h"

//-----------------------------------------------------------------
// Types:
//-----------------------------------------------------------------
typedef int (*FP_OUTBUF_PUTC)(void *arg, char c);

//-----------------------------------------------------------------
// Structures
//-----------------------------------------------------------------
struct vbuf
{
    FP_OUTBUF_PUTC  function;
    void *          function_arg;
    char *          buffer;
    int             offset;
    int             max_length;
};

//-----------------------------------------------------------------
// Prototypes:
//-----------------------------------------------------------------
int     http_vsprintf(char *s, const char *format, va_list arg);
int     http_vsnprintf(char *s, int maxlen, const char *format, va_list arg);
int     http_sprintf(char *s, const char *format, ...);
int     http_snprintf(char *s, int maxlen, const char *format, ...);
int     http_xsnprintf(FP_OUTBUF_PUTC outfunc, void *outfunc_arg, const char *format, va_list arg);

#endif

