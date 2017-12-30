#ifndef _FS_H_
#define _FS_H_
#include "espmissingincludes.h"
#include "ets_sys.h"
#include "osapi.h"
#include "c_types.h"
#include "spi_flash.h"
#include "ssl/ssl_crypto.h"

extern void MD5Init  ( MD5_CTX *mdContext);
extern void MD5Update( MD5_CTX *mdContext, const unsigned char *inBuf, unsigned int inLen);
extern void MD5Final ( unsigned char hash[], MD5_CTX *mdContext);
extern int (*flash_global_writer)( uint32 from1, uint32 to1, uint32 size1, uint32 from2, uint32 to2, uint32 size2 );


#define BEGIN_ADDRESS_GIF 0x100000
#define BEGIN_ADDRESS_HTML 0x150000
#define MAX_FILES 40
#define MAX_FILENAME 20

#define ESP_PARAM_START_SEC		0x3D

#define ICACHE_STORE_ATTR
extern SpiFlashChip * flashchip;



typedef struct {
	uint32      from1;
    uint32      to1;
    uint32 	 size1;
    char     md51[48];

    uint32      from2;
    uint32      to2;
	uint32 	 size2;
    char     md52[48];
} CopyCommand;


typedef struct INDEXSTRUCT {
	char unique[4];
	char filesactive;
	char filename[MAX_FILES][MAX_FILENAME];
	uint32 len[MAX_FILES];
	uint32 offset[MAX_FILES];
} INDEXSTRUCT;

typedef struct FILESTRUCT {
	uint32 len;
	char * data;
} FILESTRUCT;

INDEXSTRUCT fs_readindex();
void* fs_readfile(uint32 address, char * buf, int i);
void* fs_readfile_max(uint32 address, char * buf, int i, int len, int offset);
int fs_findfile_name(uint32* address, char * name);

#endif
