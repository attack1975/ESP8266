#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
 
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <stdint.h>
unsigned char totalbuffer[131072*8];

#define SPI_FLASH_SEC_SIZE 4096
#define MFS_STARTFLASHSECTOR  0x100
#define MFS_START	(MFS_STARTFLASHSECTOR*SPI_FLASH_SEC_SIZE)
#define MFS_SECTOR	256
#define MFS_FILENAMELEN 32-8
#define ENTRIES 8192

#define ENDIAN(x) x//htonl

#define MAX_FILES 40
#define MAX_FILENAME 20

typedef struct INDEXSTRUCT {
	char unique[4];
	char filesactive;
	char filename[MAX_FILES][MAX_FILENAME];
	int len[MAX_FILES];
	int offset[MAX_FILES];
} INDEXSTRUCT;



int main( int argc, char ** argv )
{
    int i = 0;
    DIR           *d;
    	char * dirname = argv[1];

    	d = opendir( dirname );

    	INDEXSTRUCT str;
    		str.unique[0] = '1';
    		str.unique[1] = '2';
    		str.unique[2] = '3';
    		str.unique[3] = 0;
    		str.filesactive = 1;

    		int filecount = 0;
    		long datapointer = 0;

    		 struct dirent **namelist;
    		 int n;

    		  int nn = scandir(dirname, &namelist, 0, alphasort);

    	for(n = 0; n < nn; n++)
        {
    		if( namelist[n]->d_type & DT_REG )
    		{
				char thisfile[1024];
				struct stat buf;
				int dlen = strlen( namelist[n]->d_name );
				int sprret = snprintf( thisfile, 1023, "%s/%s", dirname, namelist[n]->d_name );
				printf("%s\r\n",namelist[n]->d_name);
				if( sprret > 1023 || sprret < 1 )
				{
					fprintf( stderr, "Error processing1 \"%s\" (snprintf)\n", namelist[n]->d_name );
					continue;
				}

				int statret = stat( thisfile, &buf );

				if( statret )
				{
					fprintf( stderr, "Error processing2 \"%s\" (stat)\n", thisfile );
					continue;
				}
				if( buf.st_size )
				{
					FILE * f = fopen( thisfile, "rb" );
					if( !f )
					{
						fprintf( stderr, "Error: cannot open3 \"%s\" for reading.\n", namelist[n]->d_name );
						return -9;
					}
					fread( &totalbuffer[datapointer], 1, buf.st_size, f );
					fclose( f );
					int rs = buf.st_size;

					sprintf(str.filename[filecount], "%s",namelist[n]->d_name);
					str.offset[filecount] = datapointer;
					str.len[filecount] = rs;
					printf( "%s: %d (%ld)\n", thisfile, rs, datapointer );
					//rs = (rs+1)&(~(1));
					datapointer += rs;


					filecount++;
				}
    		}
        }

    		FILE * f = fopen( "test.out", "w" );
    		str.filesactive = filecount;
    		if( !f || ferror( f ) )
    		{
    			fprintf( stderr, "Error: cannot open \"%s\" for writing.\n", argv[2] );
    		}
    		fwrite( &str, sizeof(INDEXSTRUCT), 1, f );
    		fwrite( totalbuffer, datapointer, 1, f );
    		fclose( f );


//	_del(dec);
 
	return 0;
}
