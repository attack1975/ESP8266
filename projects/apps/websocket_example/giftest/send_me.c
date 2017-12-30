#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <libwebsockets.h>
#include "md5.h"

#define KGRN "\033[0;32;32m"
#define KCYN "\033[0;36m"
#define KRED "\033[0;32;31m"
#define KYEL "\033[1;33m"
#define KBLU "\033[0;32;34m"
#define KCYN_L "\033[1;36m"
#define KBRN "\033[0;33m"
#define RESET "\033[0m"

static int destroy_flag = 0;
static int connection_flag = 0;
static int writeable_flag = 0;
char * addresss;
char * file;
	
	int readLock = 0;
	
#define BLOCK_SIZE 65536
#define SECTOR_SIZE 4096
#define PADDING 1024
	

void ComputeMD5WithKey( char * md5retText, FILE * f, const char * key)
{
	uint8_t retmd5[16];
	MD5_CTX ctx;
	//FILE * f = fopen( filename, "rb" );
	if( !f )
	{
		//fprintf( stderr, "Error opening %s\n", filename );
		exit( -9 );
	}

	fseek( f, 0, SEEK_END );
	int l = ftell( f );
printf("MD5 Size: %d\n", l );
	int padl = ((l-1) / PADDING)*PADDING+PADDING;
printf("MD5 Pad: %d\n", padl );
	fseek( f, 0, SEEK_SET );
	uint8_t data[padl];
	fread( data, l, 1, f );
	fclose( f );

	memset( data+l, 0, padl-l );
	MD5_Init( &ctx );

	MD5_Update( &ctx, data, padl );
	MD5_Final( retmd5, &ctx );

	for( l = 0; l < 16; l++ )
	{
		sprintf( md5retText + l*2, "%02x", retmd5[l] & 0xFF );
	}

	return;
}



static void INT_HANDLER(int signo) {
    destroy_flag = 1;
}

struct session_data {
    int fd;
};

struct pthread_routine_tool {
    struct lws_context *context;
    struct lws *wsi;
};

static int websocket_write_back(struct lws *wsi_in, char *str, int str_size_in) 
{
    if (str == NULL || wsi_in == NULL)
        return -1;

    int n;
    int len;
    char *out = NULL;

    if (str_size_in < 1) 
        len = strlen(str);
    else
        len = str_size_in;

    out = (char *)malloc(sizeof(char)*(LWS_SEND_BUFFER_PRE_PADDING + len + LWS_SEND_BUFFER_POST_PADDING));
    //* setup the buffer*/
    memcpy (out + LWS_SEND_BUFFER_PRE_PADDING, str, len );
    //* write out*/
  
    n = lws_write(wsi_in, out + LWS_SEND_BUFFER_PRE_PADDING, len, LWS_WRITE_TEXT);
    //while (!lws_send_pipe_choked(wsi_in));  // wait for ready
    
   // printf(KBLU"[websocket_write_back] %s\n"RESET, str);
    //* free the buffer*/
    free(out);

    return n;
}


static int ws_service_callback(
                         struct lws *wsi,
                         enum lws_callback_reasons reason, void *user,
                         void *in, size_t len)
{

    switch (reason) {

        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            printf(KYEL"[Main Service] Connect with server success.\n"RESET);
            connection_flag = 1;
            break;

        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
            printf(KRED"[Main Service] Connect with server error.\n"RESET);
            destroy_flag = 1;
            connection_flag = 0;
            break;

        case LWS_CALLBACK_CLOSED:
            printf(KYEL"[Main Service] LWS_CALLBACK_CLOSED\n"RESET);
            destroy_flag = 1;
            connection_flag = 0;
            break;

        case LWS_CALLBACK_CLIENT_RECEIVE:
            printf(KCYN_L"MD5: %s\n\n"RESET, (char *)in);
            readLock = 1;
            if (writeable_flag)
                destroy_flag = 1;

            break;
        case LWS_CALLBACK_CLIENT_WRITEABLE :
            printf(KYEL"[Main Service] On writeable is called. send byebye message\n"RESET);
            //websocket_write_back(wsi, "Byebye! See you later", -1);
            writeable_flag = 1;
            break;

        default:
            break;
    }

    return 0;
}

