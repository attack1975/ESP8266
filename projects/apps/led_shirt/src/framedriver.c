/*
 * framedriver.c
 *
 *  Created on: Jul 20, 2015
 *      Author: wouters
 */
#include "framedriver.h"
#include "config.h"
#include "ws2812.h"
#include "espmissingincludes.h"
#include "font8x8_basic.h"
#include "log.h"
#include "spi_ram.h"

static char framebuffer[BUFFERS * (COLUMNS *ROWS) * (COLORS)];
static char bitmaptextbuffer[BUFFERS][(COLUMNS * TEXTROWS)][ROWS];
int mTextLength = 0;

char mBuffers[COLUMNS * ROWS * COLORS] = { 0 };
extern int bufferrecord;
extern int bufferframes;
extern int bufferplay;

char *  get_buffersaved(int buffer) {
	os_printf("Returning buffer: %d - %d \r\n", buffer, bufferframes);
	if(buffer >= bufferframes)
		buffer = 0;
	freebuffers();
	long offset = buffer * (COLUMNS * ROWS * COLORS);
	int len = COLUMNS * ROWS * COLORS;
	os_printf("%d \r\n", (offset));
	freebuffers();
	for(int i = 0; i < len; i++) {
		mBuffers[i] = read_byte(i + offset);

	}

	return &mBuffers[0];
}

void freebuffers() {
	for(int i = 0; i < COLUMNS * ROWS * COLORS; i++) {
		if(mBuffers[i] != 0) {
			//vPortFree(mBuffers[i]);
			mBuffers[i] = 0;
		}
	}
}

void ram_memcpy(int off, char * dat, int len) {
	long offset = off * (COLUMNS * ROWS * COLORS);

	os_printf("writing buffer: %d - len:%d \r\n", offset, len);

	for(int i = 0; i < len; i++) {
		write_byte(i+offset , dat[i]);
		mBuffers[i] = dat[i];
	}
}

void  writestream(int buffer, char * data, int len) {
	if(len > (COLUMNS * ROWS * COLORS)) {
		len = COLUMNS * ROWS * COLORS;
	}
	LOG_T(LOG_FRAMEDRIVER,  LOG_FRAMEDRIVER_TAG, "Writing stream to framebuffer");

	if(bufferrecord == 1) {
//		if(mBuffers[bufferframes] == 0) {
//			mBuffers[bufferframes] = (void*) pvPortMalloc((size_t) len);
//		}
		int prevcount = 0;
		if(bufferframes != 0) {
			prevcount = bufferframes - 1;
		} else {
			ets_memcpy(&framebuffer[(buffer * COLUMNS * ROWS * COLORS)], data, len);
		}
		if(ets_memcmp(&mBuffers[0], data, len) != 0 || bufferframes == 0) {
			ram_memcpy(bufferframes, data, len);
			bufferframes++;
		}
		if(bufferframes >= 80) {
			bufferrecord = 0;
			bufferplay = 1;
		}

	} else {
		ets_memcpy(&framebuffer[(buffer * COLUMNS * ROWS * COLORS)], data, len);
	}

}


void ICACHE_FLASH_ATTR set_textpixel(int buffer, int column, int row, char pix) {
	LOG_T(LOG_FRAMEDRIVER,  LOG_FRAMEDRIVER_TAG, "settextpix - col:%d row:%d val:%d \n",column, row, pix);
	bitmaptextbuffer[buffer][column][row] = pix;
}

int tryout = 0;
char ICACHE_FLASH_ATTR get_textpixel(int buffer, int column, int row, long offset) {
	long c = column + offset;
	tryout = c % mTextLength;
	LOG_T(LOG_FRAMEDRIVER,  LOG_FRAMEDRIVER_TAG, "get_textpix: col:%d row:%d val:%d \n",c, row, bitmaptextbuffer[c][row]);
    return bitmaptextbuffer[buffer][tryout][row];
}


