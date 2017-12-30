#include "espmissingincludes.h"
#include "lwipopts.h"
#include "lwip/sockets.h"
#include "lwip/ip_addr.h"
#include "lwip/init.h"
#include "lwip/netif.h"
#include "lwip/igmp.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "ssl/ssl_crypto.h"
#include "base64.h"
#include "lwip/tcp.h"
#include "lwip_websocket.h"
#include "html_dataparser.h"
#include "connection_list.h"

static websocket_gotdata data_callback;

void websocket_init(websocket_gotdata call) {
	data_callback = call;
}

void ICACHE_FLASH_ATTR websocket_recv(char * string, char*url, connections* con,  struct pbuf *p) {
	if (strstr(string, "/echo") != 0) {
		char * key;
		if (strstr(string, header_key) != 0) {
			char * begin = strstr(string, header_key) + os_strlen(header_key);
			char * end = strstr(begin, "\r");
			key = os_malloc((end - begin) + 1);
			os_memcpy(key, begin, end - begin);
			key[end - begin] = 0;
		}
		uint8_t digest[20]; //sha1 is always 20 byte long
		SHA1_CTX ctx;
		os_printf("key:  -%s-\n", key);
		SHA1_Init(&ctx);
		SHA1_Update(&ctx, key, os_strlen(key));
		SHA1_Update(&ctx, ws_uuid, os_strlen(ws_uuid));
		SHA1_Final(digest, &ctx);
		char base64Digest[31]; //
		Base64encode(base64Digest, (const char*) digest, 20);
		int file = 0;
		char * d = os_malloc(os_strlen(HEADER_WEBSOCKET) + 36);
		os_sprintf(d, "%s%s\r\n\r\n", HEADER_WEBSOCKET, base64Digest);
		os_printf("Handshake completed \r\n");
		tcp_write(con->connection, d, os_strlen(d), TCP_WRITE_FLAG_MORE);
		os_free(d);
		if(key != 0) {
			os_free(key);
		}
		key = 0;
		con->websocket = 1;
	} else if (con->websocket == 1) {
		os_printf("WEBSOCKET MESSAGE \r\n");
		websocket_parse(string, os_strlen(string), con->connection);
	}
}

int ICACHE_FLASH_ATTR websocket_write(void *arg, struct tcp_pcb *pcb, enum ws_frame_type type) {
	uint8_t byte;
	int ret = -1;
	int fsize = os_strlen(arg) + 2;
	char * buff = os_malloc(fsize);
	byte = 0x80; //set first bit
	byte |= type; //set op code
	buff[0] = byte;
	byte = 0;
	int SIZE = os_strlen(arg);
	if (SIZE < 126) {
		byte = os_strlen(arg);
		buff[1] = byte;
		os_memcpy(&buff[2], arg, byte);
		ret = tcp_write(pcb, buff, fsize, TCP_WRITE_FLAG_MORE);
	} else {
		os_printf("Too much data \r\n");
	}
	os_free(buff);
	return ret;
}

void ICACHE_FLASH_ATTR websocket_parse(char * data, size_t dataLen, struct tcp_pcb *pcb) {
	uint8_t byte = data[0];
	int FIN = byte & 0x80;
	int TYPE = byte & 0x0F;
	os_printf("frame type %02X %02X \r\n", TYPE, FIN);
	os_printf("%02X %02X %02X %02X \r\n", data[0], data[1], data[2], data[3]);
	if ((TYPE > 0x03 && TYPE < 0x08) || TYPE > 0x0B) {
		os_printf("Invalid frame type %02X \r\n", TYPE);
		return;
	}
	if(TYPE == WS_CONTINUATION) {
		os_printf("WS_CONTINUATION \r\n");
		return;
	}
	if(TYPE == WS_PING) {
		os_printf("Returning pong \r\n");
		websocket_write("  ", pcb, WS_PONG);
		return;
	}

	byte = data[1];
	int MASKED = byte & 0x80;
	int SIZE = byte & 0x7F;

	int offset = 2;
	if (SIZE == 126) {
		SIZE = 0;
		SIZE = data[3];                 //LSB
		SIZE |= (uint64_t) data[2] << 8; //MSB
		offset = 4;
	} else if (SIZE == 127) {
		SIZE = 0;
		SIZE |= (uint64_t) data[2] << 56;
		SIZE |= (uint64_t) data[3] << 48;
		SIZE |= (uint64_t) data[4] << 40;
		SIZE |= (uint64_t) data[5] << 32;
		SIZE |= (uint64_t) data[6] << 24;
		SIZE |= (uint64_t) data[7] << 16;
		SIZE |= (uint64_t) data[8] << 8;
		SIZE |= (uint64_t) data[9];
		offset = 10;
	}

	if (MASKED) {
		//read mask key
		char mask[4];
		mask[0] = data[offset];
		mask[1] = data[offset + 1];
		mask[2] = data[offset + 2];
		mask[3] = data[offset + 3];
		offset += 4;
		uint64_t i;
		for (i = 0; i < SIZE; i++) {
			data[i + offset] ^= mask[i % 4];
		}
		char * DATA = &data[offset];
		DATA[SIZE] = 0;
		os_printf("SIZE: %d  tSIZE: %d, DATA: =%s=  \r\n", SIZE, dataLen, DATA);

		if(SIZE == 0) {
			//server_close(pcb);
			return;
		}
		//hexDump(0, DATA, SIZE);

		data_callback(DATA, SIZE);


		if (SIZE + offset < dataLen) {
			websocket_parse(&data[SIZE + offset], dataLen - (SIZE + offset), pcb);
		}
	}
}


int ICACHE_FLASH_ATTR flush_sockets() {
	connections * con;
	connections ** connection_list = getConnectionsBegin();
	int ret = -1;
	for (int i = 0; i < HTTP_POOL; i++) {
		if (connection_list[i] != 0) {
			con = connection_list[i];
			if (con->websocket == 1) {
				tcp_output(con->connection);
			}
		}
	}
	return ret;
}

int ICACHE_FLASH_ATTR websocket_writedata(char * data)  {
	return websocket_writedata_t(data, WS_TEXT);
}

int ICACHE_FLASH_ATTR websocket_writedata_t(char * data, enum ws_frame_type type)  {
	connections * con;
	connections ** connection_list = getConnectionsBegin();
	int ret = -1;
	for (int i = 0; i < HTTP_POOL; i++) {
		if (connection_list[i] != 0) {
			con = connection_list[i];
			if (con->websocket == 1) {
				int tret = -1;
				tret = websocket_write(data, con->connection, type);
				if (tret == 0) {
					con->timeout = 0;
					ret = 0;
				}
			}
		}
	}
	return ret;
}
