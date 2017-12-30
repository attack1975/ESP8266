#include "espmissingincludes.h"
#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "driver/uart.h"
#include "lwip_websocket.h"
#include "gpio.h"
#include "driver/pwm.h"
#include "include/gif.h"
#include <c_types.h>
#include "font8x8_basic.h"
#include "ssl/ssl_crypto.h"
#include "framedriver.h"
#include "fs.h"
#include "os_type.h"
#include "espconn.h"

#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include "lwip/err.h"
#include "log.h"
#include "lwip/ip_addr.h"
#include "mem.h"

char batterymonitor = 0;
ETSTimer tickTimer;

static ETSTimer callbackTimerKill;
ETSTimer sendSettingsTimer;

extern struct client_str client_list_ip[10];
extern void sendSettingsCb();
extern handle_udp_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p,  ip_addr_t *addr, u16_t port);
char wasconnectedtoclient = 0;

extern uint32_t current_settings[FRC_INVALID];


extern char lTmlBuf[totleds];

//long textoffset;
char ap_ssid[32];
char ap_password[32];
char client_ssid[32];
char client_password[32];
char animation_random_choises[100];



extern char animation_custom;
extern int animationenabled;

int server_connect = 0;

INDEXSTRUCT str;

#define INPUT_PIN 12

char udpdata[1024];
#define user_procTaskPrio        0
#define user_procTaskQueueLen    1


//static char connecttoclient;

typedef struct wifi_settings {
	char isset;
	char ssidAp[32];
	char ssidPass[32];
	char issetclient;
	char clientAp[32];
	char clientPass[32];
} wifi_settings;



int tcp_enabled = 0;
u32_t own_ip;
struct udp_pcb * pUdpConnection = NULL;
int udpcount = 0;
char lowbatterymode = 0;
char lowbatterytext[20];

static int button_was_pressed = 0;
static uint32 wastime = 0;
static uint32 wastime1 = 0;
static uint32 wastimeprev = 0;
static char brightnessset;

#ifdef LEDFIX
	char dimmode = 1;
	char compensate = 40; // 20
	char ledfix = 1; // 1
#else
	#ifdef WHITE
		char dimmode = 1;
		char compensate = 35; // 20
		char ledfix = 0; // 1
	#else
		char dimmode = 0;
		char ledfix = 0;
		char compensate = 0;
	#endif
#endif


int ICACHE_FLASH_ATTR h2i(char * ph)
{
    int result;
    if(*ph > 96 && *ph < 103) result = *ph - 87;
    else if(*ph > 64 && *ph < 71) result = *ph - 55;
    else if(*ph > 47 && *ph < 59) result = *ph - 48;
    else return -1;
    result <<= 4;
    ph++;
    if(*ph > 96 && *ph < 103) result |= *ph - 87;
    else if(*ph > 64 && *ph < 71) result |= *ph - 55;
    else if(*ph > 47 && *ph < 59) result |= *ph - 48;
    else return -1;
    return result;
}


void ICACHE_FLASH_ATTR shell_init(void)
{
	err_t err;
	struct ip_addr ipSend;
	lwip_init();
	pUdpConnection = udp_new();

	for(int i = 0; i < 10; i++) {
		client_list_ip[i].ip_addr.addr = 0;
	}
	if(pUdpConnection == NULL) {
		LOG_E(LOG_USER, LOG_USER_TAG, "Could not create new udp socket");
	}

	err = udp_bind(pUdpConnection, IP_ADDR_ANY, 8080);
	if(err != 0) {
		LOG_E(LOG_UDP, LOG_UDP_TAG,"ERROR  udp_bind");
	}

	udp_recv(pUdpConnection, handle_udp_recv, pUdpConnection);

}

