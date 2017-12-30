#include "lwipopts.h"
#include "lwip/sockets.h"
#include "lwip/ip_addr.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "lwip/err.h"
#include "espmissingincludes.h"
#include "lwip_websocket.h"
#include "connection_list.h"
#include "html_server.h"
#include "html_dataparser.h"
#include "fs.h"
#include "log.h"

connections * normal_connection_list[HTTP_POOL] = { 0 };

static ETSTimer callbackTimer;

extern uint32_t current_settings[FRC_INVALID];

static websocket_gotdata data_callback;

#define HTTP_BUFFER_SIZE (1000)

void ICACHE_FLASH_ATTR server_init(websocket_gotdata call) {
	struct tcp_pcb *pcb;
	espconn_tcp_set_max_con(3); // 1 websocket, 2 extra
	pcb = tcp_new();
	tcp_bind(pcb, IP_ADDR_ANY, 80); //server port for incoming connection
	pcb = tcp_listen(pcb);
	tcp_accept(pcb, server_accept);
	data_callback = call;
	websocket_init(call);
	//os_timer_setfn(&pollTimer, server_poll, 0);
	//os_timer_arm(&pollTimer, 1000, 0);
}



void server_kill(void * tKill) {
	if(tKill != 0) {
		LOG_I(LOG_HTTPSERVER, LOG_HTTP_TAG, "server_kill(): Closing... %p \r\n", tKill);
		tcp_abort((struct tcp_pcb *) tKill);
		tKill = 0;
	}
}
void ICACHE_FLASH_ATTR server_close(struct tcp_pcb *pcb) {
	int i = 0;
	tcp_arg(pcb, NULL);
	tcp_sent(pcb, NULL);
	tcp_recv(pcb, NULL);
	tcp_err(pcb, NULL);
	deleteConnection(normal_connection_list, pcb);
	LOG_I(LOG_HTTPSERVER, LOG_HTTP_TAG, "server_close(): Closing... %p \r\n", pcb);
	if(pcb != 0) {
		i = tcp_close(pcb);
		os_timer_disarm(&callbackTimer);
		os_timer_setfn(&callbackTimer, server_kill, pcb);
		os_timer_arm(&callbackTimer, 2, 0);
	}
	LOG_H(LOG_HTTPSERVER, LOG_HTTP_TAG, "server_close(): Closing...%d-%d \r\n", i, 0);
}

static err_t ICACHE_FLASH_ATTR server_accept(void *arg, struct tcp_pcb *pcb, err_t err) {
	LWIP_UNUSED_ARG(arg);
	LWIP_UNUSED_ARG(err);

	tcp_setprio(pcb, TCP_PRIO_MAX);
	tcp_arg(pcb, NULL);

	tcp_recv(pcb, server_recv);
	tcp_err(pcb, server_err);
	tcp_sent(pcb, server_sent);
	tcp_poll(pcb, server_poll, 4); //every two seconds of inactivity of the TCP connection
	tcp_accepted(pcb);


	LOG_I(LOG_HTTPSERVER, LOG_HTTP_TAG, "server_accept(): Accepting incoming connection on server... %p \r\n", pcb);
	return ERR_OK;
}


int datadone = 1;


