
#define _GNU_SOURCE
#include <stdio.h>
/* For "exit". */
#include <stdlib.h>
/* For "strerror". */
#include <string.h>
/* For "errno". */
#include <errno.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

#include <dirent.h>


#include <sys/stat.h>

#include <zlib.h>

/* CHUNK is the size of the memory chunk used by the zlib routines. */

#define CHUNK 0x4000

/* The following macro calls a zlib routine and checks the return
   value. If the return value ("status") is not OK, it prints an error
   message and exits the program. Zlib's error statuses are all less
   than zero. */

#define CALL_ZLIB(x) {                                                  \
        int status;                                                     \
        status = x;                                                     \
        if (status < 0) {                                               \
            fprintf (stderr,                                            \
                     "%s:%d: %s returned a bad status of %d.\n",        \
                     __FILE__, __LINE__, #x, status);                   \
            exit (EXIT_FAILURE);                                        \
        }                                                               \
    }

/* if "test" is true, print an error message and halt execution. */

#define FAIL(test,message) {                             \
        if (test) {                                      \
            fprintf (stderr, "%s:%d: " message           \
                     " file '%s' failed: %s\n",          \
                     __FILE__, __LINE__,       \
                     strerror (errno));                  \
            exit (EXIT_FAILURE);                         \
        }                                                \
    }

/* These are parameters to inflateInit2. See
   http://zlib.net/manual.html for the exact meanings. */

#define windowBits 15
#define ENABLE_ZLIB_GZIP 32

#define windowBits 15
#define GZIP_ENCODING 16



unsigned char totalbuffer[131072*8];

#define SPI_FLASH_SEC_SIZE 4096
#define MFS_STARTFLASHSECTOR  0x100
#define MFS_START	(MFS_STARTFLASHSECTOR*SPI_FLASH_SEC_SIZE)
#define MFS_SECTOR	256
#define MFS_FILENAMELEN 32-8
#define ENTRIES 8192

#define ENDIAN(x) x//htonl



static void strm_init (z_stream * strm)
{
    strm->zalloc = Z_NULL;
    strm->zfree  = Z_NULL;
    strm->opaque = Z_NULL;
    CALL_ZLIB (deflateInit2 (strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
                             windowBits | GZIP_ENCODING, 8,
                             Z_DEFAULT_STRATEGY));
}

/* Compress from file source to file dest until EOF on source.
   def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_STREAM_ERROR if an invalid compression
   level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
   version of the library linked do not match, or Z_ERRNO if there is
   an error reading or writing the files. */
int def(FILE *source, FILE *dest, int level)
{
    int ret, flush;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
//    strm_init(&strm);
//    ret = deflateInit(&strm, level);
  
 
          
                
                
     ret = deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, (15+16), 8, Z_DEFAULT_STRATEGY);  

  
  
     printf("start \r\n");
    if (ret != Z_OK) {
        printf("ERROR\r\n");
        return ret;
    }

    /* compress until end of file */
    do {
        strm.avail_in = fread(in, 1, CHUNK, source);
        if (ferror(source)) {
            (void)deflateEnd(&strm);
            return Z_ERRNO;
        }
        flush = feof(source) ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = in;

        /* run deflate() on input until output buffer not full, finish
           compression if all of source has been read in */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = deflate(&strm, flush);    /* no bad return value */
            //assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            have = CHUNK - strm.avail_out;
            if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                printf("done \r\n");
                (void)deflateEnd(&strm);
                return Z_ERRNO;
            }
        } while (strm.avail_out == 0);
        //assert(strm.avail_in == 0);     /* all input will be used */

        /* done when last data in file processed */
    } while (flush != Z_FINISH);
                    printf("done \r\n");
    //assert(ret == Z_STREAM_END);        /* stream will be complete */

    /* clean up and return */
    (void)deflateEnd(&strm);
    return Z_OK;
}


int main2 (FILE * file, char * file_name_out)
{
   // const char * file_name = "./html/socket.js";

    FILE * filewrite;

    filewrite = fopen(file_name_out , "w" );
    printf("\r\n");
   // FAIL (! file, "open");
    FAIL (! filewrite, "open");
    
    def(file, filewrite, Z_DEFAULT_COMPRESSION);
    return 0;
}



int main( int argc, char ** argv )
{
    int i = 0;
    DIR           *d;
    	char * dirname = argv[1];

    	d = opendir( dirname );

    	

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
					
					
					char bufname[100];
					sprintf(bufname, "%s/%s", "html_compr", namelist[n]->d_name);
					printf("%s", bufname);
					main2(f, bufname);
					fclose( f );
				
				}
    		}
        }

    

//	_del(dec);
 
	return 0;
}




