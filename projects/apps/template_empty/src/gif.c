/*
 *  Cross platform GIF source code.
 *
 *  Platform: Neutral
 *
 *  Version: 2.30  1997/07/07  Original version by Lachlan Patrick.
 *  Version: 2.35  1998/09/09  Minor upgrade to list functions.
 *  Version: 2.50  2000/01/01  Added the ability to load an animated gif.
 *  Version: 3.00  2001/03/03  Fixed a few bugs and updated the interface.
 *  Version: 3.34  2002/12/18  Debugging code is now better encapsulated.
 *  Version: 3.56  2005/08/09  Silenced a size_t conversion warning.
 */

/* Copyright (c) L. Patrick

   This file is part of the App cross-platform programming package.
   You may redistribute it and/or modify it under the terms of the
   App Software License. See the file LICENSE.TXT for details.
*/

/*
 *  Gif.c - Cross-platform code for loading and saving GIFs
 *
 *  The LZW encoder and decoder used in this file were
 *  written by Gershon Elber and Eric S. Raymond as part of
 *  the GifLib package.
 *
 *  The remainder of the code was written by Lachlan Patrick
 *  as part of the GraphApp cross-platform graphics library.
 *
 *  GIF(sm) is a service mark property of CompuServe Inc.
 *  For better compression and more features than GIF,
 *  use PNG: the Portable Network Graphics format.
 */

/*
 *  Copyright and patent information:
 *
 *  Because the LZW algorithm has been patented by
 *  CompuServe Inc, you probably can't use this file
 *  in a commercial application without first paying
 *  CompuServe the appropriate licensing fee.
 *  Contact CompuServe for more information about that.
 */

/*
 *  Known problems with this code:
 *
 *  There is really only one thing to watch out for:
 *  on a PC running a 16-bit operating system, such
 *  as Windows 95 or Windows 3.1, there is a 64K limit
 *  to the size of memory blocks. This may limit the
 *  size of GIF files you can load, perhaps to less
 *  than 256 pixels x 256 pixels. The new row pointer
 *  technique used in this version of this file should
 *  remove that limitation, but you should test this
 *  on your system before believing me.
 */

#include "include/gif.h"

#include "espmissingincludes.h"
#include <osapi.h>
#include "lwip/mem.h"
#include <string.h>
#include <ctype.h>


void* ICACHE_FLASH_ATTR app_alloc(unsigned long Size) {
	return os_malloc(Size);
}

void* ICACHE_FLASH_ATTR app_realloc(void* pMem,unsigned long Size) {
	return os_realloc(pMem, Size);
}

void ICACHE_FLASH_ATTR app_free(void* pMem) {
	os_free(pMem);
}


void ICACHE_FLASH_ATTR app_memcpy(void* pDst,void* pSrc,unsigned long Size) {
	os_memcpy(pDst, pSrc, Size);
}

void * ICACHE_FLASH_ATTR app_zero_alloc(unsigned long Size) {
	return os_zalloc(Size);
}




/*
 *  GIF file input/output functions.
 */

static unsigned char ICACHE_FLASH_ATTR read_byte(FILE *file)
{
	char d =  file->data[file->intpos];
	os_printf("Getting data from pos: %d, data: %d, len:%d \r\n", file->intpos, file->data[file->intpos], file->len);
	if(file->intpos + 1 < file->len) {
		file->intpos++;
	}
	return d;
//	int ch = getc(file);
//	if (ch == EOF)
//		ch = 0;
//	return ch;
}


static int ICACHE_FLASH_ATTR read_stream(FILE *file, unsigned char buffer[], int length)
{
//	os_memcpy(buffer, file, length);
//	//int count = (int) fread(buffer, 1, length, file);
	int count = 0;
//	int i = count;
//	while (i < length)
//		buffer[i++] = '\0';
	for(int i = 0; i < length; i++) {
		//os_printf("Getting data from pos: %d, data: %d, len:%d \r\n", file->intpos, file->data[file->intpos], file->len);
		buffer[i] = file->data[file->intpos];
		if(file->intpos + 1 < file->len) {
			file->intpos++;
			count++;
		} else {
			return 0;
		}
	}
	buffer[count] = '\0';
	return count;
}

