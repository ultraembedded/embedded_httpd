#ifndef __HTTP_OUTPUT_H__
#define __HTTP_OUTPUT_H__

//-------------------------------------------------------
// Prototypes:
//-------------------------------------------------------
void    http_output_init(struct http_session *session);
void    http_output_flush(struct http_session *session);
int     http_output_printf(struct http_session *session, const char *fmt, ...);
int     http_output_write(struct http_session *session, const char *str, int len);

#endif