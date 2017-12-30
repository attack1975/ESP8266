#include "lwipopts.h"
#include "lwip/sockets.h"
#include "lwip/ip_addr.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "espmissingincludes.h"
#include "html_dataparser.h"
#include "connection_list.h"
#include "framedriver.h"

extern uint32_t current_settings[FRC_INVALID];

extern char console_output;
int ICACHE_FLASH_ATTR send_chunk_spi(connections * con, char*header, int len) {
	//os_printf("YEAHH FOUND FILE %d \r\n", str.len[fi]);

	int file = 0;
	if (con != 0) {
		if(current_settings[FRC_CONSOLE_ENABLE])
		os_printf("YEAHH SENDING HEADER \r\n");

		int flag = TCP_WRITE_FLAG_MORE | TCP_WRITE_FLAG_COPY;
		char buf[os_strlen(header) + 200];
		if(header != HEADER_GIF_OK) {
			os_sprintf(buf, "%sContent-Length: %d \r\n\r\n", header, len);
		} else {
			os_sprintf(buf, "%s\r\n", header);
		}
		if(current_settings[FRC_CONSOLE_ENABLE])
		os_printf("YEAHH SENDING HEADER 2 \r\n");


		int i = tcp_write(con->connection, buf, os_strlen(buf), flag);
		if(current_settings[FRC_CONSOLE_ENABLE])
		os_printf("YEAHH SENDING HEADER 3 %d  - %p \r\n", i, con->connection);
		if(i == 0)
		tcp_output(con->connection);

		if(current_settings[FRC_CONSOLE_ENABLE])
		os_printf("Return http header spi flash\r\n");

		if (con->dataleft == 0) {
			return 0;
		}
	}
	return -1;
	//os_free(d);
}


int ICACHE_FLASH_ATTR send_chunk(connections *  con, char*header, char * data, int len) {
	int file = 0;

	if (con != 0) {
		con->dataleft = len;
		int flag = TCP_WRITE_FLAG_MORE;
		if(len == 0)
			flag = 0;
		con->data = data;
		tcp_write(con->connection, header, os_strlen(header), flag);
		tcp_output(con->connection);
		if (con->dataleft == 0) {
			return 0;
		}
	}
	return -1;
	//os_free(d);
}