void ICACHE_FLASH_ATTR send_connect_package() {
	os_timer_disarm(&callbackTimerKill);
	LOG_I(LOG_USER, LOG_USER_TAG, "SENDING UDP CONNECT \r\n");
	server_connect = 1;
	os_printf("Sending client string \r\n");
	send_udp_package("CLIENT", 6, 1);

	// BROADCAST STUFF
	if(tcp_enabled > 0) {
		os_printf("Sending adc data \r\n");
		char buffer[20];
		os_memset(buffer, 0, 20);
		char buffer2[20];
		os_memset(buffer2, 0, 20);
		int lastownip = ((own_ip & 0xFF000000) >> 24);
		ets_sprintf( buffer, "%d=%d ", lastownip, system_adc_read());
		int datacount = 0;
		add_setting_data_string(buffer2, &datacount,  FRC_UPDATE, FRC_BATTERY_VALUE, buffer, os_strlen(buffer));
		udpcount = 0;
		udpdata[udpcount] = 0x0F;
		udpdata[udpcount+1] = datacount & 0xFF;
		udpdata[udpcount+2] = (datacount & 0xFF00) >> 8;
		os_memcpy(&udpdata[udpcount + 3], buffer2, datacount);
		udpcount += datacount + 3;
		send_udp_package(udpdata, udpcount, 1);
		udpcount = 0;
		//send_udp_flush();
	}

	os_timer_setfn(&callbackTimerKill, send_connect_package, 0);
	os_timer_arm(&callbackTimerKill, 5000, 0);
}

void ICACHE_FLASH_ATTR wifi_clb(System_Event_t *event) {
	struct ip_info ip;
	static char wifitries = 0;

	if(wifi_station_get_connect_status() == STATION_NO_AP_FOUND && wifitries > 5) {
		wifi_station_disconnect();
		wifitries = 0;
		current_settings[FRC_CONNECT_CLIENT_SAVE] = 0;
	} else if(wifi_station_get_connect_status() == STATION_NO_AP_FOUND) {
		wifitries++;
	} else  if(wifi_station_get_connect_status == STATION_GOT_IP){
		current_settings[FRC_CONNECT_CLIENT_SAVE] = 1;
		wifitries = 0;
	} else {
		wifitries = 0;
	}

	switch (event->event) {
		case EVENT_STAMODE_DISCONNECTED:
			LOG_I(LOG_USER, LOG_USER_TAG, "DISCONNECTED FROM STATION\r\n", own_ip);
				tcp_enabled = 0;
				wasconnectedtoclient = 1;
			break;
		case EVENT_STAMODE_GOT_IP:
			wifi_get_ip_info(STATION_IF, &ip);
			own_ip = ip.ip.addr;
			LOG_I(LOG_USER, LOG_USER_TAG, "GOT IP %d \r\n", own_ip);
			wasconnectedtoclient = 0;
			if(ip.gw.addr == ipaddr_addr("192.168.4.1")) { // STILL WRONGGG!!!
				LOG_I(LOG_USER, LOG_USER_TAG,"===============STATION==========\r\n");
				LOG_I(LOG_USER, LOG_USER_TAG,"=========DISABLING LOGGING====\r\n");
				tcp_enabled = 1;
				shell_init();
				os_timer_disarm(&callbackTimerKill);
				os_timer_setfn(&callbackTimerKill, send_connect_package, 0);
				os_timer_arm(&callbackTimerKill, 1000, 0);

			}
			break;
		default:
			break;
	}

}

void ICACHE_FLASH_ATTR writeWifiSettings(char* ssidAp,char* ssidPass,char* clientAp,char* clientPass, char client) {
	wifi_settings wsettings;
	wsettings.isset = 1;
	wsettings.issetclient = client;
	os_memcpy(wsettings.clientAp, clientAp, 32);
	os_memcpy(wsettings.clientPass, clientPass, 32);
	os_memcpy(wsettings.ssidAp, ssidAp, 32);
	os_memcpy(wsettings.ssidPass, ssidPass, 32);
    spi_flash_erase_sector(ESP_PARAM_START_SEC + 1);
    spi_flash_write((ESP_PARAM_START_SEC + 1) * SPI_FLASH_SEC_SIZE, &wsettings, sizeof(wifi_settings));
    LOG_I(LOG_USER, LOG_USER_TAG, "Writing to sector: 0x%02X", (ESP_PARAM_START_SEC + 1) * SPI_FLASH_SEC_SIZE);

}


