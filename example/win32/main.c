#include <stdio.h>
#include <winsock2.h>
#include "httpd\httpd.h"

#pragma comment(lib, "wsock32.lib")

// HTTP Server port
#define HTTPD_PORT		81

int http_send(int s, const char * buf, int len, int flags) { return send(s, buf, len, flags); }

//-----------------------------------------------------------------
// main:
//-----------------------------------------------------------------
void main(void)
{
	SOCKET listen_sock;
	struct sockaddr_in sin;
	char recv_buf[128];
	int res;

	struct http_session httpd;

	// Init Winsock
	WSADATA info;
	WSAStartup(MAKEWORD(2,0), &info);

	// Create server socket
	listen_sock = socket(AF_INET, SOCK_STREAM, 0);

	// Create server endpoint
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(HTTPD_PORT);

	// Bind server socket to endpoint
	if (bind(listen_sock, (struct sockaddr *) &sin, sizeof(sin)) < 0)
	{
		printf("Bind failed\n");
		return;
	}

	// Listen for connections
	if (listen(listen_sock, 5) < 0)
	{
		printf("Listen failed\n");
		return;
	}

	// Initialise HTTPD library (requires file IO & network send functions)
	http_init(
		fopen,			// File open 'fopen' function
		fread,			// File open 'fread' function
		fclose,			// File open 'fclose' function
		http_send		// Network socket send function
		);

	printf("HTTP server listening on port %d\n", HTTPD_PORT);
	while (1)
	{
		// Wait for new connection request
		SOCKET s = accept( listen_sock, NULL, NULL );
		if( s >= 0 )
		{
			// Initialise HTTP request session
			http_new_connection(&httpd, (int)s);

			// Enable HTTP username/password basic auth
			// (NOTE: HTTP_OPT_SUPPORT_AUTH must be 1 in options file)
			//http_set_auth(&httpd, "admin", "password");

			// Receive all data until end of request if reached (or the
			// end terminates the connection).
			while ( (res = recv(s, recv_buf, sizeof(recv_buf), 0)) > 0)
			{
				// Process received data (if return = 1, more data
				// is expected else connection can be closed).
				if (http_process_data(&httpd, recv_buf, res))
					continue;
				else
					break;
			}

			// Request is complete, close client connection
			closesocket(s);
		}
	}
}
