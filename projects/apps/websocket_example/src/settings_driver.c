/*
 * settings_driver.c
 *
 *  Created on: Jul 26, 2016
 *      Author: wouters
 */

#include "espmissingincludes.h"
#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "log.h"
extern connections * normal_connection_list[HTTP_POOL];

static ETSTimer frameCounterTimer;
int client_connect = 0;
static int framespeed = 10;


int fontcolorR = 0xFF;
int fontcolorG = 0xFF;
int fontcolorB = 0xFF;
struct client_str client_list_ip[10] = {0};

int animationenabled = 0;
char flashing = 0;
int flashingenabled = 0;

char framecountisoff = 0;
char textoffsetisoff = 0;
char animation_off = 0;
extern char lowbatterytext[20];


char animation_custom = 0;
char textbuf[25] = { 0 };
static char textcol[30] = { 0 };

extern char ap_ssid[32];
extern char ap_password[32];
extern char client_ssid[32];
extern char client_password[32];
extern char animation_random_choises[100];

extern u32_t own_ip;
extern char batterymonitor;
extern char lowbatterymode;
extern ETSTimer sendSettingsTimer;
extern ETSTimer tickTimer;

uint32_t current_settings[FRC_INVALID] = {0};

void ICACHE_FLASH_ATTR sendSettingsFramecount();

void ICACHE_FLASH_ATTR settings_set_text(char * text, int size) {
	textbuf[0] = 'L';
		textbuf[1] = 'E';
		textbuf[2] = 'D';
		textbuf[3] = '-';
		textbuf[4] = 'L';
		textbuf[5] = 'I';
		textbuf[6] = 'N';
		textbuf[7] = 'E';
		textbuf[8] = 'S';
		textbuf[9] = '.';
		textbuf[10] = 'C';
		textbuf[11] = 'O';
		textbuf[12] = 'M';
		textbuf[13] = ' ';
		textbuf[14] = 0;
		current_settings[FRC_TEXT_BIG] = 1;
		write_textwall_buffer(0, textbuf, 14);
}
void ICACHE_FLASH_ATTR settings_driver_init() {
	os_timer_disarm(&frameCounterTimer);
	os_timer_setfn(&frameCounterTimer, sendSettingsFramecount, NULL);
	os_timer_arm(&frameCounterTimer, 1500, 0);
}

void ICACHE_FLASH_ATTR sendSettingsCb() {
	sendSettings(0);
}


void ICACHE_FLASH_ATTR sendSettingsFramecount() {
	if(flashing)
		return;

	os_timer_disarm(&frameCounterTimer);

	if(client_connect > 0 && tcp_enabled == 0) {
		char buffer [450];
		os_memset(buffer, 0, 450);
		int datacount = 0;
		add_setting_data_u32(buffer, &datacount,  FRC_UPDATE, FRC_ANIMATION_COUNTER, current_settings[FRC_ANIMATION_COUNTER]);
		add_setting_data_u32(buffer, &datacount,  FRC_UPDATE, FRC_ANIMATION_OFFSET, current_settings[FRC_ANIMATION_OFFSET]);
		add_setting_data_u32(buffer, &datacount,  FRC_UPDATE, FRC_FRAME_COUNTER, current_settings[FRC_FRAME_COUNTER]);
		add_setting_data_u32(buffer, &datacount,  FRC_UPDATE, FRC_TEXT_OFFSET, current_settings[FRC_TEXT_OFFSET]);
		add_setting_data_u32(buffer, &datacount,  FRC_UPDATE, FRC_ANIMATION_FRAMECOUNT, current_settings[FRC_ANIMATION_FRAMECOUNT]);
		send_add_udp(buffer, datacount);
		send_udp_flush();
	}

	os_timer_arm(&frameCounterTimer, 500, 0);
}



