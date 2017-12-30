#include "espmissingincludes.h"
#include "ets_sys.h"
#include "osapi.h"
#include "lwip/mem.h"
#include "spi_flash.h"
#include "fs.h"
#include "ssl/ssl_crypto.h"
#include "lwip_websocket.h"
#include "framedriver.h"

#define SRCSIZE 4096
#define BLKSIZE 65536
extern int animationenabled;

extern uint32_t current_settings[FRC_INVALID];


extern char flashing;
extern int flashingenabled;
extern char wasconnectedtoclient;
static int flash_rewriter_rom( uint32 from1, uint32 to1, uint32 size1, uint32 from2, uint32 to2, uint32 size2 )
{
	MD5_CTX md5ctx;
	char  __attribute__ ((aligned (32))) buffer[512];
	char * colons[8];
	int i, ipl = 0;
	int p;


	if( from1 == 0 || from2 == 0 || size1 == 0 )
	{
		return 2;
	}

	if( ( from1 & 0xfff ) || ( from2 & 0xfff ) || ( to1 & 0xfff ) || ( to2 & 0xfff ) )
	{
		return 3;
	}


	///////////////////////////////
	char st[400];


	//Need to round sizes up.
	size1 = ((size1-1)&(~0xfff))+1;
	if(size2!=0)
	size2 = ((size2-1)&(~0xfff))+1;

	ets_sprintf( st, "Copy 1: %08x to %08x, size %d\r\n", from1, to1, size1 );
	uart0_sendStr( st );
	ets_sprintf( st, "Copy 2: %08x to %08x, size %d\r\n", from2, to2, size2 );
	uart0_sendStr( st );

	uart0_sendStr( "go..\r\n" );
	//Disable all interrupts.
	ets_intr_lock();

	int j;


	ipl = (size1/BLKSIZE)+1;
	p = to1/BLKSIZE;
	for( i = 0; i < ipl; i++ )
	{
		SPIEraseBlock( p++ );

		for( j = 0; j < BLKSIZE/SRCSIZE; j++ )
		{
			SPIWrite( to1, (uint32_t*)(0x40200000 + from1), SRCSIZE );
			to1 += SRCSIZE;
			from1 += SRCSIZE;
		}
	}


	if(size2 != 0 && size2 > 0) {
		ipl = (size2/BLKSIZE)+1;
		p = to2/BLKSIZE;
		for( i = 0; i < ipl; i++ )
		{
			SPIEraseBlock( p++ );

			for( j = 0; j < BLKSIZE/SRCSIZE; j++ )
			{
				SPIWrite( to2, (uint32_t*)(0x40200000 + from2), SRCSIZE );
				to2 += SRCSIZE;
				from2 += SRCSIZE;
			}
		}
	}


	uart0_sendStr( "Done flashing..\r\n" );




	void(*rebootme)() = (void(*)())0x40000080;
	rebootme();
	return 0;
}


void ICACHE_FLASH_ATTR SafeMD5Update( MD5_CTX * md5ctx, uint8_t*from, uint32_t size1 )
{
	char  __attribute__ ((aligned (32))) buffer[32];

	while( size1 > 32 )
	{
		ets_memcpy( buffer, from, 32 );
		MD5Update( md5ctx, buffer, 32 );
		size1-=32;
		from+=32;
	}
	ets_memcpy( buffer, from, 32 );
	MD5Update( md5ctx, buffer, size1 );
}



size_t fs_size() { // returns the flash chip's size, in BYTES
  uint32_t id = spi_flash_get_id();
  uint8_t mfgr_id = id & 0xff;
  uint8_t type_id = (id >> 8) & 0xff; // not relevant for size calculation
  uint8_t size_id = (id >> 16) & 0xff; // lucky for us, WinBond ID's their chips as a form that lets us calculate the size
//  if(mfgr_id != 0xEF) // 0xEF is WinBond; that's all we care about (for now)
//    return 0;
  return id;
}