static err_t ICACHE_FLASH_ATTR server_sent(void *arg, struct tcp_pcb *pcb, u16_t len) {
	connections * con = getConnection(normal_connection_list, pcb);
	if (con != 0 && con->dataleft > 0) {
		int flag = TCP_WRITE_FLAG_MORE;
		long filesize = con->dataleft;
		if (con->dataleft > HTTP_BUFFER_SIZE) {
			filesize = HTTP_BUFFER_SIZE;
		} else {
			flag = 0;
		}
		int i = 5;
		LOG_T(LOG_HTTPSERVER, LOG_HTTP_TAG, "next package \r\n");

		if(con->is_spi) {
			if(current_settings[FRC_CONSOLE_ENABLE])
				os_printf("html file dataleft: %d", con->dataleft);
			char buf[filesize];
			fs_readfile_max(BEGIN_ADDRESS_HTML, buf, con->spi_num, filesize, con->filepos);
			i = tcp_write(pcb, buf, filesize, flag);
		} else if(con->is_gif) {
			if(current_settings[FRC_CONSOLE_ENABLE])
			os_printf("gif file dataleft: %d", con->dataleft);
			char buf[filesize];
			fs_readfile_max(BEGIN_ADDRESS_GIF, buf, con->spi_num, filesize, con->filepos);
			i = tcp_write(pcb, buf, filesize, flag);
		} else {
			if(current_settings[FRC_CONSOLE_ENABLE])
				os_printf("raw data: %d", filesize);
			i = tcp_write(pcb, con->data + con->filepos, filesize, flag);
		}
		if(current_settings[FRC_CONSOLE_ENABLE])
		os_printf("\r\n");
		//os_printf("ERROR CODE: %d \r\n", i);
		if (i == ERR_OK) {
			con->filepos += filesize;
			con->dataleft -= filesize;
			LOG_I(LOG_HTTPSERVER, LOG_HTTP_TAG, "Last package was done, sending new package pcb: %p  dataleft: %d \r\n", con->connection, con->dataleft);
		} else {
			LOG_E(LOG_HTTPSERVER, LOG_HTTP_TAG, "ERRRRORRRR \r\n");
		}

		if (con->dataleft <= 0) {
			con->dataleft = 0;
			con->filepos = 0;
			if(con->websocket == 0) {
				LOG_I(LOG_HTTPSERVER, LOG_HTTP_TAG, "Done sending, therefore closing, sockets: %d,  websockets:%d, heap: %d \r\n", getConnectionSize(normal_connection_list), getConnectionSizeWebsocket(normal_connection_list), system_get_free_heap_size());
				server_close(pcb);
			} else {
				LOG_I(LOG_HTTPSERVER, LOG_HTTP_TAG, "Done sending websocket, therefore closing, sockets: %d,  websockets:%d, heap: %d \r\n", getConnectionSize(normal_connection_list), getConnectionSizeWebsocket(normal_connection_list), system_get_free_heap_size());
				server_close(pcb);
			}
		}
	} else {
		if (con != 0 && con->websocket == 0) {
			LOG_I(LOG_HTTPSERVER, LOG_HTTP_TAG, "Done sending, therefore closing  2\r\n");
			server_close(pcb);
		}
	}
	datadone = 1;

	return 0;
}

extern char animation_random_choises[100];