uint16 ICACHE_FLASH_ATTR read_gif_int(FILE *file)
{
	uint16 output = 0;
	unsigned char buf[2];
//	if (fread(buf, 1, 2, file) != 2)
//		return 0;
	output = (((unsigned int) file->data[file->intpos+1]) << 8) | file->data[file->intpos];
	if(file->intpos + 2 < file->len) {
		file->intpos += 2;
	}
	return output;
}







/*
 *  GIF memory allocation helper functions.
 */

void * ICACHE_FLASH_ATTR gif_alloc(long bytes)
{
	return app_zero_alloc(bytes);
}



/*
 *  Gif data blocks:
 */

GifData * ICACHE_FLASH_ATTR new_gif_data(int size)
{
	GifData *data = gif_alloc(sizeof(GifData));
	if (data) {
		data->byte_count = size;
		data->bytes = app_zero_alloc(size * sizeof(unsigned char));
	}
	return data;
}

void ICACHE_FLASH_ATTR del_gif_data(GifData *data)
{
	app_free(data->bytes);
	app_free(data);
}

/*
 *  Read one code block from the Gif file.
 *  This routine should be called until NULL is returned.
 *  Use app_free() to free the returned array of bytes.
 */
GifData * ICACHE_FLASH_ATTR read_gif_data(FILE *file)
{
	GifData *data;
	int size;

	size = read_byte(file);

	if (size > 0) {
		data = new_gif_data(size);
		read_stream(file, data->bytes, size);
	}
	else {
		data = NULL;
	}


	return data;
}



#ifdef GIF_DEBUG
void ICACHE_FLASH_ATTR print_gif_data(FILE *file, GifData *data)
{
	int i, ch, prev;
	int ch_printable, prev_printable;

	if (data) {
		os_printf( "(length=%d) [", data->byte_count);
		prev_printable = 1;
		for (i=0; i < data->byte_count; i++) {
			ch = data->bytes[i];
			ch_printable = isprint(ch) ? 1 : 0;

			if (ch_printable != prev_printable)
				os_printf( " ");

			if (ch_printable)
				os_printf( "%c", (char)ch);
			else
				os_printf( "%02X,", ch);

			prev = ch;
			prev_printable = isprint(prev) ? 1 : 0;
		}
		os_printf( "]\n");
	}
	else {
		os_printf( "[]\n");
	}
}
#endif

/*
 *  Read the next byte from a Gif file.
 *
 *  This function is aware of the block-nature of Gif files,
 *  and will automatically skip to the next block to find
 *  a new byte to read, or return 0 if there is no next block.
 */
static ICACHE_FLASH_ATTR unsigned char read_gif_byte(FILE *file, GifDecoder *decoder)
{
	unsigned char *buf = decoder->buf;
	unsigned char next;

	if (decoder->file_state == IMAGE_COMPLETE)
		return '\0';

	if (decoder->position == decoder->bufsize)
	{	/* internal buffer now empty! */
		/* read the block size */
		decoder->bufsize = read_byte(file);
		if (decoder->bufsize == 0) {
			decoder->file_state = IMAGE_COMPLETE;
			return '\0';
		}
		read_stream(file, buf, decoder->bufsize);
		next = buf[0];
		decoder->position = 1;	/* where to get chars */
	}
	else {
		next = buf[decoder->position++];
	}

	return next;
}

/*
 *  Read to end of an image, including the zero block.
 */
static ICACHE_FLASH_ATTR void finish_gif_picture(FILE *file, GifDecoder *decoder)
{
	unsigned char *buf = decoder->buf;

	while (decoder->bufsize != 0) {
		decoder->bufsize = read_byte(file);
		if (decoder->bufsize == 0) {
			decoder->file_state = IMAGE_COMPLETE;
			break;
		}
		read_stream(file, buf, decoder->bufsize);
	}
}


/*
 *  Colour maps:
 */

Colour app_new_rgb(int r, int g, int b)
{
	Colour col;

	col.alpha = 0;
	col.red = r;
	col.green = g;
	col.blue = b;

	return col;
}
#define rgb(r,g,b)             app_new_rgb((r),(g),(b))