INDEXSTRUCT ICACHE_FLASH_ATTR fs_writeindex() {
	flashchip->chip_size = 0x01000000;

	INDEXSTRUCT str;
	str.unique[0] = '1';
	str.unique[1] = '2';
	str.unique[2] = '3';
	str.unique[3] = 0;
	str.filesactive = 1;

	os_sprintf(str.filename[0], "%s", "test.txt");
	str.offset[0] = sizeof(INDEXSTRUCT);
	str.len[0] = 4;

	spi_flash_write(BEGIN_ADDRESS_GIF, &str,  sizeof(INDEXSTRUCT));
	spi_flash_write(BEGIN_ADDRESS_GIF + str.offset[0], "TEST",  str.len[0]);
	return str;
}

INDEXSTRUCT ICACHE_FLASH_ATTR fs_readindex(uint32 address) {
	if(current_settings[FRC_CONSOLE_ENABLE])
		os_printf("fs_readindex entry\r\n");
	flashchip->chip_size = 0x01000000;

	INDEXSTRUCT str;
	char dat[20];

	SpiFlashOpResult res = spi_flash_read(address, &str, (sizeof(INDEXSTRUCT)/4)*4);
	if(current_settings[FRC_CONSOLE_ENABLE])
			os_printf("fs_readindex exit\r\n");
	return str;
}
#define PADDING 1024
#define SRCSIZE 4096
#define BLKSIZE 65536
static int f = 0;
static int j = 0;

