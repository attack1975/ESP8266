#include "espmissingincludes.h"
#include <osapi.h>
#include <eagle_soc.h>
#include "driver/spi.h"		// ADXL345 definitions.


void ram_init()
{
    uint8 status = 1;
    spi_init();
    //disable();
}

