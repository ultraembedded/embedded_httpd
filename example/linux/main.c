#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>
#include "httpd.h"

#define HTTP_SERVER_PORT 8080

//-------------------------------------------------------
// http_send: TCP send function
//-------------------------------------------------------
static int http_send(int s, const char * buf, int len, int flags) 
{ 
    return send(s, buf, len, flags); 
}
//-------------------------------------------------------
// request_handler: Response to HTTP requests
//-------------------------------------------------------
static int request_handler(struct http_session *p, char * path, char * args)
{
    char buffer[1024];

    printf("REQUEST: PATH='%s' ARGS='%s'\n", path, args);

    // Example of directory listing
    if (strcmp(path, "/index.htm") == 0)
    {
        int len;
        struct dirent *pDirent;
        DIR *pDir;

#if HTTP_OPT_USE_PATH
        chdir(HTTP_OPT_FILE_PATH);
#endif

        // File exists, skip dir listing generation
        if (access("index.htm", F_OK ) != -1)
            return 0;

#if HTTP_OPT_USE_PATH
        pDir = opendir (HTTP_OPT_FILE_PATH);
#else        
        pDir = opendir (".");
#endif
        if (pDir == NULL) 
        {
            printf ("Cannot open directory\n");
            exit(-1);
        }

        http_send_start(p, "Directory");

        while ((pDirent = readdir(pDir)) != NULL)
        {
            if (strcmp(pDirent->d_name, ".") == 0 || strcmp(pDirent->d_name, "..") == 0)
                continue;

            http_output_printf(p, "<a href='%s'>%s</a><br/>", pDirent->d_name, pDirent->d_name);
        }
        closedir (pDir);

        http_send_end(p);
        return 1;
    }
    // Example of generated file
    else if (strcmp(path, "/hello.htm") == 0)
    {
        printf("Responding to index.htm\n");

        strcpy(buffer, "Hello\r\n");
        strcat(buffer, "\r\n");
            
        http_send_response(p,(char*)"Index", (char*)"text/html", buffer, strlen(buffer));

        return 1;
    }

    // Else - try and open path as file
    return 0;
}
//-------------------------------------------------------
// Entry point
//-------------------------------------------------------
int main(void)
{
    struct http_session httpd;
    struct sockaddr_in addr;
    int addrlen;
    int server_socket;
    int optval = 1;

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
        return -1;

    // Bind to port
    memset(&addr, 0x00, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(HTTP_SERVER_PORT);
    addrlen = sizeof(addr);

    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));

    printf("Listening on port %d\n", HTTP_SERVER_PORT);
    if (bind(server_socket, (struct sockaddr *) &addr, sizeof(addr)) < 0) 
    {
        printf("Listen failed\n");
        return -1;
    }

    // Listen for connections
    if (listen(server_socket, 5) < 0)
    {
        printf("Listen failed\n");
        return -1;
    }

    // Initialise HTTPD library
    http_init(
        fopen,          // File open 'fopen' function
        fread,          // File open 'fread' function
        fclose,         // File open 'fclose' function
        http_send       // Network socket send function
        );

    // Intecept requests to provide custom responses
    http_attach_cgi_handler(request_handler);

    memset(&addr, 0x00, sizeof(addr));
    addrlen = sizeof(addr);

    // Wait for packet
    while (1)
    {
        char buffer[512];

        // Wait for new connection request
        int s = accept( server_socket, NULL, NULL );
        if( s >= 0 )
        {
            int res;

            // Initialise HTTP request session
            http_new_connection(&httpd, (int)s);

            // Receive all data until end of request if reached (or the
            // end terminates the connection).
            while ( (res = recv(s, buffer, sizeof(buffer)-1, 0)) > 0)
            {
                buffer[res] = '\0';

                // Process received data (if return = 1, more data
                // is expected else connection can be closed).
                if (http_process_data(&httpd, buffer, res))
                    continue;
                else
                    break;
            }

            // Request is complete, close client connection
            close(s);
        }
    }
}