void ICACHE_FLASH_ATTR fs_process_command(char* data, int size, struct tcp_pcb *pcb) {


		//hexDump("", data, 40);

		if(strncmp(data, "FRB:", 4) == 0) {
			os_printf("Reboot after flashing...\r\n");
			os_delay_us(100000);
			void(*rebootme)() = (void(*)())0x40000080;
			rebootme();
		} else if(strncmp(data, "FD5:", 4) == 0) {
			CopyCommand cmd;
			os_printf("FD5\r\n");

			char * begin1 = &data[4];

			char buf[size];
			int lenlen = size - 4;

			os_memcpy(buf, begin1, lenlen);
			os_memcpy(&cmd, begin1, sizeof(CopyCommand));
			//buf[lenlen] = 0;
			hexDump("", data, size);
			os_printf("from1: %02X, to:%02X, size:%d md5:%s \r\n", cmd.from1, cmd.to1, cmd.size1, cmd.md51);
			os_printf("from2: %02X, to:%02X, size:%d md5:%s \r\n", cmd.from2, cmd.to2, cmd.size2, cmd.md52);
			os_printf("FD1\r\n");

			//return;

			MD5_CTX md5ctx;
			char     md5h1raw[48];
			char     md5h1[48];
			char     md5h2[48];


			MD5Init( &md5ctx );
			int tsize = ((cmd.size1-1) / PADDING)*PADDING+PADDING;
			os_printf("MD5 SHOULD:%s - %d - %d \r\n", cmd.md51, cmd.size1, tsize);
			fs_md5_all(cmd.from1, cmd.size1 ,&md5ctx);
			//SafeMD5Update( &md5ctx, &tmpbuf[0], totalsize );
			MD5Final( md5h1raw, &md5ctx );
			for(int i = 0; i < 16; i++ )
			{
				ets_sprintf( md5h1+i*2, "%02x", md5h1raw[i] & 0xFF );
			}
			os_printf("MD5 READ  :%s \r\n", md5h1);

			if(cmd.size2 != 0) {
				MD5Init( &md5ctx );
				tsize = ((cmd.size2-1) / PADDING)*PADDING+PADDING;
				fs_md5_all(cmd.from2, cmd.size2 ,&md5ctx);
				os_printf("MD5 SHOULD:%s - %d - %d \r\n", cmd.md52, cmd.size2, tsize);

				//SafeMD5Update( &md5ctx, &tmpbuf[0], totalsize );
				MD5Final( md5h1raw, &md5ctx );
				for(int i = 0; i < 16; i++ )
				{
					ets_sprintf( md5h2+i*2, "%02x", md5h1raw[i] & 0xFF );
				}
				os_printf("MD5 READ  :%s \r\n", md5h2);
			}

			int ret = 0;
			for(int i = 0; i < 32; i++ )
			{
				if( md5h1[i] != cmd.md51[i] )
				{
					os_printf( "File 1 MD5 mismatch\r\n" );
					ret = 1;
				}
			}

			if(cmd.size2 != 0 && cmd.size2 > 0) {
				for(int i = 0; i < 32; i++ )
				{
					if( md5h2[i] != cmd.md52[i] )
					{
						os_printf( "File 2 MD5 mismatch\r\n" );
						ret = 1;
					}
				}
			}


			if(ret == 0) {
				os_printf("MD5 MATCHES, MOVE DATA \r\n");
				(*flash_global_writer)( cmd.from1, cmd.to1, cmd.size1, cmd.from2, cmd.to2, cmd.size2 );
				//websocket_write(data, pcb, WS_TEXT);
			} else {
				websocket_writedata_flush("STATUS:FAILED");
				system_restart();
			}
		} else if(strncmp(data, "FLE:", 4) == 0) {
			//os_printf("ERASE DATA \r\n");
			flashingenabled = 1;
			//os_printf("Found other characters \r\n");
			animationenabled = 0;
			current_settings[FRC_GIF_VALUE] = 0;
			flashing=1;
			if(wasconnectedtoclient) {
				wifi_station_disconnect();
			}
			//wifi_station_set_reconnect_policy(0);
			//wifi_station_set_auto_connect(false);
			frame_freegif();

			char * begin1 = &data[4];
			char * end1 = strstr(begin1, ":");
			char buf[10];
			int lenlen = end1 - begin1;
			os_memcpy(buf, begin1, lenlen);
			buf[lenlen] = 0;
			int totalsize = atoi(buf);

			char * begin2 = &end1[1];
			char * end2 = strstr(begin2, ":");
			char buf2[10];
			int lenlen2 = end2 - begin2;
			os_memcpy(buf2, begin2, lenlen2);
			buf2[lenlen2] = 0;
			int offset = atoi(buf2);
			os_printf("ERASING... %02X - %02X, %s\r\n", totalsize*0x1000, offset, buf);
			for(int i = totalsize; i <= totalsize + offset; i++) {
				system_soft_wdt_feed();
				spi_flash_erase_sector(i);
				os_printf("ERASED SECTOR... %02X \r\n",i);

			}
			os_printf("ERASED SECTORS DONE... \r\n");
			websocket_write(buf, pcb, WS_TEXT);
			os_printf("ERASING.DONE.. %02X - %02X, %s\r\n", totalsize*0x1000, offset, buf);

			system_soft_wdt_feed();
		} else if(strncmp(data, "FLB:", 4) == 0) {
			MD5_CTX md5ctx;
			char     md5h1raw[48];
			char     md5h1[48];
			//
			char * begin1 = &data[4];
			char * end1 = strstr(begin1, ":");
			char buf[10];
			int lenlen = end1 - begin1;
			os_memcpy(buf, begin1, lenlen);
			buf[lenlen] = 0;
			int totalsize = atoi(buf);
			//os_printf("BLOCKSIZE %d, %s\r\n", totalsize, buf);


			char * begin2 = &end1[1];
			char * end2 = strstr(begin2, ":");
			char buf2[10];
			int lenlen2 = end2 - begin2;
			os_memcpy(buf2, begin2, lenlen2);
			buf2[lenlen2] = 0;
			int offset = atoi(buf2);

			char * begin3 = &end2[1];
			char * end3 = strstr(begin3, ":");
			char buf3[10];
			int lenlen3 = end3 - begin3;
			os_memcpy(buf3, begin3, lenlen3);
			buf3[lenlen3] = 0;
			int p = atoi(buf3);
			if(offset == 0)
			os_printf("Flashing data to address: %p\r\n", p);

			//os_printf("ADDRESS %02X, %s\r\n", p, buf3);
			char databuffer[totalsize+1];
			char databufferraw[totalsize+1];

			os_memcpy(databuffer, end3+1, totalsize);
			databuffer[totalsize] = 0;
			system_soft_wdt_feed();
			if(end3 != 0 && totalsize > 0) {
					char tmpbuf2[totalsize+5];
					//hexDump2("WRITE:",  end3+1, 10, offset);
					flashchip->chip_size = 0x01000000;
					int add = p + offset;
					add = (add / 4) * 4;
					SpiFlashOpResult res = spi_flash_write(add, databuffer, totalsize);
					//if(console_output)
					os_printf("Wrote to address: %p - bytes: %d\r\n",add, totalsize);
					//hexDump("WRITE DATA=:",  databuffer , totalsize);
					MD5Init( &md5ctx );
					char tmpbuf[totalsize];
					//os_printf("Going to read from:0x%02X to:0x%02X, size:%d \r\n", add, add + totalsize, totalsize);
					res = spi_flash_read(add, tmpbuf, totalsize);
					//os_printf("Read from address: %p - bytes: %d\r\n",add, totalsize);
					SafeMD5Update( &md5ctx, tmpbuf, totalsize );
					//SafeMD5Update( &md5ctx, databuffer, totalsize );
					MD5Final( md5h1raw, &md5ctx );
					for(int i = 0; i < 16; i++ )
					{
						ets_sprintf( md5h1+i*2, "%02x", md5h1raw[i] & 0xFF);
					}
					//hexDump("READ DATA=:",  tmpbuf , totalsize);
					//hexDump("MD5 READ: ",  md5h1, 48);
					//if(console_output)
					os_printf("MD5 READ:%s\r\n", md5h1);
					os_sprintf(tmpbuf2, "MD5:%s",md5h1);
					os_printf("MD5 READ:%s\r\n", tmpbuf2);
					int ret = websocket_write_size(tmpbuf2, pcb, os_strlen(tmpbuf2), WS_TEXT);
					if(ret != 0) {
						os_printf("Error sending md5 \r\n");
					}
					ret = tcp_output(pcb);
					if(ret != 0) {
						os_printf("Error flushing md5 \r\n");
					}

			}
			system_soft_wdt_feed();

		} else {
//			/websocket_write("FX523", pcb, WS_TEXT);
		}
}