extern INDEXSTRUCT str;
static err_t ICACHE_FLASH_ATTR server_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err) {
	char *string;
	int length;
	LWIP_UNUSED_ARG(arg);

	LOG_I(LOG_HTTPSERVER, LOG_HTTP_TAG, "Received raw data pcb: %p \r\n", pcb);

	connections* con = getConnection(normal_connection_list, pcb);
	if (con == 0) {
		LOG_T(LOG_HTTPSERVER, LOG_HTTP_TAG, "create connection: %p \r\n", pcb);
		con = newConnection(normal_connection_list, pcb);
		if (con == 0) {
			LOG_E(LOG_HTTPSERVER, LOG_HTTP_TAG, "ERROR creating new conn \r\n");
			pbuf_free(p);
			return ERR_MEM;
		}
	} else {
		con->timeout = 0;
	}

	LOG_H(LOG_HTTPSERVER, LOG_HTTP_TAG, "payload: -%s-\r\n", pcb, p->payload);

	if (err == ERR_OK && p != NULL) {
		pcb->flags |= TF_NODELAY;  //TF_NAGLEMEMERR
		string = p->payload;
		tcp_recved(pcb, p->tot_len); // if frame is big

		char url[50] = { 0 };
		if (strstr(string, "GET /") != 0) {
			char *begin = strstr(string, "GET /") + 4;
			char *end = strstr(begin, " ");
			os_memcpy(url, begin, end - begin);
			url[end - begin] = 0;
		}

		if (strstr(string, HEADER_WEBSOCKETLINE) != 0) {
			con->websocket = 1;
			LOG_I(LOG_HTTPSERVER, LOG_HTTP_TAG, "New websocket message, socket: %d,  websockets:%d, heap: %d \r\n", getConnectionSize(normal_connection_list), getConnectionSizeWebsocket(normal_connection_list), system_get_free_heap_size());LOG_H(LOG_HTTPSERVER, LOG_HTTP_TAG, "WEBSOCKET MESSAGE \r\n");
			if(getConnectionSizeWebsocket(normal_connection_list) > 1) {
				websocket_close_all_except(con);
			}
		}

		if(con->websocket == 1 && p->len > 0) {
			LOG_H(LOG_HTTPSERVER, LOG_HTTP_TAG, "WEBSOCKET MESSAGE \r\n");
			websocket_recv(p->payload, url, con, p);
			pbuf_free(p);
		} else {
			LOG_T(LOG_HTTPSERVER, LOG_HTTP_TAG, "NORMAL MESSAGE: %s-%d \r\n", url, os_strlen(url));
			int file = 2;
			char * header = HEADER_OK;
			int err = 0;

			if(strstr(url, "/gifoptions") != 0) {
					int flag = TCP_WRITE_FLAG_MORE | TCP_WRITE_FLAG_COPY;
					char buf[2000];
					os_memset(buf, 0, 2000);
					os_sprintf(buf, "%s[",HEADER_FLAT);
					int bufsize = os_strlen(HEADER_FLAT);
					if(animation_random_choises[0] == '0') {
						os_sprintf(buf, "%s'%s-S',",buf, "aNone");
					} else {
						os_sprintf(buf, "%s'%s',",buf, "aNone");
					}
					for(int i = 0; i < str.filesactive; i++) {
						os_sprintf(buf, "%s'%s",buf, str.filename[i]);
						for(int y = 0; y < 50; y++) {
							if(animation_random_choises[y] -1 == i && animation_random_choises[y] != 255) {
								os_sprintf(buf, "%s-S",buf);
							}
						}
						os_sprintf(buf, "%s'",buf);
						if(i != str.filesactive-1) {
							os_sprintf(buf, "%s,",buf);
						}
					}
					os_sprintf(buf, "%s]",buf);
					bufsize += os_strlen(buf);
					LOG_T(LOG_HTTPSERVER, LOG_HTTP_TAG, "Sending: %d-%d  %p\r\n", os_strlen(buf), bufsize, buf);
					tcp_write(pcb, buf, os_strlen(buf), flag);
					tcp_output(pcb);
					pbuf_free(p);
					LOG_I(LOG_HTTPSERVER, LOG_HTTP_TAG, "Done sending gifoptions, therefore closing \r\n");
					server_close(pcb);
					return ERR_OK;
				}
				int fi = fs_findfile_name(BEGIN_ADDRESS_HTML, &url[1]);

				uint32 addr = BEGIN_ADDRESS_HTML;
				con->is_spi = 1;

				if(os_strlen(url) < 2)
					fi = fs_findfile_name(addr, "index.html");

				if(strstr(url, "/gif/") != 0) {
					addr = BEGIN_ADDRESS_GIF;
					fi = fs_findfile_name(addr, &url[5]);
					con->is_spi = 0;
					con->is_gif = 1;
					header = HEADER_GIF_OK;
				}

				if(fi == -1) {
					LOG_E(LOG_HTTPSERVER, LOG_HTTP_TAG, "Cannot find: %s \r\n", url);
					err = send_chunk(con, HEADER_NOTFOUND, "", 0);
				} else {
					LOG_T(LOG_HTTPSERVER, LOG_HTTP_TAG, "YEAHH FOUND FILE \r\n");

					INDEXSTRUCT str = fs_readindex(addr);

					if(strstr(url, ".css") != 0) {
						header = HEADER_CSS_OK;
					}

					if(strstr(url, ".js") != 0) {
						header = HEADER_JS_OK;
					}
					if(con == 0) {
						LOG_E(LOG_HTTPSERVER, LOG_HTTP_TAG, "http recv ERROR connection is 0 \r\n");
					}

					con->spi_num = fi;
					con->dataleft = str.len[fi];

					LOG_T(LOG_HTTPSERVER, LOG_HTTP_TAG, "YEAHH FOUND FILE %d \r\n", str.len[fi]);

					err = send_chunk_spi(con, header, str.len[fi]);

					LOG_T(LOG_HTTPSERVER, LOG_HTTP_TAG, "YEAHH FOUND FILE %d \r\n", str.len[fi]);

				}
			pbuf_free(p);
			if (err == 0) {
				LOG_I(LOG_HTTPSERVER, LOG_HTTP_TAG, "Done sending 3, therefore closing \r\n");
				server_close(pcb);
			}
		}
	} else {
		LOG_E(LOG_HTTPSERVER, LOG_HTTP_TAG, "server_recv(): Errors-> \r\n");
		if (err != ERR_OK)
			LOG_E(LOG_HTTPSERVER, LOG_HTTP_TAG, "1) Connection is not on ERR_OK state, but in %d state->\r\n", err);

		if (p == NULL) {
			LOG_E(LOG_HTTPSERVER, LOG_HTTP_TAG, "2) Pbuf pointer p is a NULL pointer->\r\n");
			LOG_E(LOG_HTTPSERVER, LOG_HTTP_TAG, "2) Remote computer closed connection \r\n");
		} else {
			pbuf_free(p);
		}
		if(pcb != 0)
		server_close(pcb);
	}
	LOG_H(LOG_HTTPSERVER, LOG_HTTP_TAG, "done receive cb\r\n");
	return ERR_OK;
}

