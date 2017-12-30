#ifndef _FRAMEDRIVER_H
#define _FRAMEDRIVER_H

#ifndef ICACHE_FLASH_ATTR
#define ICACHE_FLASH_ATTR
#define ICACHE_RODATA_ATTR
#endif /* ICACHE_FLASH */

#ifndef COLUMNS // can be set via command line
#define COLUMNS 16
#endif

#ifndef BUILDVERSION // can be set via command line
#define BUILDVERSION 1
#endif




#ifndef LEDTYPE // can be set via command line
#define LEDTYPE "SHIRT"
#endif

#define totleds (8*COLUMNS*3)
#define ROWS 8
#define TEXTROWS 25
#define TEXTHEIGHT 8
#define BUFFERS 1

#define COLORS 3

enum framedriver_typecommand {
	FRC_SET = 0x01,
	FRC_UPDATE = 0x02,
	FRC_GET = 0x03
};

enum framedriver_command {
    FRC_NONE =0x00,
	FRC_ANIMATION_SPEED	= 	0x01	,
	FRC_FADE_SPEED	= 	0x02	,
	FRC_FADE_ENABLE	= 	0x03	,
	FRC_FLICKER_ENABLE	= 	0x04	,
	FRC_FLICKER_SPEED	= 	0x05	,
	FRC_TEXT_SPEED	= 	0x06	,
	FRC_GIF_VALUE	= 	0x07	,
	FRC_LCD_ENABLE	= 	0x08	,
	FRC_FLASHLIGHT_ENABLE	= 	0x09	,
	FRC_CONSOLE_ENABLE	= 	0x0a	,
	FRC_SLIDESHOW_ENABLE	= 	0x0b	,
	FRC_BRIGHTNESS_VALUE	= 	0x0c	,
	FRC_SLIDESHOW_SPEED	= 	0x0d	,
	FRC_SYNC_ALL	= 	0x0e	,
	FRC_SLIDESHOW_CHOISE	= 	0x0f	,
	FRC_TEXT_BIG	= 	0x10	,
	FRC_TEXT_SCROLL_MULTIPLE	= 	0x11	,
	FRC_AP_USER	= 	0x12	,
	FRC_AP_PASSWORD	= 	0x13	,
	FRC_CONNECT_CLIENT_SAVE	= 	0x14	,
	FRC_TEXT_COLOR	= 	0x15	,
	FRC_COLUMNS	= 	0x16	,
	FRC_BUILD_NR	= 	0x17	,
	FRC_BUILD_TYPE	= 	0x18	,
	FRC_CLIENT_USER	= 	0x19	,
	FRC_CLIENT_PASSWORD	= 	0x1a	,
	FRC_TEXT_WIDE_DATA	= 	0x1b	,
	FRC_TEXT_DATA	= 	0x1c	,
	FRC_ANIMATION_OFFSET	= 	0x1d	,
	FRC_ANIMATION_COUNTER	= 	0x1e	,
	FRC_FRAME_COUNTER	= 	0x1f	,
	FRC_ANIMATION_FRAMECOUNT	= 	0x20	,
	FRC_TEXT_OFFSET	= 	0x21	,
	FRC_BATTERY_VALUE	= 	0x22	,
	FRC_IP_VALUE	= 	0x23	,
	FRC_CONNECT_CLIENT_NOW	= 	0x24	,
	FRC_OPEN_DATA = 0x25,
    FRC_INVALID=0x26
};



