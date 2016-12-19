#ifndef __HTTP_OPTIONS_H__
#define __HTTP_OPTIONS_H__

//-------------------------------------------------------
// General Options:
//-------------------------------------------------------

// HTTP Output Buffer Size
#define HTTP_OPT_OUTPUT_BUFFER_SIZE  1000

// Max HTTP command line length
#define HTTP_OPT_INPUT_LINE_MAX      250

// HTTP file read chunk size
// NOTE: Stack size needs to be large enough!
#define HTTP_OPT_FILE_CHUNKSIZE      128

// Max URL request path length
#define HTTP_OPT_MAX_PATH_LENGTH     128

// Max GET request args length
#define HTTP_OPT_MAX_ARGS_LENGTH     128

// Default file if none specified
#define HTTP_OPT_DEFAULT_FILE        "/index.htm"

// [Optional] Path that preceeds all file requests
#define HTTP_OPT_USE_PATH            0
    #define HTTP_OPT_FILE_PATH       "/var/www"

// POST Support
#define HTTP_OPT_SUPPORT_POST        1
#define HTTP_OPT_MAX_POST_DATA       512

// Authorization Support
#define HTTP_OPT_SUPPORT_AUTH        0
#define HTTP_OPT_MAX_AUTH_STR        250
#define HTTP_OPT_MAX_USERNAME        50
#define HTTP_OPT_MAX_PASSWORD        50

// Stop clients caching pages (useful for active content)
#define HTTP_OPT_NO_CACHE_PRAGMA    1

// Use custom printf (1) or standard vsnprintf functions (0)
#define HTTPD_OPT_USE_CUSTOM_PRINTF   1

// Use standard printf - user provided vsnprintf
#if HTTPD_OPT_USE_CUSTOM_PRINTF == 0
    #define HTTPD_OPT_FUNC_VSNPRINTF   vsnprintf

    // Max http_output_printf output length (warning created on stack)
    #define HTTPD_OPT_MAX_PRINTF_LEN   256
#endif

//-------------------------------------------------------
// String Functions (Linux)
//-------------------------------------------------------
#if defined (__linux__)
    #include <string.h>
    #include <stdarg.h>
    #include <strings.h>

    #define httpd_strlen(s)        strlen(s)
    #define httpd_strchr(s,c)      strchr(s,c)
    #define httpd_strrchr(s,c)     strrchr(s,c)
    #define httpd_strnicmp(a,b,c)  strncasecmp(a,b,c)
    #define httpd_stricmp(a,b)     strcasecmp(a,b)
    #define httpd_strcpy(a,b)      strcpy(a,b)
    #define httpd_strncpy(a,b,c)   strncpy(a,b,c)
    #define httpd_strcat(a,b)      strcat(a,b)

//-------------------------------------------------------
// String Functions (Windows)
//-------------------------------------------------------
#elif defined (_WIN32)

    #include <string.h>
    #include <stdarg.h>

    #define httpd_strlen(s)        strlen(s)
    #define httpd_strchr(s,c)      strchr(s,c)
    #define httpd_strrchr(s,c)     strrchr(s,c)
    #define httpd_strnicmp(a,b,c)  strnicmp(a,b,c)
    #define httpd_stricmp(a,b)     stricmp(a,b)
    #define httpd_strcpy(a,b)      strcpy(a,b)
    #define httpd_strncpy(a,b,c)   strncpy(a,b,c)
    #define httpd_strcat(a,b)      strcat(a,b)

//-------------------------------------------------------
// String Functions (Custom / Embedded)
//-------------------------------------------------------
#else
    #include <string.h>
    #include <stdarg.h>

    #define httpd_strlen(s)        strlen(s)
    #define httpd_strchr(s,c)      strchr(s,c)
    #define httpd_strrchr(s,c)     strrchr(s,c)
    #define httpd_strnicmp(a,b,c)  strncasecmp(a,b,c)
    #define httpd_stricmp(a,b)     strcasecmp(a,b)
    #define httpd_strcpy(a,b)      strcpy(a,b)
    #define httpd_strncpy(a,b,c)   strncpy(a,b,c)
    #define httpd_strcat(a,b)      strcat(a,b)

#endif

//-------------------------------------------------------
// Misc
//-------------------------------------------------------
#ifndef NULL
    #define NULL 0
#endif

#endif