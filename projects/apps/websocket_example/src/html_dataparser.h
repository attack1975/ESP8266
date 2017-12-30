#ifndef __HTML_DATAPARSE_H
#define __HTML_DATAPARSE_H
#include "lwip/tcp.h"
#include "connection_list.h"
#include "c_types.h"
#define HTTP_BUFFER_SIZE (1000)

static const char * HEADER_WEBSOCKETLINE = "Upgrade: websocket";

static char * HEADER_FLAT = "HTTP/1.1 200 OK \r\n\
Server: ESP \r\n\
Connection: close \r\n\
Cache-Control: no-cache \r\n\
Content-Type: text/html \r\n\
Content-Encoding: none \r\n\r\n";
static char * HEADER_NOTFOUND = "HTTP/1.1 404 Not Found \r\n\
Server: ESP \r\n\
Connection: close \r\n\
Cache-Control: no-cache \r\n\
Content-Type: text/html \r\n\r\n Not Found";


static char * HEADER_OK = "HTTP/1.1 200 OK \r\n\
Server: ESP \r\n\
Connection: close \r\n\
Cache-Control: max-age=31556926, public \r\n\
Content-Type: text/html \r\n\
Content-Encoding: gzip \r\n";

static char * HEADER_GIF_OK = "HTTP/1.1 200 OK \r\n\
Server: ESP \r\n\
Connection: close \r\n\
Cache-Control: max-age=31556926, public \r\n\
Content-Type: image/gif \r\n";


static char * HEADER_JS_OK = "HTTP/1.1 200 OK \r\n\
Server: ESP \r\n\
Connection: close \r\n\
Cache-Control: max-age=31556926, public \r\n\
Content-Type: text/javascript \r\n\
Content-Encoding: gzip \r\n";


static char * HEADER_CSS_OK = "HTTP/1.1 200 OK \r\n\
Server: ESP \r\n\
Connection: close \r\n\
Cache-Control: max-age=31556926, public \r\n\
Content-Type: text/css \r\n\
Content-Encoding: gzip \r\n";

static char * HEADER_WEBSOCKET = "\
HTTP/1.1 101 WebSocket Protocol Handshake\r\n\
Connection: Upgrade\r\n\
Upgrade: WebSocket\r\n\
Access-Control-Allow-Origin: http://192.168.179.14\r\n\
Access-Control-Allow-Credentials: true\r\n\
Access-Control-Allow-Headers: content-type \r\n\
Sec-WebSocket-Accept: ";


typedef struct {
		size_t size;
		uint8_t gzip;
		const char *name;
		uint32_t offset;
	} RO_FILE_ENTRY;

	typedef struct {
		size_t count;
		RO_FILE_ENTRY files[];
	} RO_FS;


extern const uint8_t rofs_data[];
extern const RO_FS ro_file_system;

int send_chunk(connections * con, char*header, char * data, int len);

#endif