void ICACHE_FLASH_ATTR connectToAp() {
   // char * pass = "DigiTshirt";
	wifi_set_phy_mode( PHY_MODE_11N );
	static struct softap_config apconf;
   wifi_softap_get_config(&apconf);
   os_memset(apconf.ssid, 0, 32);
   os_strncpy((char*) apconf.ssid, ap_ssid, 32);
   os_memset(apconf.password, 0, 32);
   os_strncpy((char*) apconf.password, ap_password, 32);
   if(os_strlen(ap_password) > 4) {
	   apconf.authmode = AUTH_WPA_WPA2_PSK;
   } else {
	   apconf.authmode = AUTH_OPEN;
   }
   apconf.max_connection = 20;
   apconf.ssid_hidden = 0;
   apconf.ssid_len = os_strlen(ap_ssid);
   apconf.channel = 6;
   apconf.beacon_interval = 10;
   bool ret = wifi_softap_set_config(&apconf);
   wifi_station_set_auto_connect(false);
   wifi_station_set_reconnect_policy(true); //disable auto-reconnect

   LOG_I(LOG_USER, LOG_USER_TAG,"Setting AP ssid:'%s' password:'%s'  - %d\r\n", ap_ssid, ap_password, ret);
   wifi_softap_dhcps_start();

}

void ICACHE_FLASH_ATTR connectToAp2() {
    wifi_set_phy_mode( PHY_MODE_11N );
    //wifi_set_opmode(STATION_MODE);

    struct station_config apconf;
    wifi_station_set_auto_connect(false);
    wifi_station_set_reconnect_policy(true); //disable auto-reconnect
    wifi_station_get_config(&apconf);
    os_strncpy((char*)apconf.ssid, client_ssid, 32);
    os_strncpy((char*)apconf.password, client_password, 64);
    apconf.bssid_set = 0;
    bool ret = wifi_station_set_config(&apconf);
    LOG_I(LOG_USER, LOG_USER_TAG,"Setting client ssid:'%s' password:'%s'  - %d\r\n", client_ssid, client_password, ret);
    wifi_station_connect();
    wifi_station_set_auto_connect(false);
    wifi_station_set_reconnect_policy(true); //disable auto-reconnect
    wifi_station_dhcpc_start();
   // wifi_set_event_handler_cb(wifi_event_cb);
}



void ICACHE_FLASH_ATTR tickCb() {
	os_timer_disarm(&tickTimer);

	if(lowbatterymode == 1) {
		if(dimmode) {
			framedriver_refresh(50);
		} else {
			framedriver_refresh(100);
		}
	} else {
		if(dimmode) {
			if(current_settings[FRC_BRIGHTNESS_VALUE] > (23+compensate)) {
				framedriver_refresh(current_settings[FRC_BRIGHTNESS_VALUE]-compensate);
			} else {
				if(current_settings[FRC_BRIGHTNESS_VALUE] < 5) {
					framedriver_refresh(0);
				} else {
					framedriver_refresh(23);
				}
			}
		} else {
			if(current_settings[FRC_FLASHLIGHT_ENABLE] == 1) {
				LOG_I(LOG_USER, LOG_USER_TAG,"flashlight: %d\r\n", current_settings[FRC_FLASHLIGHT_ENABLE]);
				framedriver_refresh(150);
			} else {
				framedriver_refresh(current_settings[FRC_BRIGHTNESS_VALUE] + 20); // first 20 are almost off
			}
		}
	}

	LOG_L(LOG_USER, LOG_USER_TAG,"tickCb 3\r\n");

	if(GPIO_INPUT_GET(GPIO_ID_PIN(INPUT_PIN))) {
		if(button_was_pressed == 0) {
			wastime = system_get_time();
			wastime1 = system_get_time();
			if(wastime - wastimeprev < 150000){ // only 500ms between release of last shortpres;

				LOG_I(LOG_USER, LOG_USER_TAG,"2nd short press\r\n");
				current_settings[FRC_FLASHLIGHT_ENABLE] = 1;
				animation_prev();
				animation_prev();
				animation_prev();
				sendSettingsCb();
			} else {
				LOG_I(LOG_USER, LOG_USER_TAG,"flashlight off\r\n");
				current_settings[FRC_FLASHLIGHT_ENABLE] = 0;
				sendSettingsCb();
			}
		} else {
			uint32 now = system_get_time();
			if(now - wastime1 > 2000000 || (brightnessset == 1 && now - wastime1 > 1000000)) { // 700ms
				LOG_I(LOG_USER, LOG_USER_TAG,"BRIGHTNESS INCR\r\n");
				current_settings[FRC_BRIGHTNESS_VALUE] += 30;
				wastime1 = now;
				if(current_settings[FRC_BRIGHTNESS_VALUE] > 255) {
					current_settings[FRC_BRIGHTNESS_VALUE] = 0;
				}
				brightnessset = 1;
			}
		}
		button_was_pressed = 1;

	} else if(button_was_pressed) {
		brightnessset =0;
		uint32 now = system_get_time();
		if(now - wastime > 300000 && now - wastime < 2000000) { // 700ms
			//if(console_output)
			LOG_I(LOG_USER, LOG_USER_TAG,"BUTTON RANDOM ON\r\n");
			current_settings[FRC_SLIDESHOW_ENABLE] = !current_settings[FRC_SLIDESHOW_ENABLE];
			if(current_settings[FRC_SLIDESHOW_ENABLE] == 1) {
				current_settings[FRC_GIF_VALUE]++;
			}
		} else if(now - wastime < 300000){ // only shortpress
			current_settings[FRC_SLIDESHOW_ENABLE] = 0;
			wastimeprev = now;
			//if(console_output)
			LOG_I(LOG_USER, LOG_USER_TAG,"BUTTON %d - %d\r\n", GPIO_INPUT_GET(GPIO_ID_PIN(INPUT_PIN)), now - wastime);
			animationenabled = 1;
			animation_next();
			sendSettingsCb();

		}
		button_was_pressed = 0;
	}
	os_timer_arm(&tickTimer, 10, 0);

	LOG_L(LOG_USER, LOG_USER_TAG,"tickCb exit\r\n");
}


