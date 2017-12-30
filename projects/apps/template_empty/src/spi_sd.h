#ifndef SPI_SD_H
#define SPI_SD_H

// SRAM modes
#define BYTE_MODE (0x00 | HOLD)
#define PAGE_MODE (0x80 | HOLD)
#define STREAM_MODE (0x40 | HOLD)

void sd_init();

char read_byte(int address);
char write_byte(int address, char data_byte);
void read_page(int address, char *buffer);
void write_page(int address, char *buffer);


#endif
