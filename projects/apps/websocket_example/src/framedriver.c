
#include "espmissingincludes.h"
#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "fs.h"
#include "framedriver.h"
#include "font8x8_basic.h"
#include "include/gif.h"
#include "ws2812_i2s.h"
#include "lwip_websocket.h"

int curanim = 0;
extern char animation_custom;

extern uint32_t current_settings[FRC_INVALID];
extern int animationenabled;

extern char dimmode;
extern char ledfix;
extern int fontcolorR;
extern int fontcolorG;
extern int fontcolorB;

extern INDEXSTRUCT str;
extern char animation_random_choises[50];
//extern long textoffset;

static Colour gifcolors[128];
static Gif * g = 0;

static char trcolor = 0;
static int pulsedir = 0;
static float pulsecount = 0;

static char bitmaptextbuffer[BUFFERS][(COLUMNS * TEXTROWS)][ROWS];
static int mTextLength = 0;

char lTmlBuf[totleds];
static char lTmlBuf2[totleds];
static char lTmpBufComplete[ROWS][COLUMNS][COLORS];

void ICACHE_FLASH_ATTR static copy_next_frame(long c, int anim, char buf[ROWS][COLUMNS][COLORS], int offset);

static char ICACHE_FLASH_ATTR get_textpixel(int buffer, int column, int row, long offset) {
	long c = column + offset;
    return bitmaptextbuffer[buffer][c % mTextLength][row];
}


static void ICACHE_FLASH_ATTR setpixel(int buffer, int column, int row, char g, char r, char b) {
	int p = (buffer * COLUMNS * ROWS * COLORS) + (column * ROWS * COLORS) + (row * COLORS);
	lTmlBuf[p] = g;
	lTmlBuf[p +1] = r;
	lTmlBuf[p +2] = b;
	system_soft_wdt_feed();
}

static void ICACHE_FLASH_ATTR set_textpixel(int buffer, int column, int row, char pix) {
	bitmaptextbuffer[buffer][column][row] = pix;
}

static void ICACHE_FLASH_ATTR rainbow_calc(int progress, int * color)
{
	double r = 0.0;
	double g = 0.0;
	double b = 0.0;
	double h = (double)(progress % 120) / 120;
	int i = (int)(h * 6);
	double f = h * 6.0 - i;
	double q = 1 - f;

	            switch (i % 6)
	            {
	                case 0:
	                    r = 1;
	                    g = f;
	                    b = 0;
	                    break;
	                case 1:
	                    r = q;
	                    g = 1;
	                    b = 0;
	                    break;
	                case 2:
	                    r = 0;
	                    g = 1;
	                    b = f;
	                    break;
	                case 3:
	                    r = 0;
	                    g = q;
	                    b = 1;
	                    break;
	                case 4:
	                    r = f;
	                    g = 0;
	                    b = 1;
	                    break;
	                case 5:
	                    r = 1;
	                    g = 0;
	                    b = q;
	                    break;
	            }

	            color[0] = (r * 255);
	            color[1] = (g * 255);
	            color[2] = (b * 255);
}

static void ICACHE_FLASH_ATTR rainbow_copybuffer(int buffer, int frame) {
	int cc[3];
	int count = 0;
	for(int i = 0; i < COLUMNS; i++) {
		for(int x = 0; x < ROWS; x++) {
			rainbow_calc(count+frame, &cc[0]);
			setpixel(buffer, i, x,  cc[0], cc[1], cc[2]);
			count++;
		}
	 }
}




static void ICACHE_FLASH_ATTR rainbowcolor_copybuffer(int buffer, int frame) {
	int cc[3];
	int count = 0;
	rainbow_calc(frame, &cc[0]);
	for(int i = 0; i < COLUMNS; i++) {
		for(int x = 0; x < ROWS; x++) {
			setpixel(buffer, i, x,  cc[0], cc[1], cc[2]);
			count++;
		}
	 }
}
static int c1;
static int c2;
static int c3;

static void ICACHE_FLASH_ATTR flicker_copybuffer(int buffer, int frame) {
	int count = 0;
	static int oldframe;
	if(oldframe != frame) {
		c1 = rand() % 255;
		c2 = rand() % 255;
		c3 = rand() % 255;
		oldframe = frame;
	}

	for(int i = 0; i < COLUMNS; i++) {
		for(int x = 0; x < ROWS; x++) {

			setpixel(buffer, i, x,  c1, c2, c3);
			count++;
		}
	 }
}



