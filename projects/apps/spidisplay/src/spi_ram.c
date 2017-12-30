#include "espmissingincludes.h"
#include <osapi.h>
#include <eagle_soc.h>
#include "driver/spi_lite.h"		// ADXL345 definitions.
#include "gpio.h"

#define HOLD 1
// SRAM modes
#define BYTE_MODE (0x00 | HOLD)
#define PAGE_MODE (0x80 | HOLD)
#define STREAM_MODE (0x40 | HOLD)

#define RDSR 5
#define WRSR 1
#define READ 3
#define WRITE 2


static char _current_mode;

static void enable() {
    //Enable SPI communication. The SPI Enable signal must be pulsed low for each message
    GPIO_OUTPUT_SET(GPIO_ID_PIN(15), 0);
	  os_delay_us(5);
}
static void disable() {
    //Enable SPI communication. The SPI Enable signal must be pulsed low for each message
	os_delay_us(5);
    GPIO_OUTPUT_SET(GPIO_ID_PIN(15), 1);

}


static void _set_mode(char mode)
{
	//enable();

	  enable();
	  uint32 i = spi_transaction(HSPI, 8, WRSR, 0, 0, 8, mode, 0, 0);
	  disable();

	  enable();
	 char m  = spi_transaction(HSPI, 8, RDSR, 0, 0, 0, 0, 8, 0);
	 os_printf("Mode set: %d read: %d \r\n", mode, m);
	 disable();


}

void ram_init()
{
    uint8 status = 1;
    spi_init(HSPI);
    disable();
}


char read_byte(uint16 address)
{
	_set_mode(BYTE_MODE);
	char read_byte = 0;
	enable();
	 uint32 i = spi_transaction(HSPI, 8, READ, 16, address, 0, 0, 8, 0);
	 disable();
  return i;
}

void write_byte(uint16 address, char data_byte)
{
	_set_mode(BYTE_MODE);
	  enable();
      spi_transaction(HSPI, 8, WRITE, 16, address, 8, data_byte, 0, 0);
      disable();
      //spi_sendat(data_byte);
}


void read_page(int address, char *buffer, int size)
{

  //_set_mode(PAGE_MODE);
//  // Write address, read data
//  enable();
//  char read_byte;
//  enable();
//  os_delay_us(1); //Ensure a minimum delay of 500ns between falling edge of SPI Enable signal and falling edge of SPI Clock
//  spiPutAddr(address, 0, 0);
//
	_set_mode(STREAM_MODE);
	//
	  	//_set_mode(BYTE_MODE);
		enable();
  	//spi_tx16(HSPI, address);

  	spi_readblock(address, buffer, size);
//  for (i = 0; i < 32; i++) {
//	  buffer[i] = spiGetByte();
//	    os_delay_us(1);//Ensure a minimum delay of 750ns between falling edge of SPI Clock signal and rising edge of SPI Enable!
//  }
  disable();
}

void write_page(int address, char *buffer, int length)
{
//  int i;
//

//  // Set byte mode
//  _set_mode(PAGE_MODE);
	_set_mode(STREAM_MODE);
//
  	//_set_mode(BYTE_MODE);
	enable();
 // enable();
//    char read_byte;
//    enable();
//    os_delay_us(1); //Ensure a minimum delay of 500ns between falling edge of SPI Enable signal and falling edge of SPI Clock
//    spiPutAddr(address, 0, 0);
//
		//_set_mode(STREAM_MODE);
//
//  SET_PERI_REG_MASK(SPI_USER(HSPI), SPI_USR_MOSI); //enable MOSI function in SPI module
	if(length != 6) {
		os_printf("Not correct length, blocksize = 6 \r\n");
		return;
	}
	spi_sendat(address, buffer);
//  spi_sendat(0x00);
//  spi_sendat(0x00);
//  spi_sendat(0x01);
//    spi_tx32(HSPI, 0x05000002);
	//uint32 i = spi_transaction(HSPI, 8, WRITE, 16, 0x0000, 8, 25, 0, 0);
//  /spi_transaction(HSPI, 8, WRITE, 16, 0x0000, 8, 1, 0, 0);
//	      spi_transaction(HSPI, 8, WRITE, 16, address, 8, 0, 0, 0);
//	      //spi_sendat(buffer[0]);
//	          for (int i = 0; i < 32; i++) {
//	        	  	  spi_sendat(buffer[i]);
//
//	          }

	       //   write_byte(0xFF01, buffer[1]);

	  //uint32 i = spi_transaction(HSPI, 8, WRITE, 16, address, 8 * 2, buffer, 0, 0);
//    for (i = 0; i < 32; i++) {
//  	    spiPutByte(buffer);
//  	    os_delay_us(1);//Ensure a minimum delay of 750ns between falling edge of SPI Clock signal and rising edge of SPI Enable!
//    }
    disable();
}
 /*
// Stream transfer functions. Ignores page boundaries.
void SpiRAM::read_stream(int address, char *buffer, int length)
{
  int i;

  // Set byte mode
  _set_mode(STREAM_MODE);

  // Write address, read data
  enable();
  SPI.transfer(READ);
  SPI.transfer((char)(address >> 8));
  SPI.transfer((char)address);
  for (i = 0; i < length; i++) {
    buffer[i] = SPI.transfer(0xFF);
  }
  disable();
}

void SpiRAM::write_stream(int address, char *buffer, int length)
{
  int i;

  // Set byte mode
  _set_mode(STREAM_MODE);

  // Write address, read data
  enable();
  SPI.transfer(WRITE);
  SPI.transfer((char)(address >> 8));
  SPI.transfer((char)address);
  for (i = 0; i < length; i++) {
    SPI.transfer(buffer[i]);
  }
  disable();
}
*/