void ICACHE_FLASH_ATTR websocket_callback(char * data, int size, connections *con) {
	if(size < 3 || size > 2000 || lowbatterymode) {
		LOG_E(LOG_DATA,LOG_UDP_TAG,  "Wrong size message: %p, size:%d \r\n", data, size);
		os_printf("Wrong size message: %p, size:%d \r\n", data, size);
		return;
	}
	//os_printf("websocket_callback \r\n");

	if(strncmp(data, "PING",4) != 0) {
		int ret = process_data_receive(data, data[0], con,0, size);
		if(ret == 2) {
			return; // dont affect settingstimer when adc data received
		}
		os_timer_disarm(&sendSettingsTimer);

		if(ret == 1) {
			// SUCCES PROCESSING DATA
		} else if(strncmp(data, "LEDS:", 5) == 0) {
			LOG_D(LOG_DATA, "LEDS \r\n");
			char * begin1 = &data[5];
			int lenlen = size - 5;
			if(lenlen > totleds)
				lenlen = totleds;

			os_memcpy(lTmlBuf, begin1, lenlen);
			send_add_udp(data, lenlen+5);
			send_udp_flush();
			if(current_settings[FRC_CONSOLE_ENABLE])
			hexDump("", lTmlBuf, lenlen);

			animation_custom = 1;
			current_settings[FRC_GIF_VALUE] = 0;
			animationenabled = 0;
		} 	else {
			if(con != 0) {
				os_printf("Got other data: %s", data);
				fs_process_command(data, size, con->connection);
				os_timer_arm(&sendSettingsTimer, 2000, 0);
				return;
			}
		}
	} else {
		os_printf("Received ping");
		os_timer_disarm(&sendSettingsTimer);
	}

	system_soft_wdt_feed();
	if(tcp_enabled == 0) { // if SERVER
		os_timer_arm(&sendSettingsTimer, 100, 0);
	} else {
		os_timer_arm(&sendSettingsTimer, 500, 0);
	}

	LOG_T(LOG_USER, LOG_USER_TAG, "Done receiving \r\n");
}


#define DATAPIN      12 // GPIO12
#define DATAPINBIT  BIT12

