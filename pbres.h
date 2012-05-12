/*
 * Yury P. Fedorchenko 2010
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <zlib.h>

#define MAXSIZE 520000
#define MAXIMGLIST 4096
#define PBTSIGNATURE "PocketBookTheme"
#define PBTVERSION 1

typedef struct ibitmap_s {

	unsigned short width;
	unsigned short height;
	unsigned short depth;
	unsigned short scanline;
	unsigned char data[];

} ibitmap;

struct imgcache {

	int pos;
	int len;
	unsigned char *data;

};
#ifdef __cplusplus
extern "C" {
#endif
void warning(const char *fmt, ...);
void terminate(const char *fmt, ...);
void usage(void);
void write_as_c(FILE *f, unsigned char *data, int len);
ibitmap *bmp2res(const char *fname, unsigned char *data, int len, int *outsize);
ibitmap *xpm2res(const char *fname, unsigned char *data, int len, int *outsize);
char *getname(const char *s);
#ifdef __cplusplus
};
#endif
