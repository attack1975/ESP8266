#ifndef __ESP_WEBSOCKET_H
#define __ESP_WEBSOCKET_H
#include "connection_list.h"
#include "html_server.h"
#include "lwip/err.h"

enum ws_frame_type{
    WS_CONTINUATION=0x00,
    WS_TEXT=0x01,
    WS_BINARY=0x02,
    WS_PING=0x09,
    WS_PONG=0x0A,
    WS_CLOSE=0x08,
    WS_INVALID=0xFF
};

void websocket_init(websocket_gotdata call);

int ICACHE_FLASH_ATTR websocket_writedata_t(char * data, enum ws_frame_type type);
int ICACHE_FLASH_ATTR websocket_writedata(char * data);

int ICACHE_FLASH_ATTR flush_sockets();

void ICACHE_FLASH_ATTR websocket_recv(char * string,char*url, connections* con, struct pbuf *p);
void ICACHE_FLASH_ATTR websocket_parse(char * data, size_t dataLen, struct tcp_pcb *pcb);



static const char *header_key = "Sec-WebSocket-Key: ";
static const char  *ws_uuid ="258EAFA5-E914-47DA-95CA-C5AB0DC85B11";


#endif
