#include "espmissingincludes.h"
#include <osapi.h>
#include <eagle_soc.h>
#include "os_type.h"

//#include "driver/spi_lite.h"		// ADXL345 definitions.

#include "gpio.h"

//#define HOLD 1
//// SRAM modes
//#define BYTE_MODE (0x00 | HOLD)
//#define PAGE_MODE (0x80 | HOLD)
//#define STREAM_MODE (0x40 | HOLD)
//
//#define RDSR 5
//#define WRSR 1
//#define READ 3
//#define WRITE 2

void sd_init()
{
//    uint8 status = 1;
//
//
//    disable();
//    while(spi_busy(HSPI));	//wait for SPI transaction to complete
//    spi_tx32(HSPI, 0xFFFFFFFF);
//    spi_tx32(HSPI, 0xFFFFFFFF);
//    spi_tx16(HSPI, 0xFFFF);
//    os_printf("Init 1: %d \r\n", 0);
//	while(spi_busy(HSPI));	//wait for SPI transaction to complete
//
//	os_delay_us(10);
//	enable();
//    spi_transaction(HSPI, 8, 0x40, 16, 0x0000, 24, 0x000095, 0, 0);
//	while(spi_busy(HSPI));	//wait for SPI transaction to complete
//	int ret = 0;
//	for(int i = 0; i < 8; i++) {
//	    int x = spi_transaction(HSPI, 0,0, 0, 0, 8, 0xFF, 8, 0);
//		while(spi_busy(HSPI));	//wait for SPI transaction to complete
//	}
//	disable();
//	os_delay_us(10);
//
////		SET_PERI_REG_MASK(SPI_USER(HSPI), SPI_USR_MOSI); //enable MOSI function in SPI module
////		WRITE_PERI_REG(SPI_W0(HSPI), 0xFFFFFFFF);
////		SET_PERI_REG_MASK(SPI_CMD(HSPI), SPI_USR);
//
//
//
//	//i = spi_transaction(HSPI, 32, 0xFFFFFFFF, 32, 0xFFFFFFFF, 0, 0, 0, 0);
////	while(spi_busy(HSPI));	//wait for SPI transaction to complete
//
//    disable();
//
////    enable();
////    uint32 t = spi_transaction(HSPI, 8, 0x40, 32, 0, 8, 0x95, 32, 0);
////    os_printf("Init 3: %p \r\n", t);
////    disable();
//    //enable();
//    //t= spi_transaction(HSPI, 0, 0, 0, 0, 8, 0xFF, 32, 0);
//    //os_printf("Init 3: %p \r\n", t);
//
//
////



}