void closeAll(u32_t ip) {
	//os_timer_disarm(&pollTimer);
	LOG_I(LOG_HTTPSERVER, LOG_HTTP_TAG, "POLL \r\n");
		connections * con;
		connections ** connection_listt = getConnectionsBegin(normal_connection_list);
		for (int i = 0; i < HTTP_POOL; i++) {
			if (connection_listt[i] != 0) {
				con = connection_listt[i];
				if (con->connection->remote_ip.addr == ip) {
						server_close(con->connection);
				}
			}
		}

}

err_t server_poll(void *arg, struct tcp_pcb *tpcb) {
	//os_timer_disarm(&pollTimer);

	connections * con = getConnection(normal_connection_list, tpcb);
	if (con != 0) {
		con->timeout++;
		LOG_T(LOG_HTTPSERVER, LOG_HTTP_TAG, "server_poll(): timout  %d - pcb  %p  -  %p \r\n",  con->timeout, con->connection, system_get_free_heap_size());
		if (con->timeout > 10) {
			if (con->websocket == 1) {
				//closeAll(con->connection->remote_ip.addr);
				LOG_I(LOG_HTTPSERVER, LOG_HTTP_TAG, "Timeout websocket, therefore closing \r\n");
				server_close(con->connection);
				return ERR_ABRT;
			} else if (con->websocket == 0) {
				LOG_I(LOG_HTTPSERVER, LOG_HTTP_TAG, "Timeout normal socket, therefore closing \r\n");
				server_close(con->connection);
				return ERR_ABRT;
			}
		}
		return ERR_OK;
	} else {
		LOG_E(LOG_HTTPSERVER, LOG_HTTP_TAG, "UNKNOWN POLL: %p state:%d \r\n", tpcb, tpcb->state);
//		  CLOSED      = 0,
//		  LISTEN      = 1,
//		  SYN_SENT    = 2,
//		  SYN_RCVD    = 3,
//		  ESTABLISHED = 4,
//		  FIN_WAIT_1  = 5,
//		  FIN_WAIT_2  = 6,
//		  CLOSE_WAIT  = 7,
//		  CLOSING     = 8,
//		  LAST_ACK    = 9,
//		  TIME_WAIT   = 10
		//server_close(tpcb);
		return ERR_OK;
	}

	//os_timer_arm(&pollTimer, 1000, 0);
}

static void ICACHE_FLASH_ATTR server_err(void *arg, err_t err) {
	LWIP_UNUSED_ARG(arg);
	LWIP_UNUSED_ARG(err);
	LOG_E(LOG_HTTPSERVER, LOG_HTTP_TAG, "\nserver_err(): Fatal error, exiting...\n");
	//	server_init(data_callback); // TODO
	//}
	return;
}

