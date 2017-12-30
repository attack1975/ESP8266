/*
 * log.h
 *
 *  Created on: Oct 11, 2015
 *      Author: wouters
 */

#ifndef SRC_LOG_H_
#define SRC_LOG_H_

#define LOG_ON 0x01
#define LOG_OFF 0x00

#define LOG_DATA_TAG "DATA1"
#define LOG_HTTP_TAG "HTTP"

#define LOG_LOW 0x00
#define LOG_LOW_TAG "LOW"
#define LOG_TRACE 0x01
#define LOG_TRACE_TAG "TRACE"
#define LOG_INFO 0x02
#define LOG_INFO_TAG "INFO"
#define LOG_WARNING 0x03
#define LOG_WARNING_TAG "WARNING"
#define LOG_ERROR 0x04
#define LOG_ERROR_TAG "ERROR"

#define LOG_DATA LOG_ON
#define LOG_UDP LOG_ON
#define LOG_HTTPSERVER LOG_ON
#define LOG_USER LOG_ON
#define LOG_FRAMEDRIVER LOG_ON
#define LOG_ENABLE LOG_ON

#define LOG_UDP_TAG "UDP"
#define LOG_USER_TAG "USER"
#define LOG_FRAMEDRIVER_TAG "FRAMEDRIVER"

#include "lwip_websocket.h"
#include "framedriver.h"


#define LOG_LEVEL LOG_INFO
#define LOG_LEVEL_CONNECTED LOG_INFO
#define LOG_LEVEL_TCP LOG_ERROR

#define LOG_UDP_OUTPUT LOG_OFF

extern char flashing;
extern int tcp_enabled;
//extern char console_output;

extern uint32_t current_settings[FRC_INVALID];
#define LOG(debug, tag, message, udp, ...) do { \
		 char data[100];  			\
		 char data2[100];  			\
		\
		char * t;					\
		if(debug == LOG_TRACE) {	\
			t = LOG_TRACE_TAG;		\
		}							\
									\
		if(debug == LOG_INFO) {		\
			t = LOG_INFO_TAG;		\
		}							\
									\
		if(debug == LOG_WARNING) {	\
			t = LOG_WARNING_TAG;	\
		}							\
									\
		if(debug == LOG_ERROR) {	\
			t = LOG_ERROR_TAG;		\
		}							\
									\
		if((debug >= LOG_LEVEL) && (LOG_ENABLE & LOG_ON)) { \
			os_memset(data, 0, 100); \
			os_memset(data2, 0, 100); \
			os_sprintf(data, "[%s][%s][%s]\t", tag, t,  __func__); \
			if(os_strlen(__func__) < 18) { \
				os_sprintf(data, "%s\t", data); \
			} \
			os_sprintf(&data[os_strlen(data)], message, ##__VA_ARGS__);  \
			if(tcp_enabled == 0 || debug >= LOG_LEVEL_CONNECTED) \
			os_printf(data);  \
		}\
		\
} while (0) /*
			os_sprintf(data2, "CONSOLE: %s", data); \
			if(LOG_UDP_OUTPUT == LOG_ON && current_settings[FRC_CONSOLE_ENABLE] == 1 && udp == 1 && flashing == 0 && (tcp_enabled != 1 || current_settings[FRC_SYNC_ALL] == 0 || debug >= LOG_LEVEL_TCP)) { \
				websocket_writedata_size(data2, os_strlen(data2), WS_TEXT, 1); \
			} \
		}\
		\
} while (0)
*/

struct client_str {
	ip_addr_t ip_addr;
};

#define LOG_W(debug, tag, message, ...) do { \
	if(debug == LOG_ON) { \
	LOG(LOG_WARNING, tag, message, 1, ##__VA_ARGS__); \
	} \
}while(0)

#define LOG_T(debug, tag, message, ...) do { \
	if(debug == LOG_ON) { \
	LOG(LOG_TRACE, tag, message, 1, ##__VA_ARGS__); \
	} \
}while(0)

#define LOG_H(debug, tag, message, ...) do { \
	if(debug == LOG_ON) { \
	LOG(LOG_TRACE, tag, message, 0, ##__VA_ARGS__); \
	} \
}while(0)

#define LOG_L(debug, tag, message, ...) do { \
	if(debug == LOG_ON) { \
	LOG(LOG_LOW, tag, message, 0, ##__VA_ARGS__); \
	} \
}while(0)

#define LOG_E(debug, tag, message, ...) do { \
	if(debug == LOG_ON) { \
	LOG(LOG_ERROR, tag, message, 1, ##__VA_ARGS__); \
	} \
}while(0)

#define LOG_D(debug, message, ...) do { \
	if(debug == LOG_ON) { \
		LOG(LOG_INFO, LOG_DATA_TAG, message, 1, ##__VA_ARGS__); \
	} \
}while(0)


#define LOG_I(debug, tag, message, ...) do { \
	if(debug == LOG_ON) { \
		LOG(LOG_INFO, tag, message, 1, ##__VA_ARGS__); \
	} \
}while(0)

#endif /* SRC_LOG_H_ */
