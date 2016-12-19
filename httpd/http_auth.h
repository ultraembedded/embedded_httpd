#ifndef __HTTP_AUTH_H__
#define __HTTP_AUTH_H__

#include "httpd.h"

//-------------------------------------------------------
// Prototypes:
//-------------------------------------------------------
int http_auth_validate(struct http_session *session);

#endif