char ICACHE_FLASH_ATTR checkBattery() {
	static char wasoff = 0;
	static char offcounter = 0;

	if(lowbatterymode == 1) {
		wasoff = 1;
		offcounter++;
		LOG_I(LOG_USER, LOG_USER_TAG,"offcounter: %d \r\n", offcounter);

		if(offcounter > 5) {
			LOG_I(LOG_USER, LOG_USER_TAG,"DOOONEEEE: %d \r\n", offcounter);
			os_timer_disarm(&tickTimer);
			copy_color(0, 0,0,0, 255);
			system_deep_sleep(0);
			return 0;
		} else {
			write_textwall_buffer(0, lowbatterytext, os_strlen(lowbatterytext));
			fontcolorR = 255;
			fontcolorG = 0;
			fontcolorB = 0;
			animationenabled = 0;
			current_settings[FRC_GIF_VALUE] = 0;
			animation_custom = 0;
			os_timer_arm(&sendSettingsTimer, 2000, 0);
			return 0;
		}
	} else if(wasoff == 1) {
		wasoff = 0;
		os_timer_arm(&tickTimer, framespeed, 0);
	}

	uint16 adcread = system_adc_read();
	if(batterymonitor==1 && adcread < 800 && adcread != 1024 ) {
		os_memset(lowbatterytext, 0, 20);
		char * txt = "Low Battery!!  ";
		os_memcpy(lowbatterytext,txt , os_strlen(txt));
		lowbatterymode = 1;
		os_timer_arm(&sendSettingsTimer, 2000, 0);
		return 0;
	}

	return 1;

}