#define SRCSIZE 4096
#define BLKSIZE 65536


int32 ICACHE_FLASH_ATTR my_atoi( const char * in )
{
	int positive = 1; //1 if negative.
	int hit = 0;
	int val = 0;
	while( *in && hit < 11 	)
	{
		if( *in == '-' )
		{
			if( positive == -1 ) return val*positive;
			positive = -1;
		} else if( *in >= '0' && *in <= '9' )
		{
			val *= 10;
			val += *in - '0';
			hit++;
		} else if (!hit && ( *in == ' ' || *in == '\t' ) )
		{
			//okay
		} else
		{
			//bad.
			return val*positive;
		}
		in++;
	}
	return val*positive;
}



#define BLKSIZE 65536
void ICACHE_FLASH_ATTR fs_copy_all(uint32 from, uint32 to, uint32 length) {
	ets_wdt_disable();
	int f = 0;
	int p = 0;
	int w = 0;
	int togo = length;
	os_printf("Starting transfer of: %d bytes...", togo);

	int ipl = (length/0x1000)+1;
	for(f = 0; f < ipl; f++ )
	{
		os_printf("[%d]new  writing block \r\n", 2);
		os_printf("erasing block :%d size: %d \r\n", p, BLKSIZE);
		spi_flash_erase_sector( p++ );
		system_soft_wdt_feed();

	}
	int read = 1024;
	char buff[1024];
	p = 0;
	for(f = 0; f < length/1024; f++ )
	{
		system_soft_wdt_feed();
		os_printf("Going to read from:0x%02X to:0x%02X, size:%d \r\n", from+p, from +p+ read, read);
		int res = spi_flash_read(from+p,buff, read);
		//os_printf()
		//SPIWrite( to2, (uint32_t*)(0x40200000 + from2), SRCSIZE );
		os_printf("Going to write from:0x%02X to:0x%02X, size:%d \r\n", from+p, to+p, read);
		spi_flash_write(to+p, buff, read);
		p+= 1024;
	}
	os_printf("Done with: %d bytes...", p);
	system_soft_wdt_feed();
}


