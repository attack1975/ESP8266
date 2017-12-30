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

static ETSTimer callbackTimer;


static websocket_gotdata data_callback;

#define HTTP_BUFFER_SIZE (1000)

void ICACHE_FLASH_ATTR server_init(websocket_gotdata call) {
	struct tcp_pcb *pcb;
	pcb = tcp_new();
	tcp_bind(pcb, IP_ADDR_ANY, 8000); //server port for incoming connection
	pcb = tcp_listen(pcb);
	tcp_accept(pcb, server_accept);
	data_callback = call;
	websocket_init(call);

	//os_timer_setfn(&pollTimer, server_poll, 0);
	//os_timer_arm(&pollTimer, 1000, 0);
}



void server_kill(void * tKill) {
	if(tKill != 0) {
		tcp_abort((struct tcp_pcb *) tKill);
		tKill = 0;
	}
}
void ICACHE_FLASH_ATTR server_close(struct tcp_pcb *pcb) {

	tcp_arg(pcb, NULL);
	tcp_sent(pcb, NULL);
	tcp_recv(pcb, NULL);
	tcp_err(pcb, NULL);
	deleteConnection(pcb);
	int i = tcp_close(pcb);
	if(i == 0) {
		os_timer_disarm(&callbackTimer);
		os_timer_setfn(&callbackTimer, server_kill, pcb);
		os_timer_arm(&callbackTimer, 10, 0);
	}
	os_printf("\nserver_close(): Closing...%d-%d \n", i,0);
}

static err_t ICACHE_FLASH_ATTR server_accept(void *arg, struct tcp_pcb *pcb, err_t err) {
	LWIP_UNUSED_ARG(arg);
	LWIP_UNUSED_ARG(err);
	tcp_setprio(pcb, TCP_PRIO_MIN);
	tcp_arg(pcb, NULL);
	tcp_recv(pcb, server_recv);
	tcp_err(pcb, server_err);
	tcp_sent(pcb, server_sent);
	//tcp_poll(pcb, server_poll, 4); //every two seconds of inactivity of the TCP connection
	tcp_accepted(pcb);
	os_printf("\nserver_accept(): Accepting incoming connection on server...\n");
	return ERR_OK;
}


static err_t ICACHE_FLASH_ATTR server_sent(void *arg, struct tcp_pcb *pcb, u16_t len) {
	connections * con = getConnection(pcb);
	if (con != 0 && con->dataleft > 0) {
		int flag = TCP_WRITE_FLAG_MORE;
		long filesize = con->dataleft;
		if (con->dataleft > HTTP_BUFFER_SIZE) {
			filesize = HTTP_BUFFER_SIZE;
		} else {
			flag = 0;
		}

		int i = tcp_write(pcb, con->data + con->filepos, filesize, flag);
		os_printf("ERROR CODE: %d \r\n", i);
		if (i == 0) {
			con->filepos += filesize;
			con->dataleft -= filesize;
		}
		if (con->dataleft <= 0) {
			con->dataleft = 0;
			con->filepos = 0;
			if(con->websocket == 0) {
				server_close(pcb);
			}
		}
	} else {
		if (con != 0 && con->websocket == 0) {
			server_close(pcb);
		}
	}

	return 0;
}



static err_t ICACHE_FLASH_ATTR server_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err) {
	char *string;
	int length;
	LWIP_UNUSED_ARG(arg);
	os_printf("pcb: %p \r\n", pcb);

	connections* con = getConnection(pcb);
	if (con == 0) {
		os_printf("create connection: %p \r\n", pcb);
		con = newConnection(pcb);
		if (con == 0) {
			os_printf("ERROR creating new conn \r\n");
			return ERR_MEM;
		}
	} else {
		con->timeout = 0;
	}

	if (err == ERR_OK && p != NULL) {
		pcb->flags |= TF_NODELAY;  //TF_NAGLEMEMERR
		string = p->payload;
		length = strlen(string);
		//os_printf("string:%s \r\n", string);
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
			os_printf("WEBSOCKET MESSAGE \r\n");
		}

		if(con->websocket == 1 && p->len > 0) {
			websocket_recv(string,url, con, p);
			pbuf_free(p);
		} else {
			os_printf("NORMAL MESSAGE: %s \r\n", url);
			int file = 1;
			char * header = HEADER_OK;
			if(strstr(url, "/socket.js") != 0) {
				file = 0;
			} else if(strstr(url, "/style.css") != 0) {
				header = HEADER_CSS_OK;
				file = 2;
			}
			int err = send_chunk(con->connection, header, &rofs_data[ro_file_system.files[file].offset], ro_file_system.files[file].size);
			pbuf_free(p);
			if (err == 0) {
				server_close(pcb);
			}
		}
	} else {
		os_printf("\nserver_recv(): Errors-> ");
		if (err != ERR_OK)
			os_printf("1) Connection is not on ERR_OK state, but in %d state->\n", err);

		if (p == NULL) {
			os_printf("2) Pbuf pointer p is a NULL pointer->\n ");
			os_printf("2) Remote computer closed connection \n ");
			con->websocket = 0;
		} else {
			pbuf_free(p);
		}

		if (con->websocket != 1) {
			os_printf("server_recv(): Closing server-side connection...");
			server_close(pcb);
		} else {
			os_printf("server_recv(): Closing  SOCKET...");
			con->websocket = 0;
		}
	}

	return ERR_OK;
}




void closeAll(u32_t ip) {
	//os_timer_disarm(&pollTimer);
		os_printf("POLL \r\n");
		connections * con;
		connections ** connection_list = getConnectionsBegin();
		for (int i = 0; i < HTTP_POOL; i++) {
			if (connection_list[i] != 0) {
				con = connection_list[i];
				if (con->connection->remote_ip.addr == ip) {
						server_close(con->connection);
				}
			}
		}

}

err_t server_poll(void *arg, struct tcp_pcb *tpcb) {
	//os_timer_disarm(&pollTimer);

	connections * con = getConnection(tpcb);

		if (con != 0) {
			con->timeout++;
			os_printf("server_poll(): timout  %d - pcb  %p  -  %p \r\n",  con->timeout, con->connection, system_get_free_heap_size());
			if (con->timeout > 10) {
				if (con->websocket == 1 && (con->timeout > 20)) {
					//closeAll(con->connection->remote_ip.addr);
					server_close(con->connection);
				} else if (con->websocket == 0) {
					server_close(con->connection);
				}
			}
		} else {
			server_close(tpcb);
			os_printf("UNKNOWN POLL \r\n");
		}

	//os_timer_arm(&pollTimer, 1000, 0);
}

static void ICACHE_FLASH_ATTR server_err(void *arg, err_t err) {
	LWIP_UNUSED_ARG(arg);
	LWIP_UNUSED_ARG(err);
	os_printf("\nserver_err(): Fatal error, exiting...\n");
	server_init(data_callback); // TODO
	return;
}

