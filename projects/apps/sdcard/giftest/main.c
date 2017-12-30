#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
 
static int code_masks[] = { 0x7, 0xf   };

#define BYTETOBINARYPATTERN "%d%d%d%d%d%d%d%d\r\n\r\n"
#define BYTETOBINARY(byte)  \
  (byte & 0x80 ? 1 : 0), \
  (byte & 0x40 ? 1 : 0), \
  (byte & 0x20 ? 1 : 0), \
  (byte & 0x10 ? 1 : 0), \
  (byte & 0x08 ? 1 : 0), \
  (byte & 0x04 ? 1 : 0), \
  (byte & 0x02 ? 1 : 0), \
  (byte & 0x01 ? 1 : 0) 
char chopmask[4] = { 0x00, 0x03, 0x07, 0x0c };
char bytmask[4] = { 0x00, 0x03, 0x06, 0x0c };
// 11 111 111
char * lzw_decode(char * bytes) {
    int i,x = 0;
    int file = 0;
    int bytesize = 3;
    

    long offset = 0;
    for(x = 0; x < 4; x++) {
        printf ("\r\ntotal - "BYTETOBINARYPATTERN, BYTETOBINARY(bytes[x]));   
        
        for(i = 0; i < bytesize; i++) {
            printf("realoffset = %d\r\n", offset);
            char boffset = (offset % 8);
            int filecount = (boffset + bytesize) / 8;
            //printf("offset: %d \r\n", boffset);
            int togo = 8 - boffset;            
            
            char b = (bytes[x] & (0x7 << boffset)) >> boffset;
            if(togo < bytesize) {
                b = b & chopmask[togo];
            }
             //printf("b:"BYTETOBINARYPATTERN, BYTETOBINARY(b));    
            if(filecount > 0) {
                int exceed = (((boffset + bytesize) % 8));  
                printf("exceeding by: %d\r\n", exceed);
                char bnext = bytes[x+1];
                //printf ("\r\ntotal - "BYTETOBINARYPATTERN, BYTETOBINARY(bnext));   
                
                b = b | ((bnext & bytmask[exceed+1])  & chopmask[exceed]);
                printf("offset:%d filecount:%d \r\n", boffset, filecount);  
                printf ("Leading text "BYTETOBINARYPATTERN, BYTETOBINARY(b));

            } else {
              printf("offset:%d filecount:%d \r\n", boffset, filecount);  
              printf ("Leading text   "BYTETOBINARYPATTERN, BYTETOBINARY(b));
          
            }
            offset += bytesize;
        }
        
    }

}


int main()
{
    int i = 0;
    
    
    char enc2[16] = {0x84, 0x8f, 0xa9, 0xcb, 0x2d, 0xd1, 0xa2, 0x9c, 0xb4, 0xa2, 0x02, 0x00, 0x21, 0xf9, 0x04, 0x00};
	char *dec2 = lzw_decode(enc2);
	//printf("decoded size: %d\n", _len(dec));
 

 


//	_del(dec);
 
	return 0;
}