GifPalette ICACHE_FLASH_ATTR * new_gif_palette(void)
{
	return gif_alloc(sizeof(GifPalette));
}

void ICACHE_FLASH_ATTR del_gif_palette(GifPalette *cmap)
{
	app_free(cmap->colours);
	app_free(cmap);
}

Colour	app_new_rgb(int r, int g, int b);


void ICACHE_FLASH_ATTR read_gif_palette(FILE *file, GifPalette *cmap)
{
	int i;
	unsigned char r, g, b;

	cmap->colours = app_alloc(cmap->length * sizeof(Colour));

	for (i=0; i<cmap->length; i++) {
		r = read_byte(file);
		g = read_byte(file);
		b = read_byte(file);

		cmap->colours[i] = rgb(r,g,b);
	}

}



#ifdef GIF_DEBUG
void ICACHE_FLASH_ATTR print_gif_palette(FILE *file, GifPalette *cmap)
{
	int i;

	os_printf("  GifPalette (length=%d):\n", cmap->length);
	for (i=0; i<cmap->length; i++) {
		os_printf( "   %02X = 0x", i);
		os_printf( "%02X", cmap->colours[i].red);
		os_printf( "%02X", cmap->colours[i].green);
		os_printf( "%02X\n", cmap->colours[i].blue);
	}
}
#endif

/*
 *  GifScreen:
 */

GifScreen * ICACHE_FLASH_ATTR new_gif_screen(void)
{
	GifScreen *screen = gif_alloc(sizeof(GifScreen));
	if (screen)
		screen->cmap = new_gif_palette();
	return screen;
}

void ICACHE_FLASH_ATTR del_gif_screen(GifScreen *screen)
{
	del_gif_palette(screen->cmap);
	app_free(screen);
}

void ICACHE_FLASH_ATTR read_gif_screen(FILE *file, GifScreen *screen)
{
	unsigned char info;

	screen->width       = read_gif_int(file);
	screen->height      = read_gif_int(file);

	info                = read_byte(file);
	screen->has_cmap    =  (info & 0x80) >> 7;
	screen->color_res   = ((info & 0x70) >> 4) + 1;
	screen->sorted      =  (info & 0x08) >> 3;
	screen->cmap_depth  =  (info & 0x07)       + 1;

	screen->bgcolour    = read_byte(file);
	screen->aspect      = read_byte(file);


	if (screen->has_cmap) {
		screen->cmap->length = 1 << screen->cmap_depth;
		read_gif_palette(file, screen->cmap);
	}
}


#ifdef GIF_DEBUG
void ICACHE_FLASH_ATTR print_gif_screen(FILE *file, GifScreen *screen)
{
	os_printf( " GifScreen:\n");
	os_printf( "  width      = %d\n", screen->width);
	os_printf( "  height     = %d\n", screen->height);

	os_printf( "  has_cmap   = %d\n", screen->has_cmap ? 1:0);
	os_printf( "  color_res  = %d\n", screen->color_res);
	os_printf( "  sorted     = %d\n", screen->sorted ? 1:0);
	os_printf( "  cmap_depth = %d\n", screen->cmap_depth);

	os_printf( "  bgcolour   = %02X\n", screen->bgcolour);
	os_printf( "  aspect     = %d\n", screen->aspect);

	if (screen->has_cmap) {
		print_gif_palette(file, screen->cmap);
	}
}
#endif

/*
 *  GifExtension:
 */

GifExtension * ICACHE_FLASH_ATTR new_gif_extension(void)
{
	return gif_alloc(sizeof(GifExtension));
}

void ICACHE_FLASH_ATTR del_gif_extension(GifExtension *ext)
{
	int i;

	for (i=0; i < ext->data_count; i++)
		del_gif_data(ext->data[i]);
	app_free(ext->data);
	app_free(ext);
}

void ICACHE_FLASH_ATTR read_gif_extension(FILE *file, GifExtension *ext)
{
	GifData *data;
	int i;
	ext->marker = read_byte(file);
	data = read_gif_data(file);

	while (data) {
		/* Append the data object: */
		i = ++ext->data_count;
		ext->data = app_realloc(ext->data, i * sizeof(GifData *));
		ext->data[i-1] = data;
		data = read_gif_data(file);
	}
}