static ICACHE_FLASH_ATTR void writeletter(int textbuffer, char cha, int off) {
			for(int x = 0; x < 5; x++) {
			for(int i = 0; i < 5; i++) {
				//os_printf("writing letter: %d \r\n", cha);
				int pix = smallfont[cha][(5-x)] & (1 << 6 - i); // smallfont[0][2]   01111100
				if(pix) {
					//os_printf("\t%d", pix  > 0 ? 1 : 0);
				}else {
					//os_printf("\t");
				}
				set_textpixel(textbuffer, i + off, x + 1, pix  > 0 ? 1 : 0);
			}
			//os_printf("\r\n");
		}
}



void ICACHE_FLASH_ATTR write_textwall_buffer(int textbuffer, char * textp, int len) {
	char text[250];
	os_memset(text, 0, 250);
	os_memcpy(text, textp, len);
	int bfstart = 0;
	int txtpix = 0;
	for(int i = 0; i < COLUMNS * TEXTROWS; i++) {
		for(int x = 0; x < ROWS; x++) {
			bitmaptextbuffer[0][i][x] = 0;
		}
	}
	if(current_settings[FRC_CONSOLE_ENABLE])
	os_printf( "Writing text to textbuffer:%d-%s\r\n", len, text);

	if(current_settings[FRC_TEXT_BIG]) {
		int letteroffset = 0;
		for(int i = 0; i < (COLUMNS * TEXTROWS); i++) {
			char tmpchar = text[bfstart];
			char emptyscreen = 0;

			if(txtpix % TEXTHEIGHT == 0 && i != 0 && tmpchar != '~') {
					bfstart++;
					letteroffset = 0;
			} else if(tmpchar == '~') {
				if(letteroffset >= COLUMNS && i != 0) {
					bfstart++;
					letteroffset = 0;
					emptyscreen = 0;
				}
			}

			int r = ROWS;

			tmpchar = text[bfstart];
			if(tmpchar == '~') {
				emptyscreen = 1;
			}

			for(int x = 0; x < ROWS; x++) {
				int a = (1 << (i % ROWS));
				if(bfstart >= len) {
					mTextLength = i;
					//os_printf( "Done:%c-%d \r\n", tmpchar, i);
					return;
				}

				if(emptyscreen) {
					set_textpixel(textbuffer, i, x, false);
				} else {
					bool pix = ((font8x8_basic[text[bfstart]][(ROWS-1)-x]) & a);
					//os_printf("Writing pix: %d,  - %c", pix, tmpchar);
					set_textpixel(textbuffer, i, x, pix);
				}
			}
			//os_printf("Writing for position: %c - %d - %d \r\n", tmpchar, txtpix, letteroffset);
			txtpix++;
			letteroffset++;
		}

	} else {
		if(len > 30) {
			len = 30;
		}
		if(current_settings[FRC_CONSOLE_ENABLE])
		os_printf("Length: %d \r\n", len);

		int offset = 0;
		for(int i= 0; i < len; i++) {
			if(current_settings[FRC_CONSOLE_ENABLE])
			os_printf( "%c %d \r\n", text[i], text[i]);



			if(text[i] == ' ') {
				writeletter(0, 26, offset);
				offset += 3;
			} else if(text[i] == '!') {
				writeletter(0, 37, offset);
				offset += 4;
			}  else if(text[i] == '.') {
				writeletter(0, 39, offset);
				offset += 2;
			}  else if(text[i] == '-') {
				writeletter(0, 38, offset);
				offset += 3;
			}  else if(text[i] == '~') {
				//if(console_output)
				os_printf("Writing screenwidth: + %d \r\n", COLUMNS);
				offset += COLUMNS;
			} else {
				if(text[i] > 64 && text[i] < 91) {
					text[i] += 32;
				}
				if(text[i] > 47 && text[i] < 58) {
					text[i] += 76;
				}
				int c = text[i] - 97;

				if(current_settings[FRC_CONSOLE_ENABLE])
				os_printf("getting char from bufer pos: %d \r\n", c);
				writeletter(0, c, offset);
				offset += 6;
			}
		}
		mTextLength = offset;
		//writeletter(0, 26, offset);
	}
}

