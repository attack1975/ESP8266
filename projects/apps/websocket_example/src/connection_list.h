#ifndef __CONNECTION_LIST_H
#define __CONNECTION_LIST_H

#define HTTP_POOL 20

typedef struct {
	struct tcp_pcb * connection;
	int timeout;
	const char * data;
	int dataleft;
	int filepos;
	int websocket;
	int pos;
	char is_spi;
	char is_gif;
	char spi_num;
} connections;

void deleteConnection(connections * connection_list[HTTP_POOL], struct tcp_pcb* pcb);
connections* newConnection(connections * connection_list[HTTP_POOL], struct tcp_pcb* pcb);
connections* getConnectionOffset(connections * connection_list[HTTP_POOL], struct tcp_pcb * pcb, int off);
connections* getConnection(connections * connection_list[HTTP_POOL], struct tcp_pcb * pcb);
int getConnectionSize(connections * connection_list[HTTP_POOL]);
int getConnectionSizeWebsocket(connections * connection_list[HTTP_POOL]);
connections** getConnectionsBegin(connections * connection_list[HTTP_POOL]);
#endif
