/*
 * testDisplay.c - minimal test of the i2c_oled functions
 *
 * Uses all of the i2c_oled functions.
 *
 * Copyright 2015 Jerry Dunmire
 * jedunmire+i2c_oled AT gmail
 * This file is part of i2c_oled
 *
 * i2c_oled is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * i2c_oled is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with i2c_oled.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Files that are not part of i2c_oled are clearly identified at the top
 * of each of the files. These files are distributed under terms noted in each
 * of the files.
 */
#include "espmissingincludes.h"
#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <mem.h>
#include "driver/uart.h"
#include "driver/i2c_oled.h"
#include "driver/i2c.h"
#include "driver/spi.h"
#include "driver/diskio.h"
#include "user_config.h"
#include "bitmap.h"
#include "driver/ff.h"
#include "gif.h"

static ETSTimer tickTimer;
FATFS FATFS_Obj;

char buffer[128];
char buffer2[128];
char buffer3[128];
char buffer4[128];
uint8 framecount = 0;

char buf2[512];

static volatile os_timer_t sd_timer;

//Timer event.
static void ICACHE_FLASH_ATTR
mysdTimer(void *arg) {
	//100Hz timer
	disk_timerproc();
	os_timer_setfn(&sd_timer, (os_timer_func_t *) mysdTimer, NULL);
	os_timer_arm(&sd_timer, 10, 0);
}

Gif * g;

void readGif() {
	int res = 0;
		FIL f;
		res = f_open (
			&f,	"/GIF_BEER.GIF", FA_READ
		);
		if (res == FR_OK)
				os_printf("\n\r Open file  OK \n\r");
			else
				os_printf("Open file ERROR %d \n\r", (int) res);


		int len = 0;
		os_printf("\n\r Read file  OK: %d \n\r", f.fsize);
		int read = f.fsize;
		if(f.fsize > 512) {
			read = 512;
		}
		res = f_read (&f, buf2, read, &len);

		if (res == FR_OK)
				os_printf("\n\r Read file  OK \n\r");
			else
				os_printf("Read file ERROR %d \n\r", (int) res);


		if(g != 0)
			del_gif(g);

		g = read_gif_file(buf2, len);
}
void tickCb() {

	os_timer_disarm(&tickTimer);

	//

//
////	char buf[512];
////	os_memset(buf, 0x00, 512);
//
//	//sd_init();
//	int i = disk_status(0);//(0, buf2, 0, 1);
//	if(i == 0) {
//		os_printf("DATA\r\n");
//		//disk_read(0, buf, 0, 1);
//		os_printf("dat 1 %d  \r\n", buf2[0]);
//		os_printf("dat 2 %d  \r\n", buf2[1]);
//		os_printf("dat 3 %d  \r\n", buf2[2]);
//		os_printf("dat 4 %d  \r\n", buf2[3]);
//		os_printf("dat 5 %d  \r\n", buf2[4]);
//		os_printf("dat 6 %d  \r\n", buf2[5]);
//		os_printf("dat 7 %d  \r\n", buf2[6]);
//		os_printf("dat 8 %d  \r\n", buf2[7]);
//		os_printf("dat 9 %d  \r\n", buf2[8]);
//
//	}
//
//		//sd_readstatus();
//		// read_page(0x0004, buffer2, 4);
//		os_sprintf(buffer3, "Line %d  ", buf2[0]);
////
//		OLED_Print(9, 0, buffer3, 1);
//		os_sprintf(buffer4, "Line %d  ", buf2[1]);
//		OLED_Print(9, 1, buffer4, 1);
//		os_sprintf(buffer4, "Line %d  ", buf2[2]);
//		OLED_Print(9, 2, buffer4, 1);
//		os_sprintf(buffer4, "Line %d  ", buf2[3]);
//		OLED_Print(9, 3, buffer4, 1);

//		count++;
//		if(count > 205) {
//			count = 0;
//		}
//		buffer[0] = count;
//		buffer[1] = count + 10;
//		buffer[2] = count + 20;
//		buffer[3] = count + 30;
//		buffer[4] = count + 40;
//
//		write_page(0x0004, buffer, 6);


//	//    OLED_CLS();
	//
	////    OLED_Print(0, 1, "Test 2 - ", 1);
	////    for (line = 0; line < 4; line++)
	////    {
	////        os_sprintf(buffer, "Line %d", line);
	////        OLED_Print(8, line, buffer, 2);
	////    }
	////    os_delay_us(10000);
	////    OLED_CLS();
	////
	////    OLED_Print(0, 0, "Test 3 - wrap", 1);
	////    OLED_Print(0, 2, "123456789012345678901234567890", 1);
	////    OLED_Print(0, 2, "12345678901234567890", 2);
	////
	////    os_delay_us(1000);

//	    OLED_CLS();
//
//	    // Test #4, bit map
//	    OLED_DrawBMP(0, 0, BITMAP_COLUMNS , BITMAP_ROWS, bitmap);
//	//
//	    os_delay_us(10000);
//

    os_timer_arm(&tickTimer, 1000, 0);


}