void ICACHE_FLASH_ATTR sendSettings(char begin) {
	uint8_t textbuffer[50];
	uint8_t buffer[450];
	os_memset(buffer, 450, 0);
	os_memset(textbuffer, 50, 0);


	LOG_T(LOG_USER, LOG_USER_TAG,"send settings cb \r\n");
	os_timer_disarm(&sendSettingsTimer);


	int ret = checkBattery();
	if(ret == 0) {
		os_printf("BATTERY LOW! \r\n");
		return;
	}

	LOG_E(LOG_USER, LOG_USER_TAG,"heap: %d, adc: %d sockets: %d\r\n", system_get_free_heap_size(), system_adc_read(), getConnectionSize(normal_connection_list));

	ets_sprintf( buffer, " ");
	websocket_writedata_t(buffer, WS_PING);
	flush_sockets();

	if(flashing == 1) {
		return;
	}

	int datacount = 0;


	// CLIENT UDP UPDATE
	if(client_connect > 0  && tcp_enabled == 0) {
		datacount = 0;
		os_memset(textbuffer, 0, 50);
		os_memset(buffer, 0, 450);
		ets_sprintf( textbuffer, "%02X%02X%02X", fontcolorR & 0xFF, fontcolorG & 0xFF, fontcolorB & 0xFF);
		add_setting_data_string(buffer, &datacount,  FRC_UPDATE, FRC_TEXT_COLOR, textbuffer, os_strlen(textbuffer));

		os_memset(textbuffer, 0, 50);
		textbuffer[0] = ',';
		for(int i = 0; i < 100; i++) {
			if(animation_random_choises[i] != '0' && animation_random_choises[i] != 255) {
				ets_sprintf( textbuffer, "%s%d,", textbuffer, animation_random_choises[i]);
			}
		}
		add_setting_data_string(buffer, &datacount,  FRC_UPDATE, FRC_SLIDESHOW_CHOISE, textbuffer, os_strlen(textbuffer));

		// TEXT DATA
		if(animationenabled == 0 && animation_custom == 0) {
			if(current_settings[FRC_TEXT_SCROLL_MULTIPLE] && client_connect > 0) {
				char count = 0;
				os_memset(textbuffer, 0, 50);
				textbuffer[0] = ':';
				for(int i = 0; i < 10; i++) {
					if(client_list_ip[i].ip_addr.addr  != 0) {
						os_sprintf(textbuffer, "%s%d:", textbuffer, ((client_list_ip[i].ip_addr.addr & 0xFF000000) >> 24));
						count++;
						addemptyframe(textbuffer, textbuffer, count);
						os_sprintf(textbuffer, "%s%s", textbuffer, textbuf);
						addemptyframe(textbuffer, textbuffer, client_connect - count);
						os_sprintf(textbuffer, "%s:", textbuffer);
					}
				}
				os_printf("Sending wide text: %s", textbuffer);
				add_setting_data_string(buffer, &datacount,  FRC_UPDATE, FRC_TEXT_WIDE_DATA, textbuffer, os_strlen(textbuffer));
			} else {
				os_sprintf(textbuffer, "%s", textbuf);
				add_setting_data_string(buffer, &datacount,  FRC_UPDATE, FRC_TEXT_DATA, textbuffer, os_strlen(textbuffer));
			}

		}
		send_add_udp(buffer, datacount);
	}

	// BEGIN HTTP UPDATE
	if(begin) {
		datacount = 0;
		os_memset(buffer, 0, 450);
		os_printf("Sending data \r\n");
		ets_sprintf( textbuffer, "%02X%02X%02X", fontcolorR, fontcolorG, fontcolorB);
		add_setting_data_string(buffer, &datacount,  FRC_UPDATE, FRC_TEXT_COLOR, textbuffer, os_strlen(textbuffer));
		add_setting_data_string(buffer, &datacount,  FRC_UPDATE, FRC_AP_USER, ap_ssid, os_strlen(ap_ssid));
		add_setting_data_string(buffer, &datacount,  FRC_UPDATE, FRC_AP_PASSWORD, ap_password, os_strlen(ap_password));
		add_setting_data_string(buffer, &datacount,  FRC_UPDATE, FRC_CLIENT_USER, client_ssid, os_strlen(client_ssid));
		add_setting_data_string(buffer, &datacount,  FRC_UPDATE, FRC_CLIENT_PASSWORD, client_password, os_strlen(client_password));
		add_setting_data_u8(buffer, &datacount,  FRC_UPDATE, FRC_COLUMNS, COLUMNS);
		add_setting_data_u16(buffer, &datacount,  FRC_UPDATE, FRC_BUILD_NR, BUILDVERSION);
		add_setting_data_string(buffer, &datacount,  FRC_UPDATE, FRC_BUILD_TYPE, LEDTYPE, os_strlen(LEDTYPE));
		ets_sprintf( textbuffer, "%d.%d.%d.%d", own_ip & 0x000000FF, (own_ip & 0x0000FF00) >> 8, (own_ip & 0x00FF0000) >> 16,  (own_ip & 0xFF000000) >> 24);
		add_setting_data_string(buffer, &datacount,  FRC_UPDATE, FRC_IP_VALUE, textbuffer, os_strlen(textbuffer));
		add_setting_data_u8(buffer, &datacount,  FRC_UPDATE, FRC_CONNECT_CLIENT_SAVE, current_settings[FRC_CONNECT_CLIENT_SAVE]);
		add_setting_data_u8(buffer, &datacount,  FRC_UPDATE, FRC_SYNC_ALL, current_settings[FRC_SYNC_ALL]);
		add_setting_data_u8(buffer, &datacount,  FRC_UPDATE, FRC_CONSOLE_ENABLE, current_settings[FRC_CONSOLE_ENABLE]);
		websocket_writedata_size(buffer, datacount, WS_BINARY, 1);
	}

	// NORMAL UPDATE
	os_memset(buffer, 0, 450);
	datacount = 0;
	add_setting_data_u8(buffer, &datacount,  FRC_UPDATE, FRC_ANIMATION_SPEED, current_settings[FRC_ANIMATION_SPEED]);
	add_setting_data_u8(buffer, &datacount,  FRC_UPDATE, FRC_FADE_ENABLE, current_settings[FRC_FADE_ENABLE]);
	add_setting_data_u8(buffer, &datacount,  FRC_UPDATE, FRC_FADE_SPEED, current_settings[FRC_FADE_SPEED]);
	add_setting_data_u8(buffer, &datacount,  FRC_UPDATE, FRC_FLICKER_ENABLE, current_settings[FRC_FLICKER_ENABLE]);
	add_setting_data_u8(buffer, &datacount,  FRC_UPDATE, FRC_FLICKER_SPEED, current_settings[FRC_FLICKER_SPEED]);
	add_setting_data_u8(buffer, &datacount,  FRC_UPDATE, FRC_TEXT_SPEED, current_settings[FRC_TEXT_SPEED]);
	add_setting_data_u8(buffer, &datacount,  FRC_UPDATE, FRC_BRIGHTNESS_VALUE, current_settings[FRC_BRIGHTNESS_VALUE]);
	add_setting_data_u8(buffer, &datacount,  FRC_UPDATE, FRC_GIF_VALUE, current_settings[FRC_GIF_VALUE]);
	add_setting_data_u8(buffer, &datacount,  FRC_UPDATE, FRC_FLASHLIGHT_ENABLE, current_settings[FRC_FLASHLIGHT_ENABLE]);
	add_setting_data_u8(buffer, &datacount,  FRC_UPDATE, FRC_SLIDESHOW_ENABLE, current_settings[FRC_SLIDESHOW_ENABLE]);
	add_setting_data_u8(buffer, &datacount,  FRC_UPDATE, FRC_SLIDESHOW_SPEED, current_settings[FRC_SLIDESHOW_SPEED]);
	add_setting_data_u8(buffer, &datacount,  FRC_UPDATE, FRC_TEXT_BIG, current_settings[FRC_TEXT_BIG]);
	add_setting_data_u16(buffer,&datacount,  FRC_UPDATE, FRC_BATTERY_VALUE, system_adc_read());
	add_setting_data_u8(buffer, &datacount,  FRC_UPDATE, FRC_TEXT_SCROLL_MULTIPLE, current_settings[FRC_TEXT_SCROLL_MULTIPLE]);
	websocket_writedata_size(buffer, datacount, WS_BINARY, 1);

	if(client_connect > 0 && tcp_enabled == 0) { // IF CLIENT ALSO SEND TO CLIENTS
		send_add_udp(buffer, datacount);
		send_udp_flush();
	}

	if(current_settings[FRC_CONSOLE_ENABLE])
		LOG_I(LOG_USER, "SendSettings","send settings cb exit \r\n");

	os_timer_arm(&sendSettingsTimer, 2000, 0);
}

