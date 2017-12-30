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
#include "log.h"
#include "mem.h"

static websocket_gotdata data_callback;
extern connections * normal_connection_list[HTTP_POOL];
extern int tcp_enabled;

void websocket_init(websocket_gotdata call) {
	data_callback = call;
}

extern int datadone;
void ICACHE_FLASH_ATTR websocket_recv(char * string, char*url, connections* con,  struct pbuf *p) {


	if (strstr(string, "/echo") != 0) {
		char * key = 0;
		if (strstr(string, header_key) != 0) {
			char * begin = strstr(string, header_key) + os_strlen(header_key);
			char * end = strstr(begin, "\r");
			key = os_malloc((end - begin) + 1);
			os_memcpy(key, begin, end - begin);
			key[end - begin] = 0;
		}
		uint8_t digest[20]; //sha1 is always 20 byte long
		SHA1_CTX ctx;
		//os_printf("key:  -%s-\n", key);
		SHA1_Init(&ctx);
		SHA1_Update(&ctx, key, os_strlen(key));
		SHA1_Update(&ctx, ws_uuid, os_strlen(ws_uuid));
		SHA1_Final(digest, &ctx);
		char base64Digest[31]; //
		Base64encode(base64Digest, (const char*) digest, 20);
		int file = 0;
		char * d = os_malloc(os_strlen(HEADER_WEBSOCKET) + 36);
		os_sprintf(d, "%s%s\r\n\r\n", HEADER_WEBSOCKET, base64Digest);
		//os_printf("Handshake completed \r\n");
		tcp_write(con->connection, d, os_strlen(d), TCP_WRITE_FLAG_MORE);
		os_free(d);
		if(key != 0) {
			os_free(key);
		}
		key = 0;
		con->websocket = 1;
	} else if (con->websocket == 1) {
		int s = p->len;
		LOG_H(LOG_HTTPSERVER, LOG_HTTP_TAG, "WEBSOCKET MESSAGE \r\n");
		websocket_parse(string, s, con);
	}
}

int ICACHE_FLASH_ATTR websocket_write_size(void *arg, struct tcp_pcb *pcb, int size, enum ws_frame_type type) {
	uint8_t byte;
		int ret = -1;

//		if(datadone == 0)
//			return 0;
//
//		datadone = 0;

		if(current_settings[FRC_CONSOLE_ENABLE])
		os_printf("Sending out data: %p  %p \r\n", arg, pcb);

		char * buff = os_malloc(size+3);
		byte = 0x80; //set first bit
		byte |= type; //set op code
		buff[0] = byte;
		byte = 0;
		int SIZE = size;

		if (SIZE < 126) {
			byte = size;
			buff[1] = byte;
			os_memcpy(&buff[2], arg, byte);
			ret = tcp_write(pcb, buff, size+2, TCP_WRITE_FLAG_MORE | TCP_WRITE_FLAG_COPY);
		} else {
			buff[1] = ( 0x00 | 126 );
			buff[2] = ( size>>8 );
			buff[3] = ( size&0xff );
			os_memcpy(&buff[4], arg, size);
			ret = tcp_write(pcb, buff, size+4, TCP_WRITE_FLAG_MORE | TCP_WRITE_FLAG_COPY);
		}
		os_free(buff);

		return ret;
}

int ICACHE_FLASH_ATTR websocket_write(void *arg, struct tcp_pcb *pcb, enum ws_frame_type type) {
	int fsize = os_strlen(arg);
	return websocket_write_size(arg, pcb, fsize, type);
}

void ICACHE_FLASH_ATTR websocket_parse(char * data, int dataLen, connections *pcb) {
	uint8_t byte = data[0];
	int FIN = byte & 0x80;
	int TYPE = byte & 0x0F;
	//
	//hexDump("", data, dataLen);

	LOG_H(LOG_HTTPSERVER, LOG_HTTP_TAG, "Going to parse websocket \r\n");
	//os_printf("frame type %02X %02X %02X \r\n", TYPE, FIN, data[1]);
	//os_printf("%02X %02X %02X %02X \r\n", data[0], data[1], data[2], data[3]);
	if ((TYPE > 0x03 && TYPE < 0x08) || TYPE > 0x0B) {
		connections * con  = getConnection(normal_connection_list, pcb);
		data_callback(&data[1], dataLen-1, con);
		os_printf("Invalid frame type %02X \r\n", TYPE);
		return;
	}

	if(TYPE == WS_CONTINUATION) {
		os_printf("WS_CONTINUATION \r\n");
		return;
	}
	if(TYPE == WS_PING) {
		os_printf("Returning pong \r\n");
		websocket_write("  ", pcb->connection, WS_PONG);
		return;
	}

	byte = data[1];
	int MASKED = byte & 0x80;
	int SIZE = byte & 0x7F;

	if(current_settings[FRC_CONSOLE_ENABLE])
	os_printf("MASK:%d   SIZE:%d\r\n", MASKED, SIZE);

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

	//hexDump("", data, SIZE);
	//os_printf("MASK:%d   SIZE:%d\r\n", MASKED, SIZE);
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
		//DATA[SIZE] = 0;

		LOG_H(LOG_HTTPSERVER, LOG_HTTP_TAG, "SIZE: %d  dataLen: %d \r\n", SIZE, dataLen);


		if(SIZE == 0) {
			os_printf("SIZE == 0\r\n");
			//server_close(pcb);
			return;
		}

		if(SIZE > 1) {
			LOG_H(LOG_HTTPSERVER, LOG_HTTP_TAG, "data_callback started: %p - %d - %p \r\n", DATA, SIZE, pcb);
			data_callback(DATA, SIZE, pcb);
		}

		if (SIZE + offset < dataLen) {
			websocket_parse(&data[SIZE + offset], dataLen - (SIZE + offset), pcb);
		}
	} else {
		os_printf("NOT MASKED: %\r\n");
		hexDump("", data, SIZE);
	}
}