// http://elm-chan.org/fsw/ff/en/readdir.html
FRESULT ICACHE_FLASH_ATTR scan_files (
    char* path        /* Start node to be scanned (also used as work area) */
)
{
    FRESULT res;
    FILINFO fno;
    DIR dir;
    int i;
    char *fn;   /* This function assumes non-Unicode configuration */
#if _USE_LFN
    static char lfn[_MAX_LFN + 1];   /* Buffer to store the LFN */
    fno.lfname = lfn;
    fno.lfsize = sizeof lfn;
#endif


    res = f_opendir(&dir, path);                       /* Open the directory */
    if (res == FR_OK) {
        i = strlen(path);
        for (;;) {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            if (fno.fname[0] == '.') continue;             /* Ignore dot entry */
#if _USE_LFN
            fn = *fno.lfname ? fno.lfname : fno.fname;
#else
            fn = fno.fname;
#endif
            if (fno.fattrib & AM_DIR) {                    /* It is a directory */
                os_sprintf(&path[i], "/%s", fn);
                res = scan_files(path);
                path[i] = 0;
                if (res != FR_OK) break;
            } else {                                       /* It is a file. */
                os_printf("%s/%s\n", path, fn);
            }
        }
        f_closedir(&dir);
    }

    return res;
}


//Init function
void ICACHE_FLASH_ATTR
user_init()
{
 g = 0;
	os_memset(buf2, 0xFF, 512);
//
	gpio_init();
//	 GPIO_OUTPUT_SET(GPIO_ID_PIN(5), 1);  // SET
//	 GPIO_OUTPUT_SET(GPIO_ID_PIN(5), 0);  // CLEAR

	os_printf("Starting \r\n");
	os_timer_disarm(&tickTimer);
	os_timer_setfn(&tickTimer, tickCb, NULL);
	os_timer_arm(&tickTimer, 1000, 0);

	os_timer_disarm(&sd_timer);
	os_timer_setfn(&sd_timer, (os_timer_func_t *) mysdTimer, NULL);
	os_timer_arm(&sd_timer, 10, 0);

	//spi_init();

	//SPI_CS_SETUP
	 PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO15);
	 PIN_PULLUP_DIS(PERIPHS_IO_MUX_MTDO_U); // disable pullodwn
	 GPIO_REG_WRITE(GPIO_ENABLE_W1TS_ADDRESS,BIT15);
	 GPIO_OUTPUT_SET(GPIO_ID_PIN(15), 1);  // CLEAR


	uart_init(BIT_RATE_115200, BIT_RATE_115200);
	//system_soft_wdt_stop();


		os_printf("START SYSTEM \n\r");
		spi_init();
		os_printf("INIT SPI OK  \n\r");

		DSTATUS stinit = disk_initialize(0);

		if (stinit) {
			if (stinit == RES_ERROR) {
				os_printf("R/W ERROR \n\r");
				return;
			}
			if (stinit == RES_WRPRT) {
				os_printf("Write Protected \n\r");
				return;
			}
			if (stinit == RES_NOTRDY) {
				os_printf("Not Ready \n\r");
				return;
			}
			if (stinit == RES_PARERR) {
				os_printf("Invalid Parameter \n\r");
				return;
			}
		}
		os_printf("SD interface init OK \n\r");

		int i = 1;
		FRESULT res;

		while(i != 0) {
			i = disk_status(0);
		}

		res = f_mount(&FATFS_Obj, "", 0);
		if (res == FR_OK)
			os_printf("Mounted OK \n\r");
		else
			os_printf("Fat mount ERROR %d \n\r", (int) res);



		res = scan_files("");
		if (res == FR_OK)
			os_printf("\n\r Scan files  OK \n\r");
		else
			os_printf("Scan files ERROR %d \n\r", (int) res);



//		os_printf("framecount: %d, bcount: %d width: %d, height: %d, p: %p \r\n", framecount, g->block_count, g->screen->width, g->screen->height, g->blocks[0]);

//		buf2[0] = 1;
//		buf2[1] = 1;
//		buf2[2] = 2;
//		buf2[3] = 3;
//		buf2[4] = 4;
//		int f = disk_write(0, buf2, 0, 1);
//		os_printf("Wrote data: %d-%d  ", buf2[0], buf2[1]);



		i2c_init();
		OLED_Init();

//		disk_initialize(0);

	//char buf2[32] = { 0 };
//
	//os_sprintf(buf, "%s", "TEST");
	//write_page(0x0000, buf);
//
	//uint32  i = read_page(0x0000, buf2);
	//os_printf("data: %s \r\n",buf2);
//
//	write_byte(0x0000, count);
//
    // repeat by restarting
   // system_restart();

}