void ICACHE_FLASH_ATTR process_text_wide(char * datas, int size) {
	//LOG_I(LOG_DATA, LOG_DATA_TAG, "TEXT WIDE DATA:%s-\r\n",datas);

	char* charpos = 0;
	charpos = strchr(&datas[0], ':');

	while(charpos != 0) {
		char tmpip[5];
		os_memset(tmpip, 0, 5);
		char * nextpost = strchr(charpos+1, ':');
		if(nextpost != 0) {
			int ipsize = nextpost - charpos;
			os_memcpy(tmpip, charpos+1,	ipsize-1);
			int lastownip = ((own_ip & 0xFF000000) >> 24);
			char *ptr;
			long i;
			i = strtol(tmpip, &ptr, 10);
			int mine = 0;
			if(lastownip == i) {
				mine = 1;
			}
			char tmptxt[30];
			os_memset(tmptxt, 0, 30);
			char * nextposttxt = strchr(nextpost+1, ':');
			if(nextposttxt != 0) {
				int txttmpsize = nextposttxt - (nextpost) - 1;
				os_memcpy(tmptxt, nextpost+1,	txttmpsize);
				if(mine) {
					//os_memset(textbuf, 0, 30);
					os_memcpy(textbuf, tmptxt, txttmpsize);
					textbuf[txttmpsize] = 0;

					//os_printf("Setting text buffer: -%s- size:%d \r\n", textbuf, txttmpsize);
					//LOG_I(LOG_DATA, LOG_DATA_TAG, "RECEIVED TXT: -%s-\r\n", textbuf);

				}
				nextpost = nextposttxt;
			}
		}
		charpos = nextpost;
	}
}