#ifdef GIF_DEBUG
void ICACHE_FLASH_ATTR print_gif_extension(FILE *file, GifExtension *ext)
{
	int i;

	os_printf( " GifExtension:\n");
	os_printf( "  marker = 0x%02X\n", ext->marker);
	for (i=0; i < ext->data_count; i++) {
		os_printf( "  data = ");
		print_gif_data(file, ext->data[i]);
	}
}
#endif

/*
 *  GifDecoder:
 */

GifDecoder * ICACHE_FLASH_ATTR new_gif_decoder(void)
{
	return gif_alloc(sizeof(GifDecoder));
}

void ICACHE_FLASH_ATTR del_gif_decoder(GifDecoder *decoder)
{
	app_free(decoder);
}

void ICACHE_FLASH_ATTR init_gif_decoder(FILE *file, GifDecoder *decoder)
{
	int i, depth;
	int lzw_min;
	unsigned int *prefix;

	lzw_min = read_byte(file);
	depth = lzw_min;

	decoder->file_state   = IMAGE_LOADING;
	decoder->position     = 0;
	decoder->bufsize      = 0;
	decoder->buf[0]       = 0;
	decoder->depth        = depth;
	decoder->clear_code   = (1 << depth);
	decoder->eof_code     = decoder->clear_code + 1;
	decoder->running_code = decoder->eof_code + 1;
	decoder->running_bits = depth + 1;
	decoder->max_code_plus_one = 1 << decoder->running_bits;
	decoder->stack_ptr    = 0;
	decoder->prev_code    = NO_SUCH_CODE;
	decoder->shift_state  = 0;
	decoder->shift_data   = 0;

	prefix = decoder->prefix;
	for (i = 0; i <= LZ_MAX_CODE; i++)
		prefix[i] = NO_SUCH_CODE;
}

/*
 *  Read the next Gif code word from the file.
 *
 *  This function looks in the decoder to find out how many
 *  bits to read, and uses a buffer in the decoder to remember
 *  bits from the last byte input.
 */
int ICACHE_FLASH_ATTR read_gif_code(FILE *file, GifDecoder *decoder)
{
	int code;
	unsigned char next_byte;
	static int code_masks[] = {
		0x0000, 0x0001, 0x0003, 0x0007,
		0x000f, 0x001f, 0x003f, 0x007f,
		0x00ff, 0x01ff, 0x03ff, 0x07ff,
		0x0fff
	};

	while (decoder->shift_state < decoder->running_bits)
	{
		/* Need more bytes from input file for next code: */
		next_byte = read_gif_byte(file, decoder);
		decoder->shift_data |=
		  ((unsigned long) next_byte) << decoder->shift_state;
		decoder->shift_state += 8;
	}

	code = decoder->shift_data & code_masks[decoder->running_bits];

	decoder->shift_data >>= decoder->running_bits;
	decoder->shift_state -= decoder->running_bits;

	/* If code cannot fit into running_bits bits,
	 * we must raise its size.
	 * Note: codes above 4095 are used for signalling. */
	if (++decoder->running_code > decoder->max_code_plus_one
		&& decoder->running_bits < LZ_BITS)
	{
		decoder->max_code_plus_one <<= 1;
		decoder->running_bits++;
	}
	return code;
}

/*
 *  Routine to trace the prefix-linked-list until we get
 *  a prefix which is a pixel value (less than clear_code).
 *  Returns that pixel value.
 *
 *  If the picture is defective, we might loop here forever,
 *  so we limit the loops to the maximum possible if the
 *  picture is okay, i.e. LZ_MAX_CODE times.
 */
static int ICACHE_FLASH_ATTR trace_prefix(unsigned int *prefix, int code, int clear_code)
{
	int i = 0;

	while (code > clear_code && i++ <= LZ_MAX_CODE)
		code = prefix[code];
	return code;
}

/*
 *  The LZ decompression routine:
 *  Call this function once per scanline to fill in a picture.
 */