int ICACHE_FLASH_ATTR flush_sockets() {
	connections * con;
	connections ** connection_list = getConnectionsBegin(normal_connection_list);
	int ret = -1;
	for (int i = 0; i < HTTP_POOL; i++) {
		if (connection_list[i] != 0) {
			con = connection_list[i];
			if (con->websocket == 1 && con->connection != 0) {
				if(con->connection->state == ESTABLISHED) {
					if(current_settings[FRC_CONSOLE_ENABLE])
					os_printf("tcp flush - %d -%d  %d \r\n", i, con->connection->state, system_get_free_heap_size());

					//if(system_get_free_heap_size() > 2000) {
						ret = tcp_output(con->connection);
						if(ret != ERR_OK) {
							os_printf("ERROR TCP_OUPUT FLUSH: %d \r\n", ret);
						}
					//} else {
						//os_printf("FLUSH - NOT ENOUGHH HEAP \r\n");
					//}
					if(current_settings[FRC_CONSOLE_ENABLE])
					os_printf("tcp flush done %d - %d -%d\r\n", ret, i, con->connection->state);
				} else {
					os_printf("delete weird connection \r\n",  con->connection, con->connection->state);
					deleteConnection(normal_connection_list, con->connection);
				}
			}
		}
	}
	return ret;
}


extern struct udp_pcb * pUdpConnection;



int ICACHE_FLASH_ATTR websocket_abort_all()  {
	connections * con;
	connections ** connection_list = getConnectionsBegin(normal_connection_list);
	system_soft_wdt_feed();
	int ret = -1;
	for (int i = 0; i < HTTP_POOL; i++) {
		if (connection_list[i] != 0) {
			con = connection_list[i];
				tcp_abort(con->connection);
				deleteConnection(normal_connection_list, con->connection);
			}
		}
	//os_printf("Done writing all the data \r\n");
	return 0;
}


int ICACHE_FLASH_ATTR websocket_abort_all_except(connections * except)  {
	connections * con;
	connections ** connection_list = getConnectionsBegin(normal_connection_list);
	system_soft_wdt_feed();
	int ret = -1;
	for (int i = 0; i < HTTP_POOL; i++) {
		if (connection_list[i] != 0 && connection_list[i] != except) {
			con = connection_list[i];
				tcp_abort(con->connection);
				deleteConnection(normal_connection_list, con->connection);
			}
		}
	//os_printf("Done writing all the data \r\n");
	return 0;
}

int ICACHE_FLASH_ATTR websocket_close_all_except(connections * except)  {
	connections * con;
	connections ** connection_list = getConnectionsBegin(normal_connection_list);
	system_soft_wdt_feed();
	int ret = -1;
	for (int i = 0; i < HTTP_POOL; i++) {
		if (connection_list[i] != 0 && connection_list[i] != except) {
			con = connection_list[i];
				int tret = -1;
				tret = tcp_close(con->connection);
				deleteConnection(normal_connection_list, con->connection);
			}
		}
	//os_printf("Done writing all the data \r\n");
	return 0;
}

int ICACHE_FLASH_ATTR websocket_close_all()  {
	connections * con;
	connections ** connection_list = getConnectionsBegin(normal_connection_list);
	system_soft_wdt_feed();
	int ret = -1;
	for (int i = 0; i < HTTP_POOL; i++) {
		if (connection_list[i] != 0) {
			con = connection_list[i];
				int tret = -1;
				tret = tcp_close(con->connection);
				deleteConnection(normal_connection_list, con->connection);
			}
		}
	//os_printf("Done writing all the data \r\n");
	return 0;
}

int ICACHE_FLASH_ATTR websocket_writedata_size(char * data, int size, enum ws_frame_type type, int flush)  {
	connections * con;
	connections ** connection_list = getConnectionsBegin(normal_connection_list);
	system_soft_wdt_feed();
	int ret = -1;
	for (int i = 0; i < HTTP_POOL; i++) {
		if (connection_list[i] != 0) {
			con = connection_list[i];
			if (con->websocket == 1) {
				int tret = -1;
				tret = websocket_write_size(data, con->connection, size, type);
				if(tret == -1) {
					os_printf("return after websock write: %d \r\n", tret);
					//websocket_close_all();
				}

				if(flush) {
					tret = tcp_output(con->connection);
					if(tret != ERR_OK) {
						os_printf("ERROR TCP_OUPUT FLUSH: %d \r\n", tret);
					}
				}
				if (tret == 0) {
					con->timeout = 0;
					ret = 0;
				}
			}
		}
	}
	//os_printf("Done writing all the data \r\n");
	return ret;
}

int ICACHE_FLASH_ATTR websocket_writedata_flush(char * data)  {
	return websocket_writedata_size(data, os_strlen(data), WS_TEXT, 1);
}



int ICACHE_FLASH_ATTR websocket_writedata(char * data)  {
	return websocket_writedata_t(data, WS_TEXT);
}


int ICACHE_FLASH_ATTR websocket_writedata_t(char * data, enum ws_frame_type type)  {
	return websocket_writedata_size(data, os_strlen(data), type,0);
}