void ICACHE_FLASH_ATTR process_slideshow_choise(char * dat, int size) {
	os_memset(&animation_random_choises[0], 255, 100);
	int i, count;
	count=0;
	if(dat[1] == '0') {
		animation_random_choises[0] = '0';
		LOG_D(LOG_DATA, "INITIAL RANDOM: %d\r\n", 0);
		count=1;
	}
	char * pdat = &dat[0];
	while(pdat != 0) {
		char tbuf[5] = {0};
		os_memset(tbuf, 0, 5);
		char * oldp = pdat;
		pdat = strstr(oldp+1, ",");
		if(pdat != 0) {
			int len = pdat - oldp;
			os_memcpy(tbuf, oldp+1, len);
			int b = atoi(tbuf);
			if(b != 0) {
				LOG_D(LOG_DATA, "choise: %d \r\n", b);
				animation_random_choises[count] =  b;
				count += 1;
			}
		}
	}
}

char ICACHE_FLASH_ATTR process_data_receive(char * data, int size, connections *con, int offset, int totalsize) {
	static char textisoff = 0;
	static char animationframesyncisoff = 0;
	static char framesyncisoff = 0;
	char ret = 1;
	char type = data[2];
	if(type > FRC_INVALID) {
		//hex
		os_printf("Wrong type update data: %d \r\n", type);
		return 0;
	}

	//os_printf("Receiving settings data size:%d type:%d val: %d offset: %d total: %d \r\n", size, type, data[3], offset, totalsize);

	char dat[size];
	os_memset(dat, 0, size);

	uint32_t val = 0;
	switch (size) {
		case 3:
			val = data[3];
			break;
		case 4:
			val = ((data[3] << 8) | data[4]);
			break;
		case 5:
			val = ((data[3] << 16) | (data[4] << 8)  | data[5]);
			break;
		case 6:
			val = ((data[3] << 24) | (data[4] << 16) | (data[5] << 8) | data[6]);
			break;
		default:
			os_memcpy(dat, &data[3], size - 2);
			break;
	}

	char set = 1;
	long diff = 0;

	//os_printf("Receiving settings data \r\n");


	switch (type) {
		case FRC_BATTERY_VALUE:
			if(tcp_enabled == 0) {
			websocket_writedata_size(data, size+1, WS_BINARY, 1);
			}
			ret = 2;
			break;
		case FRC_GIF_VALUE:
			if(val == 0) {
				animationenabled = 0;
			} else {
				animationenabled = 1;
			}
			break;
		case FRC_TEXT_SCROLL_MULTIPLE:
			if(tcp_enabled == 0 &&  (val == 1)) {
				addemptyframe(dat, textbuf, client_connect);
			} else {
				os_sprintf(dat, "%s", textbuf);
			}
			write_textwall_buffer(0, dat, os_strlen(dat));
			break;
		case FRC_FLICKER_SPEED:
			if(val <= 0)
				val = 1;
			break;
		case FRC_FADE_SPEED:
			if(val <= 0)
				val = 1;
			break;
		case FRC_SLIDESHOW_CHOISE:
			os_printf("slideshow choise:%s \r\n", dat);
			process_slideshow_choise(dat, os_strlen(dat));
			break;
		case FRC_TEXT_COLOR:
			LOG_D(LOG_DATA, "Text color:%s\r\n", dat);
			os_memset(textcol, 0, 32);
			os_memcpy(textcol, &dat[0], 2);
			fontcolorR = h2i(textcol);
			os_memset(textcol, 0, 32);
			os_memcpy(textcol, &dat[2], 2);
			fontcolorG = h2i(textcol);
			os_memset(textcol, 0, 32);
			os_memcpy(textcol, &dat[4], 2);
			fontcolorB = h2i(textcol);
			break;
		case FRC_TEXT_BIG:
			current_settings[type] = val;
			if(tcp_enabled == 0 && current_settings[FRC_TEXT_SCROLL_MULTIPLE] == 1) {
				addemptyframe(dat, textbuf, client_connect);
			} else {
				os_sprintf(dat, "%s", textbuf);
			}
			write_textwall_buffer(0, dat, os_strlen(dat));
			break;
		case FRC_CONNECT_CLIENT_SAVE:
			writeWifiSettings(ap_ssid, ap_password, client_ssid, client_password, val);
			system_restart();
			break;
		case FRC_CONNECT_CLIENT_NOW:
			connectToAp2();
			break;
		case FRC_TEXT_WIDE_DATA:
			//os_printf("text wide data \r\n");
			process_text_wide(dat, size-3);
			break;
		case FRC_TEXT_DATA:
			//os_printf("Got text data: -%s- \r\n", dat);
			os_memset(textbuf, 0, 25);
			os_memcpy(textbuf, dat, os_strlen(dat));
			if(tcp_enabled == 0 &&  (current_settings[FRC_TEXT_SCROLL_MULTIPLE] == 1)) {
				addemptyframe(dat, textbuf, client_connect);
			} else {
				os_memcpy(dat, textbuf, size - 3);
			}
			write_textwall_buffer(0, &dat[0], os_strlen(dat));
			animationenabled = 0;
			animation_custom = 0;
			current_settings[FRC_GIF_VALUE] = 0;
			LOG_D(LOG_DATA, "Setting text buffer: %s\r\n", &dat[0]);
			break;
		case FRC_ANIMATION_OFFSET:
			ret = 2;
			diff = val - current_settings[FRC_ANIMATION_OFFSET];
			if(diff > 10 || diff < -10) {
				current_settings[FRC_ANIMATION_OFFSET] = (val);
			}
			if(diff > 2 && current_settings[FRC_GIF_VALUE] < 4) {
				LOG_D(LOG_DATA, "ANIMATION OFFSET: %ld \r\n", diff);
				os_printf("animation offset \r\n");
				current_settings[FRC_ANIMATION_OFFSET] += 1;
			} else if( diff < -2 && current_settings[FRC_GIF_VALUE] < 4 ) {
				os_printf("animation offset \r\n");
				current_settings[FRC_ANIMATION_OFFSET] += -1;
			}
			set = 0;
			break;
		case FRC_TEXT_OFFSET:
			ret = 2;
			val += 1;
			diff = (val)  - current_settings[FRC_TEXT_OFFSET];
			if(diff > 10 || diff < -10) {
				os_printf("TEXT OFFSET hard reset \r\n");
				current_settings[FRC_TEXT_OFFSET] = val;
				textisoff = 0;
			} else 	if(diff > 1) {
				textisoff++;
				if(current_settings[FRC_TEXT_OFFSET] > 1 && textisoff > 1) {
					os_printf("TEXT OFFSET +1: %ld - %ld  = %d\r\n", val, current_settings[FRC_TEXT_OFFSET], diff);
					current_settings[FRC_TEXT_OFFSET] += 1;
					textisoff = 0;
				}
			} else if(diff < -1) {
				textisoff++;
				if(current_settings[FRC_TEXT_OFFSET] > 1 && textisoff > 1) {
					os_printf("TEXT OFFSET -1: %ld - %ld  = %d\r\n", val, current_settings[FRC_TEXT_OFFSET], diff);
					current_settings[FRC_TEXT_OFFSET] -= 1;
					textisoff = 0;
				}
			} else {
				textisoff = 0;
			}
			set = 0;
			break;
		case FRC_ANIMATION_FRAMECOUNT:
			ret = 2;
			val += 3;
			diff = ((val) - current_settings[FRC_ANIMATION_FRAMECOUNT]);
			if((diff > 10 || diff < -10) && (animationframesyncisoff > 1)) {
				os_printf("animation framecount hard \r\n");
				current_settings[FRC_ANIMATION_FRAMECOUNT] = (val);
				animationframesyncisoff = 0;
			} else 	if((diff >  2 && current_settings[FRC_ANIMATION_SPEED] > 2)) {
				animationframesyncisoff++;
				if(animationframesyncisoff > 2) {
					current_settings[FRC_ANIMATION_FRAMECOUNT] += 3;
					os_printf("animation framecount +1 s:%d me:%d diff:%d \r\n", val, current_settings[FRC_ANIMATION_FRAMECOUNT], diff);
					animationframesyncisoff = 0;
				}
			} else if((diff < -2 && current_settings[FRC_ANIMATION_SPEED] > 2)) {
				animationframesyncisoff++;
				if(animationframesyncisoff > 2) {
					current_settings[FRC_ANIMATION_FRAMECOUNT] -= 3;
					os_printf("animation framecount -1 s:%d me:%d diff:%d \r\n", val, current_settings[FRC_ANIMATION_FRAMECOUNT], diff);
					animationframesyncisoff = 0;
				}
			} else {
				animationframesyncisoff = 0;
			}
			set = 0;
			break;
		case FRC_FRAME_COUNTER:
			ret = 2;
			//os_printf("Received frame counter  %ld- %ld \r\n", val, current_settings[FRC_FRAME_COUNTER]);
			//val += 10; // this is the fast counter, so while transmitting probably extraaa
			diff = ((val) - current_settings[FRC_FRAME_COUNTER]);
			//LOG_D(LOG_DATA, "FRAME OFFSET: %ld- %ld=%ld  \r\n", val, current_settings[FRC_FRAME_COUNTER], diff);
			if(diff > 25 || diff < -25) {
				os_printf("Frame counter hard jump: %ld- %ld=%ld  \r\n", val, current_settings[FRC_FRAME_COUNTER], diff);
				current_settings[FRC_FRAME_COUNTER] = (val);
				framesyncisoff = 0;
			} else if(diff > 1) {
				framesyncisoff++;
				if(framesyncisoff > 4) {
					os_printf("Frame counter sync +10: %ld- %ld=%ld  \r\n", val, current_settings[FRC_FRAME_COUNTER], diff);
					current_settings[FRC_FRAME_COUNTER] += 1;
					framesyncisoff = 0;
				}
			} else if(diff < -1) {
				framesyncisoff++;
				if(framesyncisoff > 4) {
					os_printf("Frame counter sync +10: %ld- %ld=%ld  \r\n", val, current_settings[FRC_FRAME_COUNTER], diff);
					current_settings[FRC_FRAME_COUNTER] -= 1;
					framesyncisoff = 0;
				}
			} else {
				framesyncisoff = 0;
			}
			set = 0;
			break;
		case FRC_OPEN_DATA:
			sendSettings(1);
			break;
		case FRC_CLIENT_USER:
			os_memset(client_ssid, 0, 32);
			os_memcpy(client_ssid, dat, size -3);
			LOG_D(LOG_DATA, "Settings client User \r\n");
			break;
		case FRC_CLIENT_PASSWORD:
			os_memset(client_password, 0, 32);
			os_memcpy(client_password, dat, size - 3);
			LOG_D(LOG_DATA, "Settings client password \r\n");
			writeWifiSettings(ap_ssid, ap_password, client_ssid, client_password, 1);
			system_restart();
			break;
		case FRC_AP_USER:
			os_memset(ap_ssid, 0, 32);
			os_memcpy(ap_ssid, dat, size - 3);
			LOG_D(LOG_DATA, "Settings Ap User \r\n");
			break;
		case FRC_AP_PASSWORD:
			os_memset(ap_password, 0, 32);
			os_memcpy(ap_password, dat, size - 3);
			LOG_D(LOG_DATA, "Settings Ap password \r\n");
			writeWifiSettings(ap_ssid, ap_password, client_ssid, client_password, 0);
			system_restart();
			break;

		default:
			//os_printf("No post processing for type: %d \r\n", type);
			break;
	}

	if(set == 1)
	current_settings[type] = val;

	//os_printf("Going to check for more data: size %d offset %d-%d total %d \r\n", size, offset, (size + 1 + offset), totalsize);
	if((size + 3 + offset) < totalsize) {
		offset += size + 1;
		char retsub = 1;
		retsub = process_data_receive(&data[size+1], data[size+1], con, offset, totalsize);
		if(retsub == 2)
			ret = 2;
	}
	return ret;

}