void ICACHE_FLASH_ATTR write_textwall_buffer(int textbuffer, char * text, int len) {
	int offset = 0;
	int bfstart = 0;
	int txtpix = 0;
	LOG_T(LOG_FRAMEDRIVER,  LOG_FRAMEDRIVER_TAG, "Writing text to textbuffer");
	for(int i = 0; i < (COLUMNS * TEXTROWS); i++) {
			if(txtpix % TEXTHEIGHT == 0 && i != 0) {
					bfstart++;
					offset += 0;

			}
			while(text[bfstart] == '~') { // when special char;
				for(int z = 0; z < (COLUMNS); z++) {
					for(int y = 0; y < ROWS; y++) {
						set_textpixel(textbuffer, i + offset + z,  y, false);
					}
				}
				offset = offset + (COLUMNS);	//JUMP EXACTLY 1 FRAME, for loop does next ++
				bfstart++;
			}

			for(int x = 0; x < ROWS; x++) {
				int a = (1 << (i % ROWS));
				if(bfstart >= len) {
					mTextLength = i + offset;
					return;
				}
				bool pix = ((font8x8_basic[text[bfstart]][(ROWS-1)-x]) & a);
				set_textpixel(textbuffer, i + offset, x, pix);
			}
			txtpix++;

	}
}

void ICACHE_FLASH_ATTR write_texttowall(int buffer, int textbuffer, long offset, int fR, int fG, int fB, int fbR, int fbG, int fbB) {
	LOG_T(LOG_FRAMEDRIVER,  LOG_FRAMEDRIVER_TAG, "Writing textframe to framebuffer");

	 for(int i = 0; i < COLUMNS; i++) {
		for(int x = 0; x < ROWS; x++) {
			char pix = get_textpixel(textbuffer, i, x, offset);
			if(pix > 0) {
				setpixel(buffer, i, x,  fR, fG, fB);
			} else {
				setpixel(buffer, i, x,  fbR, fbG, fbB);
			}
		}
	 }
}


void setled(char* data, int len ,int dim) {
	WS2812OutBuffer(data, len, dim);
}


char * ICACHE_FLASH_ATTR get_startbuffer(int buffer) {
	return &framebuffer[buffer * COLUMNS * ROWS * COLORS];
}
void ICACHE_FLASH_ATTR set_buffer(int buffer, int val1, int val2, int val3) {
	LOG_T(LOG_FRAMEDRIVER,  LOG_FRAMEDRIVER_TAG, "Set buffer to value");
	for(int i = 0; i < COLUMNS; i++) {
		for(int x = 0; x < ROWS; x++) {
			setpixel(buffer, i, x, val1, val2, val3);
		}
	}
}
void ICACHE_FLASH_ATTR clear_buffer(int buffer) {
	for(int i = 0; i < COLUMNS; i++) {
		for(int x = 0; x < ROWS; x++) {
			setpixel(buffer, i, x, 0x00, 0x00, 0x00);
		}
	}
}

void ICACHE_FLASH_ATTR setpixel(int buffer, int column, int row, char g, char r, char b) {
	int p = (buffer * COLUMNS * ROWS * COLORS) + (column * ROWS * COLORS) + (row * COLORS);
	LOG_T(LOG_FRAMEDRIVER,  LOG_FRAMEDRIVER_TAG, "Setpixel - led:%d col:%d row:%d val[0]:%d val[1]:%d val[2]:%d \n",p , column, row, r, g ,b);
	framebuffer[p] = g;
	framebuffer[p +1] = r;
	framebuffer[p +2] = b;
	system_soft_wdt_feed();
}

char* ICACHE_FLASH_ATTR getpixel(int buffer, int column, int row) {
	int p = (buffer * COLUMNS * ROWS * COLORS) + (column * ROWS * COLORS) + (row * COLORS);
    return &framebuffer[p];
}


