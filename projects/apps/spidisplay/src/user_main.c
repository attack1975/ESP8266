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
#include "driver/spi_lite.h"
#include "user_config.h"
#include "bitmap.h"

static ETSTimer tickTimer;

char buffer[128];
char buffer2[128];
char buffer3[128];
char buffer4[128];
uint8 count = 0;
void tickCb() {

	os_timer_disarm(&tickTimer);

		 read_page(0x0004, buffer2, 4);
		os_sprintf(buffer3, "Line %d  ", buffer2[0]);
		OLED_Print(9, 0, buffer3, 1);
		os_sprintf(buffer4, "Line %d  ", buffer2[1]);
		OLED_Print(9, 1, buffer4, 1);
		os_sprintf(buffer4, "Line %d  ", buffer2[2]);
		OLED_Print(9, 2, buffer4, 1);
		os_sprintf(buffer4, "Line %d  ", buffer2[3]);
		OLED_Print(9, 3, buffer4, 1);

		count++;
		if(count > 205) {
			count = 0;
		}
		buffer[0] = count;
		buffer[1] = count + 10;
		buffer[2] = count + 20;
		buffer[3] = count + 30;
		buffer[4] = count + 40;

		write_page(0x0004, buffer, 6);


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

    os_timer_arm(&tickTimer, 10, 0);


}

//Init function
void ICACHE_FLASH_ATTR
user_init()
{

//
	gpio_init();

	 PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO15);
	 PIN_PULLUP_DIS(PERIPHS_IO_MUX_MTDO_U); // disable pullodwn
	 GPIO_REG_WRITE(GPIO_ENABLE_W1TS_ADDRESS,BIT15);
	 GPIO_OUTPUT_SET(GPIO_ID_PIN(15), 0);  // CLEAR


	ram_init();
	i2c_init();
	OLED_Init();

//	 GPIO_OUTPUT_SET(GPIO_ID_PIN(5), 1);  // SET
//	 GPIO_OUTPUT_SET(GPIO_ID_PIN(5), 0);  // CLEAR

	os_printf("Starting \r\n");
	os_timer_disarm(&tickTimer);
	os_timer_setfn(&tickTimer, tickCb, NULL);
	os_timer_arm(&tickTimer, 100, 0);



	uart_init(BIT_RATE_115200, BIT_RATE_115200);
	//system_soft_wdt_stop();

	char buf[32] = { 1 };
	char buf2[32] = { 0 };
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