void ICACHE_FLASH_ATTR read_gif_line(FILE *file, GifDecoder *decoder,
			unsigned char *line, int length)
{
    int i = 0, j;
    int current_code, eof_code, clear_code;
    int current_prefix, prev_code, stack_ptr;
    unsigned char *stack, *suffix;
    unsigned int *prefix;

    prefix	= decoder->prefix;
    suffix	= decoder->suffix;
    stack	= decoder->stack;
    stack_ptr	= decoder->stack_ptr;
    eof_code	= decoder->eof_code;
    clear_code	= decoder->clear_code;
    prev_code	= decoder->prev_code;

    if (stack_ptr != 0) {
	/* Pop the stack */
	while (stack_ptr != 0 && i < length)
		line[i++] = stack[--stack_ptr];
    }

    while (i < length)
    {
	current_code = read_gif_code(file, decoder);

	if (current_code == eof_code)
	{
	   /* unexpected EOF */
	   if (i != length - 1 || decoder->pixel_count != 0)
		return;
	   i++;
	}
	else if (current_code == clear_code)
	{
	    /* reset prefix table etc */
	    for (j = 0; j <= LZ_MAX_CODE; j++)
		prefix[j] = NO_SUCH_CODE;
	    decoder->running_code = decoder->eof_code + 1;
	    decoder->running_bits = decoder->depth + 1;
	    decoder->max_code_plus_one = 1 << decoder->running_bits;
	    prev_code = decoder->prev_code = NO_SUCH_CODE;
	}
	else {
	    /* Regular code - if in pixel range
	     * simply add it to output pixel stream,
	     * otherwise trace code-linked-list until
	     * the prefix is in pixel range. */
	    if (current_code < clear_code) {
		/* Simple case. */
		line[i++] = current_code;
	    }
	    else {
		/* This code needs to be traced:
		 * trace the linked list until the prefix is a
		 * pixel, while pushing the suffix pixels on
		 * to the stack. If finished, pop the stack
		 * to output the pixel values. */
		if ((current_code < 0) || (current_code > LZ_MAX_CODE))
			return; /* image defect */
		if (prefix[current_code] == NO_SUCH_CODE) {
		    /* Only allowed if current_code is exactly
		     * the running code:
		     * In that case current_code = XXXCode,
		     * current_code or the prefix code is the
		     * last code and the suffix char is
		     * exactly the prefix of last code! */
		    if (current_code == decoder->running_code - 2) {
			current_prefix = prev_code;
			suffix[decoder->running_code - 2]
			    = stack[stack_ptr++]
			    = trace_prefix(prefix, prev_code, clear_code);
		    }
		    else {
			return; /* image defect */
		    }
		}
		else
		    current_prefix = current_code;

		/* Now (if picture is okay) we should get
		 * no NO_SUCH_CODE during the trace.
		 * As we might loop forever (if picture defect)
		 * we count the number of loops we trace and
		 * stop if we get LZ_MAX_CODE.
		 * Obviously we cannot loop more than that. */
		j = 0;
		while (j++ <= LZ_MAX_CODE
			&& current_prefix > clear_code
			&& current_prefix <= LZ_MAX_CODE)
		{
		    stack[stack_ptr++] = suffix[current_prefix];
		    current_prefix = prefix[current_prefix];
		}
		if (j >= LZ_MAX_CODE || current_prefix > LZ_MAX_CODE)
		    return; /* image defect */

		/* Push the last character on stack: */
		stack[stack_ptr++] = current_prefix;

		/* Now pop the entire stack into output: */
		while (stack_ptr != 0 && i < length)
		    line[i++] = stack[--stack_ptr];
	    }
	    if (prev_code != NO_SUCH_CODE) {
		if ((decoder->running_code < 2) ||
		  (decoder->running_code > LZ_MAX_CODE+2))
			return; /* image defect */
		prefix[decoder->running_code - 2] = prev_code;

		if (current_code == decoder->running_code - 2) {
		    /* Only allowed if current_code is exactly
		     * the running code:
		     * In that case current_code = XXXCode,
		     * current_code or the prefix code is the
		     * last code and the suffix char is
		     * exactly the prefix of the last code! */
		    suffix[decoder->running_code - 2]
			= trace_prefix(prefix, prev_code, clear_code);
		}
		else {
		    suffix[decoder->running_code - 2]
			= trace_prefix(prefix, current_code, clear_code);
		}
	    }
	    prev_code = current_code;
	}
    }

    decoder->prev_code = prev_code;
    decoder->stack_ptr = stack_ptr;
}

