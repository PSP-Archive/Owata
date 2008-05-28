/*
* $Id$
*/

#ifndef __PG_H__
#define __PG_H__

#define BUF_WIDTH (512)
#define BUF_HEIGHT (512)
#define SCR_WIDTH (480)
#define SCR_HEIGHT (272)
#define FONT_HEIGHT (s2ch.font.height)
#define LINE_PITCH (s2ch.font.pitch)
#define DRAW_LINE_H (s2ch.font.lineH)
#define DRAW_LINE_V (s2ch.font.lineV)
#define FRAME_SIZE (BUF_WIDTH * SCR_HEIGHT)
#define ZBUF_SIZE (BUF_WIDTH * BUF_HEIGHT)
#define RGB(r,g,b) (0xF000|(((b>>4) & 0xF)<<8)|(((g>>4) & 0xF)<<4)|((r>>4) & 0xF))
//#define RGB(r,g,b) (0x8000|(((b>>3) & 0x1F)<<10)|(((g>>3) & 0x1F)<<5)|((r>>3) & 0x1F))
#define RGB8888(r,g,b,a) ((a)<<24|(b)<<16|(g)<<8|(r))
#define WHITE RGB(255,255,255)
#define BLACK RGB(0,0,0)
#define RED RGB(255,0,0)
#define GREEN RGB(0,255,0)
#define BLUE RGB(0,0,255)
#define YELLOW RGB(255,255,00)
#define MAGENTA RGB(255,0,255)
#define CYAN RGB(0,255,255)
#define GRAY RGB(0xCC,0xCC,0xCC)

#include "psp2ch.h"

struct Vertex
{
	unsigned short u, v;
	unsigned short color;
	short x, y, z;
};

typedef struct rect_t
{
	short left;
	short top;
	short right;
	short bottom;
} RECT;

typedef struct tex
{
	int w;
	int h;
	int tb;
} TEX;

int pgExtraFontInit(void);
void pgSetupGu(void);
void pgCopyRect(void *src, TEX *tex, RECT *src_rect, RECT *dst_rect);
void pgCopyRectRotate(void *src, TEX *tex, RECT *src_rect, RECT *dst_rect);
void pgTermGu(void);
void pgFontLoad(void);
void pgWaitV();
void pgWaitVn(unsigned long count);
unsigned short* pgGetVramAddr(unsigned long x,unsigned long y, int w);
void pgFillvram(unsigned short color, int x1, int y1, int w, int h, int wide);
void pgFillRect(unsigned long color, RECT *rect);
void pgPrintTitleBar(char* ita, char* title);
void pgCopyTitleBar(void);
void pgPrintMenuBar(char* str);
void pgCopyMenuBar(void);
void pgEditBox(int color, int x1, int y1, int x2, int y2);
void pgWindowFrame(int x1, int y1, int x2, int y2);
void pgScrollbar(S_SCROLLBAR bar, S_2CH_BAR_COLOR c);
void pgPadCursor(int x, int y);
void pgCopyWindow(int offset, int x, int y, int w, int h);
void pgCopy(int offsetX, int offsetY);
void pgPrintNumber(int num, int color,int bgcolor);
int pgPutCharA(const unsigned char c,unsigned short color,unsigned short bgcolor, int width);
int pgPutCharW(unsigned char hi,unsigned char lo,unsigned short color,unsigned short bgcolor, int width);
int pgPutCharW2(unsigned char hi,unsigned char lo,unsigned short color,unsigned short bgcolor, int width, int code);
int pgSpecialChars(char** string,unsigned short color,unsigned short bgcolor, int width);
char* pgPrint(char *str, unsigned short color, unsigned short bgcolor, int width);
char* pgPrintHtml(char *str,S_2CH_RES_COLOR *c, int startX, int width,int drawLine);
int pgCountCharA(const unsigned char c, int width);
int pgCountCharW(unsigned char hi,unsigned char lo, int width);
int pgCountCharW2(unsigned char hi,unsigned char lo, int width, int code);
int pgCountSpecialChars(char** string, int width);
char* pgCountHtml(char *str, int width, int specialchar);
void pgHome(void);
void pgPrintMona(void);
void pgPrintMonaWait(void);
void pgPrintOwata(void);
void pgPrintTate(void);

#endif
