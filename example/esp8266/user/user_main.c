#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_interface.h"
#include "user_config.h"
#include "espconn.h"
#include "mem.h"
#include "httpd.h"

//-------------------------------------------------------
// Defines
//-------------------------------------------------------
#define HTTP_SERVER_PORT  80

//-------------------------------------------------------
// Locals
//-------------------------------------------------------
LOCAL struct http_session _httpd;
LOCAL struct espconn      _tcp_socket;

//-------------------------------------------------------
// request_handler: Response to HTTP requests
//-------------------------------------------------------
static int request_handler(struct http_session *p, char * path, char * args)
{
    os_printf("REQUEST: PATH='%s' ARGS='%s'\n", path, args);

    if (os_strcmp(path, "/index.htm") == 0)
    {
        http_send_response(p,(char*)"Index", (char*)"text/plain", "Hello!", -1);
        return 1;
    }

    // Else - path not found
    return 0;
}
//-----------------------------------------------------------------------------
// http_send: TCP send function
//-----------------------------------------------------------------------------
static int http_send(int s, const char * buf, int len, int flags) 
{
    struct espconn *pesp_conn = (struct espconn *)s;
    os_printf( "%s: S\n", __FUNCTION__);
    espconn_sent(pesp_conn, (char*)buf, len);
    os_printf( "%s: E = %d\n", __FUNCTION__);
    return len;
}
//-----------------------------------------------------------------------------
// webserver_recv:
//-----------------------------------------------------------------------------
LOCAL void ICACHE_FLASH_ATTR
webserver_recv(void *arg, char *pusrdata, unsigned short length)
{
    struct espconn *pespconn = arg;

    os_printf( "%s: S\n", __FUNCTION__);

    // Process received data (if return = 1, more data
    // is expected else connection can be closed).
    if (!http_process_data(&_httpd, pusrdata, length))
    {
        espconn_disconnect(pespconn);
        os_printf( "%s: E (complete)\n", __FUNCTION__);
    }
    else
    {
        os_printf( "%s: E\n", __FUNCTION__);
    }
}
//-----------------------------------------------------------------------------
// webserver_recon:
//-----------------------------------------------------------------------------
LOCAL void ICACHE_FLASH_ATTR
webserver_recon(void *arg, sint8 err)
{
    struct espconn *pespconn = arg;

    os_printf( "%s: S\n", __FUNCTION__);

    os_printf( "%s: E\n", __FUNCTION__);
}
//-----------------------------------------------------------------------------
// webserver_discon:
//-----------------------------------------------------------------------------
LOCAL void ICACHE_FLASH_ATTR
webserver_discon(void *arg)
{
    struct espconn *pespconn = arg;

    os_printf( "%s: S\n", __FUNCTION__);

    os_printf( "%s: E\n", __FUNCTION__);
}
//-----------------------------------------------------------------------------
// webserver_listen
//-----------------------------------------------------------------------------
LOCAL void ICACHE_FLASH_ATTR
webserver_listen(void *arg)
{
    struct espconn *pesp_conn = (struct espconn *)arg;

    os_printf( "%s: S\n", __FUNCTION__);

    espconn_regist_recvcb(pesp_conn, webserver_recv);
    espconn_regist_reconcb(pesp_conn, webserver_recon);
    espconn_regist_disconcb(pesp_conn, webserver_discon);

    // Initialise HTTP request session
    http_new_connection(&_httpd, (int)arg);    

    os_printf( "%s: E\n", __FUNCTION__);
}
//-----------------------------------------------------------------------------
// open_tcp_listen: Open TCP socket for device requests
//-----------------------------------------------------------------------------
static int open_tcp_listen(void)
{
    struct ip_info ipconfig;

    os_printf( "%s: S\n", __FUNCTION__);

    _tcp_socket.type = ESPCONN_TCP;
    _tcp_socket.state = ESPCONN_NONE;
    _tcp_socket.proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
    _tcp_socket.proto.tcp->local_port = HTTP_SERVER_PORT;

    espconn_regist_connectcb(&_tcp_socket, webserver_listen);
    espconn_accept(&_tcp_socket);

    // Get local IP    
    wifi_get_ip_info(STATION_IF, &ipconfig);

    os_printf( "%s: E\n", __FUNCTION__);

    return 1;
}
//-----------------------------------------------------------------------------
// wifi_callback: On WIFI status change
//-----------------------------------------------------------------------------
void wifi_callback( System_Event_t *evt )
{
    os_printf( "%s: %d\n", __FUNCTION__, evt->event );
    
    switch ( evt->event )
    {
        case EVENT_STAMODE_CONNECTED:
        {
            os_printf("connect to ssid %s, channel %d\n",
                        evt->event_info.connected.ssid,
                        evt->event_info.connected.channel);
            break;
        }

        case EVENT_STAMODE_DISCONNECTED:
        {
            os_printf("disconnect from ssid %s, reason %d\n",
                        evt->event_info.disconnected.ssid,
                        evt->event_info.disconnected.reason);
            
            deep_sleep_set_option( 0 );
            system_deep_sleep( 60 * 1000 * 1000 );  // 60 seconds
            break;
        }

        case EVENT_STAMODE_GOT_IP:
        {
            os_printf("ip:" IPSTR ",mask:" IPSTR ",gw:" IPSTR,
                        IP2STR(&evt->event_info.got_ip.ip),
                        IP2STR(&evt->event_info.got_ip.mask),
                        IP2STR(&evt->event_info.got_ip.gw));
            os_printf("\n");

            // Register TCP server
            open_tcp_listen();

            // Initialise HTTPD library
            http_init(
                NULL,           // File open 'fopen' function
                NULL,           // File open 'fread' function
                NULL,           // File open 'fclose' function
                http_send       // Network socket send function
                );

            http_attach_cgi_handler(request_handler);            
            break;
        }
        
        default:
        {
            break;
        }
    }
}
//-----------------------------------------------------------------------------
// user_init: Init function 
//-----------------------------------------------------------------------------
void ICACHE_FLASH_ATTR user_init(void)
{
    char ssid[32] = SSID;
    char password[64] = SSID_PASSWORD;
    struct station_config stationConf;

    // Configure UART
    uart_div_modify( 0, UART_CLK_FREQ / ( 115200 ) );

    // Set station mode
    wifi_set_opmode( 0x1 );

    // Set ap settings
    os_memcpy(&stationConf.ssid, ssid, 32);
    os_memcpy(&stationConf.password, password, 64);
    wifi_station_set_config(&stationConf);

    wifi_set_event_handler_cb( wifi_callback );
}
//-----------------------------------------------------------------------------
// user_rf_pre_init:
//-----------------------------------------------------------------------------
void user_rf_pre_init(void)
{
    
}