/*
 *  Hash table:
 */

/*
 *  The 32 bits contain two parts: the key & code:
 *  The code is 12 bits since the algorithm is limited to 12 bits
 *  The key is a 12 bit prefix code + 8 bit new char = 20 bits.
 */
#define HT_GET_KEY(x)	((x) >> 12)
#define HT_GET_CODE(x)	((x) & 0x0FFF)
#define HT_PUT_KEY(x)	((x) << 12)
#define HT_PUT_CODE(x)	((x) & 0x0FFF)

/*
 *  Generate a hash key from the given unique key.
 *  The given key is assumed to be 20 bits as follows:
 *    lower 8 bits are the new postfix character,
 *    the upper 12 bits are the prefix code.
 */
static int ICACHE_FLASH_ATTR gif_hash_key(unsigned long key)
{
	return ((key >> 12) ^ key) & HT_KEY_MASK;
}

/*
 *  Clear the hash_table to an empty state.
 */
static void ICACHE_FLASH_ATTR clear_gif_hash_table(unsigned long *hash_table)
{
	int i;
	for (i=0; i<HT_SIZE; i++)
		hash_table[i] = 0xFFFFFFFFL;
}

/*
 *  Insert a new item into the hash_table.
 *  The data is assumed to be new.
 */
static void ICACHE_FLASH_ATTR add_gif_hash_entry(unsigned long *hash_table, unsigned long key, int code)
{
	int hkey = gif_hash_key(key);

	while (HT_GET_KEY(hash_table[hkey]) != 0xFFFFFL) {
		hkey = (hkey + 1) & HT_KEY_MASK;
	}
	hash_table[hkey] = HT_PUT_KEY(key) | HT_PUT_CODE(code);
}

/*
 *  Determine if given key exists in hash_table and if so
 *  returns its code, otherwise returns -1.
 */
static int ICACHE_FLASH_ATTR lookup_gif_hash(unsigned long *hash_table, unsigned long key)
{
	int hkey = gif_hash_key(key);
	unsigned long htkey;

	while ((htkey = HT_GET_KEY(hash_table[hkey])) != 0xFFFFFL) {
		if (key == htkey)
			return HT_GET_CODE(hash_table[hkey]);
		hkey = (hkey + 1) & HT_KEY_MASK;
	}
	return -1;
}

/*
 *  GifEncoder:
 */

GifEncoder * ICACHE_FLASH_ATTR new_gif_encoder(void)
{
	return gif_alloc(sizeof(GifEncoder));
}

void ICACHE_FLASH_ATTR del_gif_encoder(GifEncoder *encoder)
{
	app_free(encoder);
}



/*
 *  GifPicture:
 */

GifPicture * ICACHE_FLASH_ATTR new_gif_picture(void)
{
	GifPicture *pic = gif_alloc(sizeof(GifPicture));
	if (pic) {
		pic->cmap = new_gif_palette();
		pic->data = NULL;
	}
	return pic;
}

void ICACHE_FLASH_ATTR del_gif_picture(GifPicture *pic)
{
	int row;

	del_gif_palette(pic->cmap);
	if (pic->data) {
		for (row=0; row < pic->height; row++)
			app_free(pic->data[row]);
		app_free(pic->data);
	}
	app_free(pic);
}