static void ICACHE_FLASH_ATTR write_texttowall(int buffer, int textbuffer, long offset, int fR, int fG, int fB, int fbR, int fbG, int fbB) {
	//os_printf("Writing textframe to framebuffer \r\n");
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


static void  ICACHE_FLASH_ATTR WS2812CopyBuffer2( uint8_t * buffer, uint16_t length, int flicker, int dim)
{
	uint16_t i;


	int tmpdim = dim;
	if(tmpdim > 255) {
		tmpdim = 254;
	}
	float tdim = (tmpdim / 255.0);
	float tmp2;

	int npos;
	int tot;
	uint8_t byte;
	for( i = 0; i < length; i++ )
	{
		system_soft_wdt_feed();
		int val = i;
		if(i % 3 == 0) {
			val = val + 1;
		}
		if(i % 3 == 1) {
			val = val - 1;
		}

		tot = i / 3;

		npos = (ledlookuphorizhat[tot] * 3) + (val % 3); // ledlookuphorizhat    ledlookuphoriz
		//ledlookuphorizhat
		byte = buffer[npos];
		if((tot == 14 || tot == 77 || tot == 78 ||tot == 81)  && ledfix == 1 && dimmode == 1 && dim != 0 ) {
			tmp2 = (byte * (tdim+0.05));
			if(tmp2 < 255) {
				byte = tmp2;
			} else {
				byte = byte * tdim;
			}
		} else {
			byte = byte * tdim;
		}

		if(flicker) {
			byte = 0x00;
		}
		byte = GammaE[byte];
		//totalPower += byte;
		lTmlBuf2[i] = byte;
	}
}

static void  ICACHE_FLASH_ATTR WS2812CopyBuffer( char  buffer[ROWS][COLUMNS][COLORS], uint16_t length, int flicker, int dim)
{
	//os_printf("Going to copy \r\n");
	int count = 0;
	for(int i = 0; i < 16; i++) {
		for(int x = 7; x >= 0; x--) {
			lTmlBuf[count] =   buffer[x][i][0];
			lTmlBuf[count+1] = buffer[x][i][1];
			lTmlBuf[count+2] = buffer[x][i][2];
			count+=3;
		}
	}
	WS2812CopyBuffer2(lTmlBuf, length, flicker, dim);
}


void ICACHE_FLASH_ATTR copy_color(int buffer, char r, char g, char b, char brightness) {
	for(int i = 0; i < COLUMNS; i++) {
		for(int x = 0; x < ROWS; x++) {
			setpixel(buffer, i, x,  r, g, b);
		}
	 }
	WS2812CopyBuffer2(lTmlBuf, totleds, 0, brightness);
	ws2812_push(lTmlBuf2, totleds);
}

void ICACHE_FLASH_ATTR copy_text(char * text, int offset) {
	int size = os_strlen(text);
	write_textwall_buffer(0, text, size);
	write_texttowall(0, 0, 5, fontcolorR,fontcolorG,fontcolorB, 0,0,0);
	WS2812CopyBuffer2(lTmlBuf, totleds, 0, 50);
	ws2812_push(lTmlBuf2, totleds);
}





void ICACHE_FLASH_ATTR animation_prev() {
	if(animation_random_choises[0] != '0' && animation_random_choises[0] != 0 && animation_random_choises[0] != 255) {
		current_settings[FRC_GIF_VALUE]--;
		int i = 0;
		for(i = current_settings[FRC_ANIMATION_COUNTER]; i >= 0; i--) {
			if(animation_random_choises[i] != '0' && animation_random_choises[i] != 255 && current_settings[FRC_ANIMATION_COUNTER] != i) {
				current_settings[FRC_GIF_VALUE] = animation_random_choises[i];
				//os_printf("Setting animation random: %d \r\n", i);
				current_settings[FRC_ANIMATION_COUNTER] = i;
				break;
			}
		}
		if(i < 0) { // special text added as last animation
			current_settings[FRC_GIF_VALUE] = str.filesactive;
		}

		if(i >= 48) {
			current_settings[FRC_ANIMATION_COUNTER] = 0;
			current_settings[FRC_GIF_VALUE] = animation_random_choises[0];
		}
	} else {
		current_settings[FRC_GIF_VALUE]--;
		if(current_settings[FRC_GIF_VALUE] == 0) {
			 // special text added as last animation
			current_settings[FRC_GIF_VALUE] = str.filesactive;
		}
	}
}

void ICACHE_FLASH_ATTR animation_next() {
	if(animation_random_choises[0] != '0' && animation_random_choises[0] != 0 && animation_random_choises[0] != 255) {
		current_settings[FRC_GIF_VALUE]++;
		int i = 0;
		for(i = current_settings[FRC_ANIMATION_COUNTER]; i < 50; i++) {
			if(animation_random_choises[i] != '0' && animation_random_choises[i] != 255 && current_settings[FRC_ANIMATION_COUNTER] != i) {
				current_settings[FRC_GIF_VALUE] = animation_random_choises[i];
				//os_printf("Setting animation random: %d \r\n", i);
				current_settings[FRC_ANIMATION_COUNTER] = i;
				break;
			}
		}
		if(current_settings[FRC_GIF_VALUE] == str.filesactive) { // special text added as last animation
			current_settings[FRC_TEXT_OFFSET] = 0;
		}
		if(i >= 48) {
			current_settings[FRC_ANIMATION_COUNTER] = 0;
			current_settings[FRC_GIF_VALUE] = animation_random_choises[0];
		}
	} else {
		current_settings[FRC_GIF_VALUE]++;
		if(current_settings[FRC_GIF_VALUE] == str.filesactive) { // special text added as last animation
			current_settings[FRC_TEXT_OFFSET] = 0;
		}
		if(current_settings[FRC_GIF_VALUE] > str.filesactive) {
			current_settings[FRC_GIF_VALUE] = 1;
		}
	}
}
int fadetemp = 0;

void ICACHE_FLASH_ATTR framedriver_refresh(int brightness) {
	int f = 0;


	if(current_settings[FRC_FLICKER_ENABLE] == 1) {
		int fspeed = 105 - current_settings[FRC_FLICKER_SPEED];
		if(current_settings[FRC_FRAME_COUNTER] % (fspeed) < (fspeed) / 2) {
			f = 1;
		}
	}

	if(current_settings[FRC_FRAME_COUNTER] % current_settings[FRC_ANIMATION_SPEED] == 0) {
		current_settings[FRC_ANIMATION_OFFSET] +=2;
	}


	if(current_settings[FRC_FRAME_COUNTER] % (current_settings[FRC_TEXT_SPEED] + 1/ 2) == 0) {
		current_settings[FRC_TEXT_OFFSET]++;
		if(current_settings[FRC_TEXT_OFFSET] > (TEXTROWS * mTextLength)) {
			current_settings[FRC_TEXT_OFFSET] = 0;
		}
	}

	int tmpbright = brightness;
	fadetemp++;

	if(current_settings[FRC_FADE_ENABLE] == 1) {
		if(fadetemp > 5) {
			fadetemp = 0;
			if(pulsedir == 0) {
				pulsecount += (current_settings[FRC_FADE_SPEED]+1);
				if(pulsecount >= (brightness)) {
					pulsedir = 1;
					pulsecount = brightness - 1;
				}
			} else {
				pulsecount -= (current_settings[FRC_FADE_SPEED]+1);
				if(pulsecount <= 0) {
					pulsedir = 0;
					pulsecount = 1;
				}
			}
		}


		if(tmpbright - pulsecount > 0 && tmpbright - pulsecount < 255) {
			tmpbright -= (pulsecount);
		}
	}

	if(animationenabled && current_settings[FRC_FLASHLIGHT_ENABLE] == 0) {
		if(current_settings[FRC_SLIDESHOW_ENABLE]) {
			if(current_settings[FRC_FRAME_COUNTER] % (current_settings[FRC_SLIDESHOW_SPEED] * 100) == 0) {
				animation_next();
			}
		}
		if(current_settings[FRC_GIF_VALUE] > 3 && current_settings[FRC_GIF_VALUE] != str.filesactive) {
			copy_next_frame(current_settings[FRC_FRAME_COUNTER], current_settings[FRC_GIF_VALUE], lTmpBufComplete, 0);
			WS2812CopyBuffer(lTmpBufComplete, totleds, f, tmpbright);
		} else {
			if(current_settings[FRC_GIF_VALUE] == 1) {
				rainbow_copybuffer(0, current_settings[FRC_ANIMATION_OFFSET]);
				WS2812CopyBuffer2(lTmlBuf, totleds, f, tmpbright);
			} else if(current_settings[FRC_GIF_VALUE] == 2) {
				flicker_copybuffer(0, current_settings[FRC_ANIMATION_OFFSET]);
				WS2812CopyBuffer2(lTmlBuf, totleds, f, tmpbright);
			}  else if(current_settings[FRC_GIF_VALUE] == 3) {
				rainbowcolor_copybuffer(0, current_settings[FRC_ANIMATION_OFFSET]);
				WS2812CopyBuffer2(lTmlBuf, totleds, f, tmpbright);
			} else if(current_settings[FRC_GIF_VALUE] == str.filesactive) { // special text added as last animation
				if(g != 0) {
					del_gif(g);
					g = 0;
				}

				write_texttowall(0, 0, current_settings[FRC_TEXT_OFFSET], fontcolorR,fontcolorG,fontcolorB,0,0,0);
				WS2812CopyBuffer2(lTmlBuf, totleds, f, tmpbright);
			}
		}
	} else if(current_settings[FRC_FLASHLIGHT_ENABLE] == 0) {
		if(g != 0) {
			del_gif(g);
			g = 0;
		}
		if(animation_custom == 0)
		write_texttowall(0, 0, current_settings[FRC_TEXT_OFFSET], fontcolorR,fontcolorG,fontcolorB,0,0,0);
		WS2812CopyBuffer2(lTmlBuf, totleds, f, tmpbright);
	} else {
		copy_color(0,255,255,255, brightness);
	}

	//if(frame_counter % 100 == 0 && flashingenabled == 0) {
	//	send_frametosocket(lTmlBuf, totleds);
	//}
	ws2812_push(lTmlBuf2, totleds);
	current_settings[FRC_FRAME_COUNTER]++;
}
void ICACHE_FLASH_ATTR frame_freegif() {
	if(g != 0) {
		del_gif(g);
		g = 0;
	}
}


void ICACHE_FLASH_ATTR send_frametosocket(char * buf, int length) {
	char tbuf[1024];
	int count = 0;
	os_memset(tbuf, 0 , 1024);
	tbuf[0] = 'L';
	tbuf[1] = 'E';
	tbuf[2] = 'D';
	tbuf[3] = ':';
	count = 4;
	if(length > 1024) {
		length = 1020;
	}
	os_memcpy(&tbuf[4], buf, length);
	websocket_writedata_size(tbuf, length, WS_BINARY, 1);
	if(current_settings[FRC_CONSOLE_ENABLE])
	os_printf("writing data\r\n");
}
void ICACHE_FLASH_ATTR static copy_next_frame(long c, int anim, char  buf[ROWS][COLUMNS][COLORS], int offset) {
	if(current_settings[FRC_CONSOLE_ENABLE])
		os_printf("copy_next_frame: curanim: %d anim:%d offset:%d  g:%p \r\n",curanim, anim, offset, g);

	if(curanim != anim || g == 0) {

		current_settings[FRC_ANIMATION_FRAMECOUNT] = 0;
		if(g != 0) {
			del_gif(g);
			g = 0;
		}
		int ret = 0;

		if(anim > 0 && anim-offset-1 <= str.filesactive) {
			if(current_settings[FRC_CONSOLE_ENABLE])
			os_printf("anim: %d offset:%d  str.active: %d -%d \r\n", anim, offset,  str.filesactive, str.len[anim-offset-1]);

			char buf2[str.len[anim-offset-1]];
			fs_readfile(BEGIN_ADDRESS_GIF, buf2, anim-offset-1);

			if(current_settings[FRC_CONSOLE_ENABLE])
			os_printf("str 1: %d -%d   - %d \r\n", str.filesactive, str.len[anim-offset-1], system_get_free_heap_size());

			if(system_get_free_heap_size() > 10000) {
				g = read_gif_file(buf2,  str.len[anim-offset-1]);
			} else {
				os_printf("OUT OF MEMORY, NOT READING GIF %d \r\n, ", system_get_free_heap_size());
				websocket_close_all();
				websocket_abort_all();
				if(g != 0) {
					del_gif(g);
					g = 0;
				}
				return;
			}

			if(current_settings[FRC_CONSOLE_ENABLE])
			os_printf("str 2: %d -%d \r\n", str.filesactive, str.len[anim-offset-1]);

			for(int f = 0; f < 8; f++) {
				for(int z = 0; z < 16; z++) {
					buf[f][z][0] = 0;
				    buf[f][z][1] = 0;
				    buf[f][z][2] = 0;
				}
			}

		}
		curanim = anim;
		if( g->screen->cmap->length > 128) {
			current_settings[FRC_ANIMATION_FRAMECOUNT] = 0;
			os_printf("Too much colors %d  \r\n",g->screen->cmap->length );
			return;
		}
		for(int x = 0; x < g->screen->cmap->length; x++) {
			gifcolors[x] = g->screen->cmap->colours[x];
		}

	}

	if(current_settings[FRC_CONSOLE_ENABLE])
		os_printf("copy_next_frame 2: curanim: %d anim:%d offset:%d  g:%p i:%d \r\n",curanim, anim, offset, g, current_settings[FRC_ANIMATION_FRAMECOUNT]);


	int count = 0;
	int i = current_settings[FRC_ANIMATION_FRAMECOUNT] % g->block_count;

	if(i < 0)
		i = 0;

	if(i >= g->block_count) {
		//current_settings[FRC_ANIMATION_FRAMECOUNT] = 0;
		os_printf("GIF IS TOO BIG %d - %d \r\n", i, g->block_count);
		return;
	}

	if(g == 0 || g->blocks[i] == 0) {
		os_printf("Block == null \r\n");
		return;
	}
	if(current_settings[FRC_CONSOLE_ENABLE])
	os_printf("getting transparent color \r\n");

	while(g->blocks[i]->pic == 0) {
		if(g->blocks[i]->ext != 0 && g->blocks[i]->ext->marker == 0xF9) {
			trcolor = g->blocks[i]->ext->data[0]->bytes[3];
		}


		current_settings[FRC_ANIMATION_FRAMECOUNT]++;
		i = current_settings[FRC_ANIMATION_FRAMECOUNT] % g->block_count;

//		if(i >= g->block_count) {
//			i = 0;
//		}
	}
	//current_settings[FRC_ANIMATION_FRAMECOUNT] = i;

	if(current_settings[FRC_CONSOLE_ENABLE])
		os_printf("copy_next_frame 3: height:%d  width:%d curanim: %d anim:%d offset:%d  g:%p - %d\r\n", g->blocks[i]->pic->height, g->blocks[i]->pic->width, curanim, anim, offset, g, i);


	if (g->blocks[i]->pic->height > 8 || g->blocks[i]->pic->width > 16) {
		current_settings[FRC_ANIMATION_FRAMECOUNT]++;
		os_printf("GIF IS TOO BIG, height:%d width:%d \r\n", g->blocks[i]->pic->height, g->blocks[i]->pic->width);
		return;
	}


	for(int x = 0; x < g->blocks[i]->pic->height; x++) {
			for(int y = 0; y < g->blocks[i]->pic->width; y++) {
				if(g->blocks[i]->pic->data[x][y] != trcolor) {
					buf[x+g->blocks[i]->pic->top][y+g->blocks[i]->pic->left][0] =   gifcolors[g->blocks[i]->pic->data[x][y]].red;
					buf[x+g->blocks[i]->pic->top][y+g->blocks[i]->pic->left][1] =   gifcolors[g->blocks[i]->pic->data[x][y]].green;
					buf[x+g->blocks[i]->pic->top][y+g->blocks[i]->pic->left][2] =   gifcolors[g->blocks[i]->pic->data[x][y]].blue;
				}
			}
		}

	if(c % current_settings[FRC_ANIMATION_SPEED] == 0){
		current_settings[FRC_ANIMATION_FRAMECOUNT]++;
	}

//	if(current_settings[FRC_ANIMATION_FRAMECOUNT] >= g->block_count) {
//		current_settings[FRC_ANIMATION_FRAMECOUNT] = 0;
//	}

	if(current_settings[FRC_CONSOLE_ENABLE])
	os_printf("copy_next_frame done: \r\n",curanim, anim, offset, g);


}


