#include "espmissingincludes.h"
#include "connection_list.h"
#include "log.h"
#include "mem.h"


static ICACHE_FLASH_ATTR void connection_printlist(connections * connection_list[HTTP_POOL]) {
	for (int i = 0; i < HTTP_POOL; i++) {
		uint32 tw = connection_list[i];
		if(tw != 0)
			tw = connection_list[i]->connection;

		LOG_I(LOG_HTTPSERVER, LOG_HTTP_TAG, "[%d] connection: %p \r\n", i, tw);
	}
}
void ICACHE_FLASH_ATTR deleteConnection(connections * connection_list[HTTP_POOL], struct tcp_pcb* pcb) {
	int ret = 0;
	for (int i = 0; i < HTTP_POOL; i++) {
		if (connection_list[i] != 0 && connection_list[i]->connection == pcb) {
			LOG_T(LOG_HTTPSERVER, LOG_HTTP_TAG,"Deleteconn %p \r\n", connection_list[i]->connection);
			os_free(connection_list[i]);
			connection_list[i] = 0;
			ret= 1;
		}
	}
	if(ret == 0) {
		os_printf("Couldnt remove connection: %p  total: %d \r\n", pcb, getConnectionSize(connection_list));
	}
	if(current_settings[FRC_CONSOLE_ENABLE])
	connection_printlist(connection_list);
}
connections* ICACHE_FLASH_ATTR newConnection(connections * connection_list[HTTP_POOL], struct tcp_pcb* pcb) {
	for (int i = 0; i < HTTP_POOL; i++) {
		if (connection_list[i] == 0) {
			//if(console_output)
			LOG_T(LOG_HTTPSERVER, LOG_HTTP_TAG, "newConnection %d - %p - %d \r\n", i, connection_list[i], sizeof(connections));
			connection_list[i] = os_malloc(sizeof(connections));
			connection_list[i]->websocket = 0;
			connection_list[i]->timeout = 0;
			connection_list[i]->dataleft = 0;
			connection_list[i]->filepos = 0;
			connection_list[i]->connection = pcb;
			if(current_settings[FRC_CONSOLE_ENABLE])
			connection_printlist(connection_list);
			return connection_list[i];
		}
	}
	return 0;
}

connections* ICACHE_FLASH_ATTR getConnectionOffset(connections * connection_list[HTTP_POOL], struct tcp_pcb * pcb, int off) {
	//os_printf("find connection: %p \r\n", pcb);
	for (int i = off + 1; i < HTTP_POOL; i++) {
		if (connection_list[i] != 0 && connection_list[i]->connection == pcb) {
			connection_list[i]->pos = i;
			return connection_list[i];
		}
	}
	return 0;
}


connections** ICACHE_FLASH_ATTR getConnectionsBegin(connections * connection_list[HTTP_POOL]) {
	return connection_list;
}


int ICACHE_FLASH_ATTR getConnectionSizeWebsocket(connections * connection_list[HTTP_POOL]) {
	int ret = 0;
	for (int i = 0; i < HTTP_POOL; i++) {
		if (connection_list[i] != 0 && connection_list[i]->websocket == 1) {
			ret++;
		}
	}
	if(current_settings[FRC_CONSOLE_ENABLE])
	connection_printlist(connection_list);
	return ret;
}

int ICACHE_FLASH_ATTR getConnectionSize(connections * connection_list[HTTP_POOL]) {
	int ret = 0;
	for (int i = 0; i < HTTP_POOL; i++) {
		if (connection_list[i] != 0) {
			ret++;
		}
	}
	if(current_settings[FRC_CONSOLE_ENABLE])
	connection_printlist(connection_list);
	return ret;
}

connections* ICACHE_FLASH_ATTR getConnection(connections * connection_list[HTTP_POOL], struct tcp_pcb * pcb) {
	//os_printf("find connection: %p \r\n", pcb);
	//connection_printlist(connection_list);
	for (int i = 0; i < HTTP_POOL; i++) {
		if (connection_list[i] != 0 && connection_list[i]->connection == pcb) {
			connection_list[i]->pos = i;
			return connection_list[i];
		}
	}
	return 0;
}

