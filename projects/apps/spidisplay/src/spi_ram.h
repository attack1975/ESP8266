#ifndef GDBSTUB_H
#define GDBSTUB_H

// SRAM modes
#define BYTE_MODE (0x00 | HOLD)
#define PAGE_MODE (0x80 | HOLD)
#define STREAM_MODE (0x40 | HOLD)

void ram_init();

char read_byte(int address);
char write_byte(int address, char data_byte);
void read_page(int address, char *buffer);
void write_page(int address, char *buffer);


#endif
