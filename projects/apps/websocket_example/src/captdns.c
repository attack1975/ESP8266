#include "c_types.h"
#include "user_interface.h"
#include "user_config.h"
#include "espconn.h"
#include "mem.h"
#include "osapi.h"
#include "log.h"

//Listening connection data
static struct espconn dnsConn;
static esp_udp dnsUdp;
#define INTERFACE_DOMAIN "digit"
static const char *localDomains[]={
    INTERFACE_DOMAIN, //because I can be anybody
	"led",
	"LED",
	"ledlines",
	"LEDlines",
	"LED-lines",
	"LED-lines.com",
	"LEDlines.com",
	"led.com",
	"www.led.com",
	"www.LED.com",
	"www.LEDlines.com",
	"www.LED-lines.com",
    "LED.com", //android , yes I can be Google too
	//"www.google.com",
    0
};

static int ICACHE_FLASH_ATTR isKnownDNS(char *dns){

    int i=0;
    while(1){

        const char *cmp = localDomains[i];

        if(cmp==0)
            break;

        if(strstr(dns,cmp))
            return 1;

        i++;
    }

    return 0;
}

static void ICACHE_FLASH_ATTR dnsQueryReceived(void *arg, char *data, unsigned short length) {


// parse incoming query domain
    char domain[30];
    char *writePos=domain;
    memset(domain,0,30);

    int offSet=12;
    int len=data[offSet];
    while(len!=0 && offSet<length){

        offSet++;
        memcpy(writePos,data+offSet,len);
        writePos+=len; //advance
        offSet+=len;
        len=data[offSet];

        if(len!=0){
            *writePos='.';
            writePos++;
        }

    }

    if(!isKnownDNS(domain))
       return;

    LOG_I(LOG_UDP, "DNS", "Query Approved: %s  \r\n", domain);


    struct espconn *conn=arg;
  //build response
    char response[100] = {data[0], data[1],
                0b10000100 | (0b00000001 & data[2]), //response, authorative answer, not truncated, copy the recursion bit
                0b00000000, //no recursion available, no errors
                data[4], data[5], //Question count
                data[4], data[5], //answer count
                0x00, 0x00,       //NS record count
                0x00, 0x00};      //Resource record count

    int idx = 12;
    memcpy(response+12, data+12, length-12); //Copy the rest of the query section
    idx += length-12;

    //Set a pointer to the domain name in the question section
    response[idx] = 0xC0;
    response[idx+1] = 0x0C;

    //Set the type to "Host Address"
    response[idx+2] = 0x00;
    response[idx+3] = 0x01;

    //Set the response class to IN
    response[idx+4] = 0x00;
    response[idx+5] = 0x01;

    //A 32 bit integer specifying TTL in seconds, 0 means no caching
    response[idx+6] = 0x00;
    response[idx+7] = 0x00;
    response[idx+8] = 0x00;
    response[idx+9] = 0x00;

    //RDATA length
    response[idx+10] = 0x00;
    response[idx+11] = 0x04; //4 byte IP address

    //The IP address
    response[idx + 12] = 192;
    response[idx + 13] = 168;
    response[idx + 14] = 4;
    response[idx + 15] = 1;

    remot_info *remInfo=NULL;
    	//Send data to port/ip it came from, not to the ip/port we listen on.
    	if (espconn_get_connection_info(conn, &remInfo, 0)==ESPCONN_OK) {
    		conn->proto.udp->remote_port=remInfo->remote_port;
    		memcpy(conn->proto.udp->remote_ip, remInfo->remote_ip, sizeof(remInfo->remote_ip));
    	}

    int ret = espconn_sendto(conn, (uint8_t*)response, idx+16);
    uint8_t *ip = conn->proto.udp->remote_ip;

    LOG_I(LOG_UDP, "DNS", "UDP send res : %d ip: %d.%d.%d.%d , port: %d \r\n",ret,ip[0],ip[1],ip[2],ip[3],conn->proto.udp->remote_port);

}

 void ICACHE_FLASH_ATTR init_dns() {
	
    uint8_t mode = 1;
   // wifi_softap_set_dhcps_offer_option(OFFER_ROUTER, &mode);
   // wifi_set_broadcast_if(3);
    //espconn_disconnect(&dnsConn);
    espconn_delete(&dnsConn);

	dnsConn.type=ESPCONN_UDP;
	dnsConn.state=ESPCONN_NONE;
	dnsUdp.local_port=(int)53;
	dnsConn.proto.udp=&dnsUdp;
	
    espconn_regist_recvcb(&dnsConn, dnsQueryReceived);

	int res = espconn_create(&dnsConn);

	os_printf("DNS server init, conn=%p , status=%d", &dnsConn,res);

}
