/*
 * udp_layer.c
 *
 *  Created on: Jul 26, 2016
 *      Author: wouters
 */



#include "espmissingincludes.h"
#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "log.h"
extern struct udp_pcb * pUdpConnection;
int sizen = 0;

extern int server_connect;
extern char udpdata[1024];
extern int client_connect;
extern int udpcount;
extern struct client_str client_list_ip[10];
extern char textbuf[25];
void ICACHE_FLASH_ATTR add_setting_data_string(char * buffer, int * point, char type, char datatype, char* data, int size) {
	if(buffer == data) {
		os_printf("Error cannot process data \r\n");
		return;
	}
	int pos = *point;
	char * b = &buffer[pos];
	b[0] = 2 + size;
	b[1] = type;
	b[2] = datatype;
	for(int i = 0; i < size; i++) {
		b[3+i] = data[i];
	}
	*point = pos + size + 3;
}

void ICACHE_FLASH_ATTR add_setting_data_u32(char * buffer, int * point, char type, char datatype, uint32_t data) {
	int pos = *point;
	char * b = &buffer[pos];
	b[0] = 6;
	b[1] = type;
	b[2] = datatype;
	b[3] = (data & 0xFF000000) >> 24;
	b[4] = (data & 0xFF0000) >> 16;
	b[5] = (data & 0xFF00) >> 8;
	b[6] = data & 0xFF;
	*point = pos + 7;
}



void ICACHE_FLASH_ATTR add_setting_data_u16(char * buffer, int * point, char type, char datatype, uint16_t data) {
	int pos = *point;
	char * b = &buffer[pos];
	b[0] = 4;
	b[1] = type;
	b[2] = datatype;
	b[3] = (data & 0xFF00) >> 8;
	b[4] = data & 0xFF;
	*point = pos + 5;
}

void ICACHE_FLASH_ATTR add_setting_data_u8(char * buffer, int * point, char type, char datatype, char data) {
	int pos = *point;
	char * b = &buffer[pos];
	b[0] = 3;
	b[1] = type;
	b[2] = datatype;
	b[3] = data;
	*point = pos + 4;
}

void ICACHE_FLASH_ATTR addemptyframe(char * dst, char * src, int frames) {
	os_sprintf(dst, "%s", src);
	for(int i = 0; i < frames; i++) {
		os_sprintf(dst, "%s~", dst);
	}
}


void ICACHE_FLASH_ATTR process_data_udp(char * data, int sizeleft, int abssize) {
	//LOG_I(LOG_UDP, LOG_UDP_TAG, "process_data_udp: %d - %d  \r\n", sizeleft, abssize);

	if(sizeleft < 3) {
		LOG_E(LOG_UDP, LOG_UDP_TAG,"NOT ENOUGH DATA: %d \r\n", sizeleft);
		return;
	}
	if(abssize > 2000) {
		LOG_E(LOG_UDP, LOG_UDP_TAG,"Too much data  abssize: %d \r\n", abssize);
		return;
	}


	char type = data[0];
	uint32_t size = 0;
	size = data[1];
	int SIZE1 = data[2];
	if(type != 0x0F) {
		//LOG_E(LOG_UDP, LOG_UDP_TAG, "WRONG TYPE UDP: %d   sizeleft:%d \r\n", type, sizeleft);
		os_printf("WRONG TYPE UDP: type:%x  -%s-  left:%d-abs:%d \r\n", type, data, sizeleft,abssize);
		hexDump("data", data, sizeleft);
		return;
	}


	size = size | (SIZE1 << 8);

	sizen = size;
	if(size > 600) {
		LOG_E(LOG_UDP, LOG_UDP_TAG, "SIZE IS TOO BIG \r\n");
		return;
	}

	int offset = 3;
	sizeleft -= (size+3);
	if(os_strstr(&data[offset], "FX") == 0 && sizen > 3) {
		//LOG_D(LOG_UDP, LOG_UDP_TAG, "Datasize udp: %d - %d \r\n", sizeleft, sizen);
		websocket_callback(&data[offset], sizen, 0);
	}

	if(size  > 3 && sizeleft > 0) {
		//LOG_I(LOG_UDP, LOG_UDP_TAG, "Processing next udp data: %d - %d, data:-%s- \r\n", sizeleft, sizen, tmpbuf);
		process_data_udp(data+3+size, sizeleft, abssize );
	}
}

