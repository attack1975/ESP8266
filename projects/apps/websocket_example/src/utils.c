#include "espmissingincludes.h"
#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"



void ICACHE_FLASH_ATTR hexDump2 (char *desc, void *addr, int len, int offset) {
	  int i;
	    unsigned char buff[17];
	    unsigned char *pc = (unsigned char*)addr;

	    // Output description if given.
	    if (desc != 0)
	    	os_printf ("%s:\n", desc);

	    if (len == 0) {
	    	os_printf("  ZERO LENGTH\n");
	        return;
	    }
	    if (len < 0) {
	    	os_printf("  NEGATIVE LENGTH: %i\n",len);
	        return;
	    }
	    os_printf("len:%d\n", len);
	    // Process every byte in the data.
	    for (i = 0; i < len; i++) {
	        // Multiple of 16 means new line (with line offset).

	        if ((i % 16) == 0) {
	            // Just don't print ASCII for the zeroth line.
	            if (i != 0)
	            	os_printf ("  %s\n", buff);

	            // Output the offset.
	            os_printf ("  %04x ", i+offset);
	        }

	        // Now the hex code for the specific character.
	        os_printf (" %02x", pc[i]);

	        // And store a printable ASCII character for later.
	        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
	            buff[i % 16] = '.';
	        else
	            buff[i % 16] = pc[i];
	        buff[(i % 16) + 1] = '\0';
	    }

	    while ((i % 16) != 0) {
	        os_printf ("   ");
	        i++;
	    }

	    os_printf ("  %s\n", buff);
}
void ICACHE_FLASH_ATTR hexDump (char *desc, void *addr, int len) {
	hexDump2(desc, addr, len, 0);
}
