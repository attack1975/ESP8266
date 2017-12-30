#include "lwipopts.h"
#include "lwip/sockets.h"
#include "lwip/ip_addr.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "espmissingincludes.h"
#include "html_dataparser.h"
#include "connection_list.h"

int ICACHE_FLASH_ATTR send_chunk(struct tcp_pcb * pcb, char*header, char * data, int len) {
	int file = 0;
	connections * con = getConnection(pcb);
	if (con != 0) {
		con->dataleft = len;
		int flag = TCP_WRITE_FLAG_MORE;
		con->data = data;
		tcp_write(pcb, header, os_strlen(header), flag);
		tcp_output(pcb);
		if (con->dataleft == 0) {
			return 0;
		}
	}
	return -1;
	//os_free(d);
}