void ICACHE_FLASH_ATTR send_udp_data(char * data) {
	send_add_udp(data, os_strlen(data));
}
void ICACHE_FLASH_ATTR send_add_udp(char * data, int size) {
	if(tcp_enabled == 0 && pUdpConnection != 0 && client_connect > 0) {
		udpdata[udpcount] = 0x0F;
		udpdata[udpcount+1] = size & 0xFF;
		udpdata[udpcount+2] = (size & 0xFF00) >> 8;
		os_memcpy(&udpdata[udpcount + 3], data, size);
		udpcount += size + 3;
	}
}

void ICACHE_FLASH_ATTR send_udp_flush() {
	if(current_settings[FRC_SYNC_ALL] == 0)
			return;

	if(udpcount > 0 && pUdpConnection != 0 && tcp_enabled == 0 && client_connect > 0) {
		LOG_T(LOG_UDP, LOG_UDP_TAG, "Syncing other clients: buff[%d]\r\n", udpcount);
		send_udp_package(udpdata, udpcount, 0);
		//hexDump("", udpdata, udpcount);
		//send_udp_package(udpdata, udpcount);
		os_memset(udpdata, 0, 1024);
		udpcount = 0;
	}
}


void ICACHE_FLASH_ATTR send_udp_package(char * data, int size, int connect) {
	struct ip_addr ipSend;

	if(current_settings[FRC_SYNC_ALL] == 0)
		return;


	struct pbuf* b = pbuf_alloc(PBUF_TRANSPORT, size, PBUF_RAM);
	if(b != NULL) {
		struct udp_pcb *pcb = udp_new();
		os_memcpy (b->payload, data, size);
		int i = 0;
		LOG_T(LOG_UDP, LOG_UDP_TAG, "Going to  send: %d \r\n", i);
		if(connect == 1) {
			struct netif * nif = netif_find("ew0");
			if(nif != 0)
			i = udp_sendto_if(pcb, b, IP_ADDR_BROADCAST, 8080, nif);
			//os_printf("sending udp size: %d-\r\n", size);
		} else {
			i = udp_sendto(pcb, b, IP_ADDR_BROADCAST, 8080);
		}

		LOG_T(LOG_UDP, LOG_UDP_TAG, "Done sending: %d \r\n", i);
		if(i != ERR_OK) {
			LOG_E(LOG_UDP, LOG_UDP_TAG, "Didnt send: %d \r\n", i);
		}
		udp_remove(pcb);
		pbuf_free(b);
	}
}


void ICACHE_FLASH_ATTR handle_udp_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p,  ip_addr_t *addr, u16_t port) {
	if(flashing == 1) {
		if(p != 0)
			pbuf_free(p);
		return;
	}

	if(server_connect == 1) {
		server_connect = 0;
		if(client_connect == 0) {
			//os_timer_disarm(&callbackTimerKill);
			LOG_I(LOG_USER, LOG_USER_TAG, "YEAHH CONNECTED!!!: %d \r\n", p->len);
			if(wifi_get_opmode() != STATIONAP_MODE) {
				wifi_set_opmode( STATIONAP_MODE );
			}
		}
	}

	if(p != 0) {
		if(strstr(p->payload, "CLIENT") != 0 && tcp_enabled == 0) {
			int res = 0;
			for(int i = 0; i < 10; i++) {
				if(client_list_ip[i].ip_addr.addr  == addr->addr) {
					res = 1;
				}
			}
			if(res == 0) {
				for(int i = 0; i < 10; i++) {
					if(client_list_ip[i].ip_addr.addr  == 0) {
						client_list_ip[i].ip_addr.addr = addr->addr;
						break;
					}
				}
				//LOG_I(LOG_USER, LOG_UDP_TAG, "YEAHH NEW CLIENT CONNECTED \r\n");
				client_connect += 1;
				if(tcp_enabled == 0 &&  (current_settings[FRC_TEXT_SCROLL_MULTIPLE] == 1)) {
					addemptyframe(udpdata, textbuf, client_connect);
				} else {
					os_memcpy(udpdata, textbuf, os_strlen(textbuf));
				}
				write_textwall_buffer(0, udpdata, os_strlen(udpdata));

			}
		} else if (current_settings[FRC_SYNC_ALL] == 1 && p->len > 0) {
			process_data_udp(p->payload, p->len, p->len);
			//LOG_I(LOG_UDP, LOG_UDP_TAG, "Done processing udp data \r\n");
		}
	}

	if(p != 0)
	pbuf_free(p);
}