static void *pthread_routine(void *tool_in)
{
    struct pthread_routine_tool *tool = tool_in;

    printf(KBRN"[pthread_routine] Good day. This is pthread_routine.\n"RESET);

    //* waiting for connection with server done.*/
    while(!connection_flag)
        usleep(1000*20);

    //*Send greeting to server*/
    printf(KBRN"[pthread_routine] Server is ready. send a greeting message to server.\n"RESET); 
    FILE * f = fopen( file, "r" );
	
	if( !f || ferror( f ) )
	{
		fprintf( stderr, "Error: cannot open \"%s\" for writing.\n", "test" );
		return;
	}
	
	fseek(f, 0L, SEEK_END);
    int sz = ftell(f);
    sz = ((sz-1) / PADDING)*PADDING+PADDING;
    fseek(f, 0L, SEEK_SET);
    printf("SIZE: %d \r\n", sz);
    char totalbuffer[2000];
    char buf[2000];
    
    
    
    int address =   strtol(&addresss[2], NULL, 16);
      for(int i = 0; i < (sz / 0x1000) + 1; i++) {
        int len = sprintf(buf, "FLE:%d:", (address / 0x1000) + i); 
        printf("ERASE message: %d-%s \r\n", len); 
        websocket_write_back(tool->wsi, buf, len);
        readLock = 0;
	     while(readLock == 0);
     }

    long datapointer = 0;
    int res = -1;
    
//            lws_callback_on_writable(tool->wsi);

    
    
   for(int i = 0; i < (sz / 1024); i++) {
         MD5_CTX ctx;
        memset(buf, 0x00, 2000);
        res = fread( totalbuffer, 1, 1024, f );
        //if(res == 0)
       // break;
        int len = sprintf(&buf[0], "FLB:%d:%d:%d:", 1024, datapointer, address); 
        memcpy(&buf[len], totalbuffer, res);
        //printf("DATA message: %d \r\n", res); 
       
        char retmd5[48];
        char md5[res];
                memset(md5, 0, res);
               memset(retmd5, 0, 48);
      
	    MD5_Init( &ctx );
	    MD5_Update( &ctx, &buf[len], 1024 );
	    MD5_Final( retmd5, &ctx );

	    for(int l = 0; l < 16; l++ )
	    {
		    sprintf( md5 + l*2, "%02x", retmd5[l] & 0xFF );
		         	  //  printf("MD5: $\r\n", retmd5[l]);       
	    }
	    printf("MD5: %s \r\n", md5);
	    
	    
	     websocket_write_back(tool->wsi, buf, len+1024);
	     readLock = 0;
	     while(readLock == 0);
         datapointer += 1024;
     }
     
 
  
    fseek(f, 0L, SEEK_SET);
   
    char md5_f1[48];
    
    ComputeMD5WithKey( md5_f1, f, "" );
    int len = sprintf(buf, "FL5:%d:%d:%s", datapointer, address, md5_f1); 
    printf("send message: %d-%s \r\n", len, buf);
    websocket_write_back(tool->wsi, buf, len);
       
        
  //  websocket_write_back(tool->wsi, buf, len);
    
    	fclose( f );
  
  
   // websocket_write_back(tool->wsi, buf, len);

    printf(KBRN"[pthread_routine] sleep 2 seconds then call onWritable\n"RESET);
  //  sleep(1);
    printf(KBRN"------------------------------------------------------\n"RESET);
   // sleep(1);
    //printf(KBRN"[pthread_routine] sleep 2 seconds then call onWritable\n"RESET);

    //*involked wriable*/
    printf(KBRN"[pthread_routine] call on writable.\n"RESET);   
    

}

int main(int argc, char**argv)
{
    //* register the signal SIGINT handler */
    struct sigaction act;
    act.sa_handler = INT_HANDLER;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    sigaction( SIGINT, &act, 0);


    struct lws_context *context = NULL;
    struct lws_context_creation_info info;
    struct lws *wsi = NULL;
    struct lws_protocols protocol;

    memset(&info, 0, sizeof info);
    info.port = CONTEXT_PORT_NO_LISTEN;
    info.iface = NULL;
    info.protocols = &protocol;
    info.ssl_cert_filepath = NULL;
    info.ssl_private_key_filepath = NULL;
    info.extensions = lws_get_internal_extensions();
    info.gid = -1;
    info.uid = -1;
    info.options = 0;

    protocol.name  = "my-echo-protocol";
    protocol.callback = &ws_service_callback;
    protocol.per_session_data_size = sizeof(struct session_data);
    protocol.rx_buffer_size = 0;
    protocol.id = 0;
    protocol.user = NULL;

    if (argc < 3 )
	{
		printf("usage: pushtodev [file_lower] [file_upper] [key (optional)]\n");
		exit(-1);
	}
	
    context = lws_create_context(&info);
    printf(KRED"[Main] context created.\n"RESET);

    if (context == NULL) {
        printf(KRED"[Main] context is NULL.\n"RESET);
        return -1;
    }


    wsi = lws_client_connect(context, "192.168.178.15", 9000, 0,
            "/echo", "192.168.178.15:9000", NULL,
             protocol.name, -1);
    if (wsi == NULL) {
        printf(KRED"[Main] wsi create error.\n"RESET);
        return -1;
    }




	addresss = argv[1];
	file = argv[2];
	printf("SDFSDFSDF %s", addresss);
	
    printf(KGRN"[Main] wsi create success.\n"RESET);

    struct pthread_routine_tool tool;
    tool.wsi = wsi;
    tool.context = context;

    
	
    pthread_t pid;
    pthread_create(&pid, NULL, pthread_routine, &tool);
    pthread_detach(pid);

    while(!destroy_flag)
    {
        lws_service(context, 50);
    }

    lws_context_destroy(context);

    return 0;
}