void ICACHE_FLASH_ATTR user_done() {
	wifi_settings wsettings;
	LOG_I(LOG_USER, LOG_USER_TAG, "Starting \r\n");

	system_restore();
	//
	wifi_set_opmode(STATIONAP_MODE);
	gpio_init();

	 PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12);
	 PIN_PULLUP_DIS(PERIPHS_IO_MUX_MTDI_U); // disable pullodwn
	 GPIO_REG_WRITE(GPIO_ENABLE_W1TS_ADDRESS,BIT12);
	 GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 0);  // SET
	 GPIO_DIS_OUTPUT(12);

	 PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4);
	 PIN_PULLUP_DIS(PERIPHS_IO_MUX_GPIO4_U); // disable pullodwn
	 GPIO_REG_WRITE(GPIO_ENABLE_W1TS_ADDRESS,BIT4);
	 GPIO_OUTPUT_SET(GPIO_ID_PIN(4), 0);  // SET
	 GPIO_DIS_OUTPUT(4);


	ws2812_init();
	char  dat[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	ws2812_push(dat, 6);


	spi_flash_read((ESP_PARAM_START_SEC + 1) * SPI_FLASH_SEC_SIZE, &wsettings, sizeof(wifi_settings));

	if(wsettings.isset == 0x01) {
		LOG_I(LOG_USER, LOG_USER_TAG, "Retreiving default settings %s-%s\r\n", wsettings.clientAp, wsettings.clientPass);
	} else {
		uint8_t  MAC_softAP[]          = {0,0,0,0,0,0};
		wifi_get_macaddr(SOFTAP_IF, MAC_softAP);

		 char ssid[32];
		 os_sprintf(ssid, "%s%02X%02X%02X%02X%02X%02X", "LED", MAC_softAP[0], MAC_softAP[1], MAC_softAP[2], MAC_softAP[3], MAC_softAP[4], MAC_softAP[5]);

		os_sprintf(ap_ssid, "%s", ssid);
		os_sprintf(ap_password, "LEDlines");
		os_sprintf(client_ssid, "");
		os_sprintf(client_password, "");
		writeWifiSettings(ap_ssid, ap_password, client_ssid, client_password, 0);
		system_restart();
	}

	os_sprintf(ap_ssid, wsettings.ssidAp);
	os_sprintf(ap_password, wsettings.ssidPass);
	os_sprintf(client_ssid, wsettings.clientAp);
	os_sprintf(client_password, wsettings.clientPass);
	current_settings[FRC_CONNECT_CLIENT_SAVE] = wsettings.issetclient;

	if(wsettings.issetclient == 1) {
		connectToAp2();
		connectToAp();
		wifi_set_opmode( STATIONAP_MODE );
		LOG_I(LOG_USER, LOG_USER_TAG, "SETTING STATION AP MODE!!!!!!!!!!! \r\n");
	} else {
		connectToAp();
		wifi_set_opmode( SOFTAP_MODE );
		LOG_I(LOG_USER, LOG_USER_TAG, "SETTING SOFT AP MODE!!!!!!!!!!! \r\n");
	}

	wifi_set_event_handler_cb(wifi_clb);

	server_init(websocket_callback);

	//os_memset(textbuf, 0, 30);
	settings_set_text("LED-LINES.COM", 15);

//	animation_choice = 1;
//	animationenabled = 1;
	LOG_I(LOG_USER, LOG_USER_TAG, "done\r\n");

	str = fs_readindex(BEGIN_ADDRESS_GIF);

	os_timer_disarm(&sendSettingsTimer);
	os_timer_setfn(&sendSettingsTimer, sendSettingsCb, NULL);
	os_timer_arm(&sendSettingsTimer, 1000, 0);

	os_timer_disarm(&tickTimer);
	os_timer_setfn(&tickTimer, tickCb, NULL);
	os_timer_arm(&tickTimer, 1000, 0);
	settings_driver_init();
	init_dns();
	shell_init();


}

void ICACHE_FLASH_ATTR user_init(void) {

	current_settings[FRC_BRIGHTNESS_VALUE] = 50;
	current_settings[FRC_ANIMATION_SPEED] = 10;
	current_settings[FRC_GIF_VALUE] = 0;
	current_settings[FRC_SYNC_ALL] = 1;
	current_settings[FRC_TEXT_SPEED] = 6;
	current_settings[FRC_FADE_SPEED] = 10;
	current_settings[FRC_FLICKER_SPEED] = 10;
	current_settings[FRC_SLIDESHOW_SPEED] = 10;
	current_settings[FRC_CONSOLE_ENABLE] = 0;
	current_settings[FRC_TEXT_SCROLL_MULTIPLE] = 1;
	uart_init(BIT_RATE_115200, BIT_RATE_115200);

	if(system_adc_read() < 850 && batterymonitor==1) {
		os_memset(lowbatterytext, 0, 20);
		char * txt = "Low Battery!!  ";
		os_memcpy(lowbatterytext, txt , os_strlen(txt));
		lowbatterymode = 1;
	}
	bool t = system_update_cpu_freq(SYS_CPU_160MHZ);
	os_memset(animation_random_choises, 255, 100);

	system_init_done_cb(user_done);
}

