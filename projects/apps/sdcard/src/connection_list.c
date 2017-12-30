#include "espmissingincludes.h"
#include "connection_list.h"
#include "lwip/mem.h"

static connections * connection_list[HTTP_POOL] = { 0 };

connections* ICACHE_FLASH_ATTR findWebsocketConnection() {
	for (int i = 0; i < HTTP_POOL; i++) {
		if (connection_list[i] != 0 && connection_list[i]->websocket == 1) {
			connection_list[i]->pos = i;
			return connection_list[i];
		}
	}
	return 0;
}

void ICACHE_FLASH_ATTR deleteConnection(struct tcp_pcb* pcb) {
	for (int i = 0; i < HTTP_POOL; i++) {
		if (connection_list[i] != 0 && connection_list[i]->connection == pcb) {
			os_printf("Deleteconn %p \r\n", connection_list[i]->connection);
			os_free(connection_list[i]);
			connection_list[i] = 0;
		}
	}
}
connections* ICACHE_FLASH_ATTR newConnection(struct tcp_pcb* pcb) {
	for (int i = 0; i < HTTP_POOL; i++) {
		if (connection_list[i] == 0) {
			os_printf("newConnection %d - %p - %d \r\n", i, connection_list[i], sizeof(connections));
			connection_list[i] = os_malloc(sizeof(connections));
			connection_list[i]->websocket = 0;
			connection_list[i]->timeout = 0;
			connection_list[i]->dataleft = 0;
			connection_list[i]->filepos = 0;
			connection_list[i]->connection = pcb;
			return connection_list[i];
		}
	}
	return 0;
}

connections* ICACHE_FLASH_ATTR getConnectionOffset(struct tcp_pcb * pcb, int off) {
	//os_printf("find connection: %p \r\n", pcb);
	for (int i = off + 1; i < HTTP_POOL; i++) {
		if (connection_list[i] != 0 && connection_list[i]->connection == pcb) {
			connection_list[i]->pos = i;
			return connection_list[i];
		}
	}
	return 0;
}

connections** ICACHE_FLASH_ATTR getConnectionsBegin() {
	return connection_list;
}

connections* ICACHE_FLASH_ATTR getConnection(struct tcp_pcb * pcb) {
	//os_printf("find connection: %p \r\n", pcb);
	for (int i = 0; i < HTTP_POOL; i++) {
		if (connection_list[i] != 0 && connection_list[i]->connection == pcb) {
			connection_list[i]->pos = i;
			return connection_list[i];
		}
	}
	return 0;
}