static void ICACHE_FLASH_ATTR read_gif_picture_data(FILE *file, GifPicture *pic)
{
	GifDecoder *decoder;
	long w, h;
	int interlace_start[] = {0, 4, 2, 1};
	int interlace_step[]  = {8, 8, 4, 2};
	int scan_pass, row;

	w = pic->width;
	h = pic->height;
	pic->data = app_alloc(h * sizeof(unsigned char *));
	if (pic->data == NULL)
		return;
	for (row=0; row < h; row++)
		pic->data[row] = app_zero_alloc(w * sizeof(unsigned char));

	decoder = new_gif_decoder();
	init_gif_decoder(file, decoder);

	if (pic->interlace) {
	  for (scan_pass = 0; scan_pass < 4; scan_pass++) {
	    row = interlace_start[scan_pass];
	    while (row < h) {
	      read_gif_line(file, decoder, pic->data[row], w);
	      row += interlace_step[scan_pass];
	    }
	  }
	}
	else {
	  row = 0;
	  while (row < h) {
	    read_gif_line(file, decoder, pic->data[row], w);
	    row += 1;
	  }
	}
	finish_gif_picture(file, decoder);

	del_gif_decoder(decoder);
}

void ICACHE_FLASH_ATTR read_gif_picture(FILE *file, GifPicture *pic)
{
	unsigned char info;

	pic->left   = read_gif_int(file);
	pic->top    = read_gif_int(file);
	pic->width  = read_gif_int(file);
	pic->height = read_gif_int(file);

	info = read_byte(file);
	pic->has_cmap    = (info & 0x80) >> 7;
	pic->interlace   = (info & 0x40) >> 6;
	pic->sorted      = (info & 0x20) >> 5;
	pic->reserved    = (info & 0x18) >> 3;

	if (pic->has_cmap) {
		pic->cmap_depth  = (info & 0x07) + 1;
		pic->cmap->length = 1 << pic->cmap_depth;
		read_gif_palette(file, pic->cmap);
	}

	read_gif_picture_data(file, pic);
}


#ifdef GIF_DEBUG
static void ICACHE_FLASH_ATTR print_gif_picture_data(FILE *file, GifPicture *pic)
{
	int pixval, row, col;

	for (row = 0; row < pic->height; row++) {
		os_printf( "   [");
	  for (col = 0; col < pic->width; col++) {
	    pixval = pic->data[row][col];
	    os_printf( "%02X", pixval);
	  }
	  os_printf( "]\n");
	}
}

static void ICACHE_FLASH_ATTR print_gif_picture_header(FILE *file, GifPicture *pic)
{
	os_printf( " GifPicture:\n");
	os_printf( "  left       = %d\n", pic->left);
	os_printf( "  top        = %d\n", pic->top);
	os_printf( "  width      = %d\n", pic->width);
	os_printf( "  height     = %d\n", pic->height);

	os_printf( "  has_cmap   = %d\n", pic->has_cmap);
	os_printf( "  interlace  = %d\n", pic->interlace);
	os_printf( "  sorted     = %d\n", pic->sorted);
	os_printf( "  reserved   = %d\n", pic->reserved);
	os_printf( "  cmap_depth = %d\n", pic->cmap_depth);
}

void ICACHE_FLASH_ATTR print_gif_picture(FILE *file, GifPicture *pic)
{
	print_gif_picture_header(file, pic);

	if (pic->has_cmap)
		print_gif_palette(file, pic->cmap);

	print_gif_picture_data(file, pic);
}
#endif

/*
 *  GifBlock:
 */

GifBlock * ICACHE_FLASH_ATTR new_gif_block(void)
{
	return gif_alloc(sizeof(GifBlock));
}

void ICACHE_FLASH_ATTR del_gif_block(GifBlock *block)
{
	if (block->pic) {
		del_gif_picture(block->pic);
	}
	if (block->ext) {
		del_gif_extension(block->ext);
	}
	app_free(block);
}

void ICACHE_FLASH_ATTR read_gif_block(FILE *file, GifBlock *block)
{

	block->intro = read_byte(file);

	if (block->intro == 0x2C) {
		block->pic = new_gif_picture();
		read_gif_picture(file, block->pic);
	}
	else if (block->intro == 0x21) {
		block->ext = new_gif_extension();
		read_gif_extension(file, block->ext);
	}
}



#ifdef GIF_DEBUG
void ICACHE_FLASH_ATTR print_gif_block(FILE *file, GifBlock *block)
{
	os_printf(file, " GifBlock (intro=0x%02X):\n", block->intro);
	if (block->pic)
		print_gif_picture(file, block->pic);
	if (block->ext)
		print_gif_extension(file, block->ext);
}
#endif