static int ICACHE_RODATA_ATTR GammaE[256] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2,
					2, 2, 2, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5,
					6, 6, 6, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11,
					11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18,
					19, 19, 20, 21, 21, 22, 22, 23, 23, 24, 25, 25, 26, 27, 27, 28,
					29, 29, 30, 31, 31, 32, 33, 34, 34, 35, 36, 37, 37, 38, 39, 40,
					40, 41, 42, 43, 44, 45, 46, 46, 47, 48, 49, 50, 51, 52, 53, 54,
					55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70,
					71, 72, 73, 74, 76, 77, 78, 79, 80, 81, 83, 84, 85, 86, 88, 89,
					90, 91, 93, 94, 95, 96, 98, 99,100,102,103,104,106,107,109,110,
					111,113,114,116,117,119,120,121,123,124,126,128,129,131,132,134,
					135,137,138,140,142,143,145,146,148,150,151,153,155,157,158,160,
					162,163,165,167,169,170,172,174,176,178,179,181,183,185,187,189,
					191,193,194,196,198,200,202,204,206,208,210,212,214,216,218,220,
					222,224,227,229,231,233,235,237,239,241,244,246,248,250,252,255};

static int ledlookuphoriz[128] = {	0,	8,	16,	24,	32,	40,	48,	56,
						64,	72,	80,	88,	96,	104,112,113,
						105,97,	89,	81,	73,	65,	57,	49,
						41,	33,	25,	17,	9,	1,	2,	10,
						18,	26,	34,	42,	50,	58,	66,	74,
						82,	90,	98,	106,114,115,107,99,
						91,	83,	75,	67,	59,	51,	43,	35,
						27,	19,	11,	3,	4,	12,	20,	28,
						36,	44,	52,	60,	68,	76,	84,	92,
						100,108,116,117,109,101,93,	85,
						77,	69,	61,	53,	45,	37,	29,	21,
						13,	5,	6,	14,	22,	30,	38,	46,
						54,	62,	70,	78,	86,	94,	102,110,
						118,119,111,103,95,	87,	79,	71,
						63,	55,	47,	39,	31,	23,	15,	7,
						63,	55,	47,	39,	31,	23,	15,	7};

static int ledlookuphorizhat[128] = {	7,	6,	5,	4,	3, 2,	1,	0,
										8,	9,	10,	11,	12,	13, 14 ,15,
										23,	22,	21,	20,	19,	18,	17,	16,
										24, 25, 26, 27, 28, 29, 30, 31,
										39,	38, 37, 36, 35, 34, 33, 32,
										40,	41,	42,	43,	44,	45,	46,	47,
									    55,	54,	53,	52,	51,	50,	49,	48,
										56,	57, 58,	59,	60,	61,	62,	63,

										71,	70,	69,	68,	67,	66,	65,	64,
										72,	73,	74,	75,	76,	77,	78,	79,
										87,	86,	85,	84,	83,	82,	81,	80,
										88,	89,	90,	91,	92,	93,	94,	95,
										103,	102,	101,	100,	99,	98,	97,	96,
										104,	105,	106,	107,	108,	109,	110,	111,
										119,	118,	117,	116,	115,	114,	113,	112,
										120,	121,	122,	123,	124,	125,	126,	127 };



static int ledlookuphorizsmall[128] = {	0,	8,	16,	24,	32,	40,	48,	56,
										57,	49,	41,	33,	25,	17, 9 ,1,
										2,	10,	18,	26,	34,	42,	50,	58,
										59, 51, 43, 35, 27, 19, 11, 3,
										4,	12, 20, 28, 36, 44, 52, 60,
										61,	53,	45,	37,	29,	21,	13,	5,
										6,	14,	22,	30,	38,	46,	54,	62,
										63,	55,	47,	39,	31,	23,	15,	7,

										64,	72,	80,	88,	96,	104,	112,	120,
										121,	113,	105,	97,	89,	81,	73,	65,
										66,	74,	82,	90,	98,	106,	114,	122,
										123,	115,	107,	99,	91,	83,	75,	67,
										68,	76,	84,	92,	100,	108,	116,	124,
										125,	117,	109,	101,	93,	85,	77,	69,
										70,	78,	86,	94,	102,	110,	118,	126,
										127,	119,	111,	103,	95,	87,	79,	71 };

#endif