void ICACHE_FLASH_ATTR fs_md5_all(uint32 add, uint32 length, MD5_CTX * md5ctx) {
	int p = 0;
	int togo = length;
	while(togo > 0) {
		int read = togo;
		if(read > 1024) {
			read = 1024;
		}
		char buff[read];
		system_soft_wdt_feed();
		//os_printf("Going to read from:0x%02X to:0x%02X, size:%d \r\n", add+p, add +p+ read, read);
		int res = spi_flash_read(add+p,buff, read);
		MD5Update( md5ctx, buff, read );

		p+= read;
		togo-=read;
	}
}
int ICACHE_FLASH_ATTR fs_findfile_name(uint32* address, char * name) {
	flashchip->chip_size = 0x01000000;

		INDEXSTRUCT str;
	//	for(int i = 0; i < 20; i++) {
	//		SpiFlashOpResult res = spi_flash_read(BEGIN_ADDRESS+i, &dat[0], 1);
	//		os_printf("%02X \r\n", dat[0]);
	//	}
		if(current_settings[FRC_CONSOLE_ENABLE])
		os_printf("Find file: %s \r\n", name);
		SpiFlashOpResult res = spi_flash_read(address, &str, (sizeof(INDEXSTRUCT)/4)*4);
		//os_printf("%d \r\n", res);
		//hexDump("", &str, sizeof(INDEXSTRUCT));
		if(current_settings[FRC_CONSOLE_ENABLE])
		os_printf("Find file 2: %s \r\n", name);


		for(int i = 0; i < str.filesactive; i++) {
			if(strncmp(str.filename[i], name, os_strlen(name)) == 0) {
				return i;
			}
		}
		return -1;
}

void* ICACHE_FLASH_ATTR fs_readfile_max(uint32 address, char * buf, int i, int len, int offset) {
	flashchip->chip_size = 0x01000000;


	INDEXSTRUCT str;
//	if((address & 0xF) == 0) {
//		address += 4;
//	}

	//os_printf("Read \r\n");
	SpiFlashOpResult res = spi_flash_read(address, &str, sizeof(INDEXSTRUCT));

	//os_printf("%s - %d - %d\r\n", str.filename[i], str.len[i],str.offset[i]);
	system_soft_wdt_restart();
	system_soft_wdt_feed();
	int read = len;
	if(read > 10000) {
		read = 10000;
	}
	if(read > str.len[i])
		read= str.len[i];

	address = address + sizeof(INDEXSTRUCT) + str.offset[i] + offset;
//	if((address & 0xF) == 0) {
//		address += 4;
//	}
	res = spi_flash_read(address, (uint32 *) buf, read);

	//hexDump("", buf, read);
	system_soft_wdt_restart();
	//os_printf("Done\r\n");

	//hexDump( str.filename[i], buf, 100);
	return buf;
}

void* ICACHE_FLASH_ATTR fs_readfile(uint32 address, char * buf, int i) {
	flashchip->chip_size = 0x01000000;


	INDEXSTRUCT str;
	//os_printf("Read \r\n");
	SpiFlashOpResult res = spi_flash_read(address, &str, sizeof(INDEXSTRUCT));
	if(current_settings[FRC_CONSOLE_ENABLE])
	os_printf("1 %s - %d - %d\r\n", str.filename[i], str.len[i], str.offset[i]);
	system_soft_wdt_restart();
	system_soft_wdt_feed();
	int read = str.len[i];
	if(current_settings[FRC_CONSOLE_ENABLE])
	os_printf("2 %s - %d - %d\r\n", str.filename[i], str.len[i], str.offset[i]);

	if(read > 10000) {
		read = 10000;
		os_printf("2.2 %s - %d - %d\r\n", str.filename[i], str.len[i], str.offset[i]);
	}
	res = spi_flash_read(address + sizeof(INDEXSTRUCT) + str.offset[i], (uint32 *) buf, read);
	if(current_settings[FRC_CONSOLE_ENABLE])
	os_printf("3 %s - %d - %d\r\n", str.filename[i], str.len[i], str.offset[i]);

	if(read != str.len[i]) {
		int newread = str.len[i] - read;
		if(newread > 10000) {
			newread = 10000;
		}
		res = spi_flash_read(address + sizeof(INDEXSTRUCT) + str.offset[i] + read, (uint32 *) buf, newread);

		if(newread + read != str.len[i]) {
			int newread2 = str.len[i] - read - newread;
			if(newread2 > 10000) {
				newread2 = 10000;
			}
			res = spi_flash_read(address + sizeof(INDEXSTRUCT) + str.offset[i] + newread + read, (uint32 *) buf, newread2);
		}
	}
	system_soft_wdt_restart();
	if(current_settings[FRC_CONSOLE_ENABLE])
	os_printf("Done reading\r\n");

	//if(console_output)
	//hexDump( str.filename[i], buf, 100);

	return buf;
}

int (*flash_global_writer)( uint32 from1, uint32 to1, uint32 size1, uint32 from2, uint32 to2, uint32 size2 ) = flash_rewriter_rom;