/*
 *  Gif:
 */

Gif * ICACHE_FLASH_ATTR new_gif(void)
{
	Gif *gif = gif_alloc(sizeof(Gif));
	if (gif) {
		strcpy(gif->header, "GIF87a");
		gif->screen = new_gif_screen();
		gif->blocks = NULL;
	}
	return gif;
}

void ICACHE_FLASH_ATTR del_gif(Gif *gif)
{
	int i;

	del_gif_screen(gif->screen);
	for (i=0; i < gif->block_count; i++) {
		del_gif_block(gif->blocks[i]);
	}
	os_free(gif->blocks);
	app_free(gif);
}

void ICACHE_FLASH_ATTR read_gif(FILE *file, Gif *gif)
{
	int i;
	GifBlock *block;
//
	for (i=0; i<6; i++) {
		gif->header[i] = read_byte(file);
	}

	if (strncmp(gif->header, "GIF", 3) != 0)
		return; /* error */

	read_gif_screen(file, gif->screen);

	while (1) {
		block = new_gif_block();
		read_gif_block(file, block);

		if (block->intro == 0x3B) {	/* terminator */
			del_gif_block(block);
			break;
		}
		else  if (block->intro == 0x2C) {	/* image */
			/* Append the block: */
			i = ++gif->block_count;
			gif->blocks = app_realloc(gif->blocks, i * sizeof(GifBlock *));
			gif->blocks[i-1] = block;
		}
		else  if (block->intro == 0x21) {	/* extension */
			/* Append the block: */
			i = ++gif->block_count;
			gif->blocks = app_realloc(gif->blocks, i * sizeof(GifBlock *));
			gif->blocks[i-1] = block;
		}
		else {	/* error */
			del_gif_block(block);
			break;
		}
	}
}

void ICACHE_FLASH_ATTR read_one_gif_picture(FILE *file, Gif *gif)
{
	int i;
	GifBlock *block;

	for (i=0; i<6; i++)
		gif->header[i] = read_byte(file);
	if (strncmp(gif->header, "GIF", 3) != 0)
		return; /* error */

	read_gif_screen(file, gif->screen);

	while (1) {
		block = new_gif_block();
		read_gif_block(file, block);

		if (block->intro == 0x3B) {	/* terminator */
			del_gif_block(block);
			break;
		}
		else if (block->intro == 0x2C) { /* image */
			/* Append the block: */
			i = ++gif->block_count;
			gif->blocks = app_realloc(gif->blocks, i * sizeof(GifBlock *));
			gif->blocks[i-1] = block;
			break;
		}
		else if (block->intro == 0x21) { /* extension */
			/* Append the block: */
			i = ++gif->block_count;
			gif->blocks = app_realloc(gif->blocks, i * sizeof(GifBlock *));
			gif->blocks[i-1] = block;
			continue;
		}
		else {	/* error! */
			del_gif_block(block);
			break;
		}
	}
}

#ifdef GIF_DEBUG
void ICACHE_FLASH_ATTR print_gif(FILE *file, Gif *gif)
{
	int i;
	os_printf("Gif header=%s - block count: %d\n", gif->header, gif->block_count);
	print_gif_screen(file, gif->screen);
	for (i=0; i < gif->block_count; i++) {
		print_gif_block(file, gif->blocks[i]);
	}
	os_printf("End of gif.\n\n");
}
#endif

/*
 *  Reading and Writing Gif files:
 */

Gif * read_gif_file(const char *data, int len)
{
	Gif *gif;
	FILE filee;
	filee.data = data;
	filee.intpos = 0;
	filee.len = len;
	gif = new_gif();
	read_gif(&filee, gif);
//	os_printf("GOOO \r\n");
//	FILE * file2  = app_open_file("log.txt", "rb");
	print_gif(&filee, gif);
//	app_close_file(file2);
//	app_close_file(file);
	if (strncmp(gif->header, "GIF", 3) != 0) {
		del_gif(gif);
		gif = NULL;
	}
	return gif;
}
