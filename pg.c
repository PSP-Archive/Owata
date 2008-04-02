/*
* $Id$
*/

#include <pspkernel.h>
#include <pspgu.h>
#include <pspdisplay.h>
#include <psppower.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <time.h>
#include "pg.h"
#include "intraFont.h"
#include "cp932.h"

unsigned int __attribute__((aligned(16))) list[512*512];
unsigned int __attribute__((aligned(16))) winPixels[BUF_WIDTH*BUF_HEIGHT*2];
unsigned int __attribute__((aligned(16))) pixels[BUF_WIDTH*BUF_HEIGHT*2];
unsigned int __attribute__((aligned(16))) barPixels[BUF_WIDTH*32*2];
unsigned int* printBuf;
void* framebuffer;
intraFont* jpn0;
static unsigned char *fontA, *fontJ;
static int size_fontA, size_fontJ;

extern S_2CH s2ch; // psp2chRes.c

const int cursorImgB[48] = {
        0,
        BUF_WIDTH,BUF_WIDTH+1,
        BUF_WIDTH*2,BUF_WIDTH*2+2,
        BUF_WIDTH*3,BUF_WIDTH*3+3,
        BUF_WIDTH*4,BUF_WIDTH*4+4,
        BUF_WIDTH*5,BUF_WIDTH*5+5,
        BUF_WIDTH*6,BUF_WIDTH*6+6,
        BUF_WIDTH*7,BUF_WIDTH*7+7,
        BUF_WIDTH*8,BUF_WIDTH*8+8,
        BUF_WIDTH*9,BUF_WIDTH*9+9,
        BUF_WIDTH*10,BUF_WIDTH*10+6,BUF_WIDTH*10+7,BUF_WIDTH*10+8,BUF_WIDTH*10+9,BUF_WIDTH*10+10,
        BUF_WIDTH*11,BUF_WIDTH*11+3,BUF_WIDTH*11+6,
        BUF_WIDTH*12,BUF_WIDTH*12+2,BUF_WIDTH*12+4,BUF_WIDTH*12+7,
        BUF_WIDTH*13,BUF_WIDTH*13+1,BUF_WIDTH*13+4,BUF_WIDTH*13+7,
        BUF_WIDTH*14,BUF_WIDTH*14+5,BUF_WIDTH*14+8,
        BUF_WIDTH*15+5,BUF_WIDTH*15+8,
        BUF_WIDTH*16+6,BUF_WIDTH*16+9,
        BUF_WIDTH*17+6,BUF_WIDTH*17+9,
        BUF_WIDTH*18+7,BUF_WIDTH*18+8,BUF_WIDTH*18+9};
const int cursorImgW[58] = {
        BUF_WIDTH*2+1,
        BUF_WIDTH*3+1,BUF_WIDTH*3+2,
        BUF_WIDTH*4+1,BUF_WIDTH*4+2,BUF_WIDTH*4+3,
        BUF_WIDTH*5+1,BUF_WIDTH*5+2,BUF_WIDTH*5+3,BUF_WIDTH*5+4,
        BUF_WIDTH*6+1,BUF_WIDTH*6+2,BUF_WIDTH*6+3,BUF_WIDTH*6+4,BUF_WIDTH*6+5,
        BUF_WIDTH*7+1,BUF_WIDTH*7+2,BUF_WIDTH*7+3,BUF_WIDTH*7+4,BUF_WIDTH*7+5,BUF_WIDTH*7+6,
        BUF_WIDTH*8+1,BUF_WIDTH*8+2,BUF_WIDTH*8+3,BUF_WIDTH*8+4,BUF_WIDTH*8+5,BUF_WIDTH*8+6,BUF_WIDTH*8+7,
        BUF_WIDTH*9+1,BUF_WIDTH*9+2,BUF_WIDTH*9+3,BUF_WIDTH*9+4,BUF_WIDTH*9+5,BUF_WIDTH*9+6,BUF_WIDTH*9+7,BUF_WIDTH*9+8,
        BUF_WIDTH*10+1,BUF_WIDTH*10+2,BUF_WIDTH*10+3,BUF_WIDTH*10+4,BUF_WIDTH*10+5,
        BUF_WIDTH*11+1,BUF_WIDTH*11+2,BUF_WIDTH*11+4,BUF_WIDTH*11+5,
        BUF_WIDTH*12+1,BUF_WIDTH*12+5,BUF_WIDTH*12+6,
        BUF_WIDTH*13+5,BUF_WIDTH*13+6,
        BUF_WIDTH*14+6,BUF_WIDTH*14+7,
        BUF_WIDTH*15+6,BUF_WIDTH*15+7,
        BUF_WIDTH*16+7,BUF_WIDTH*16+8,
        BUF_WIDTH*17+7,BUF_WIDTH*17+8};

const int cursorImgBV[48] = {
        18,
        17,BUF_WIDTH+17,
        16,BUF_WIDTH*2+16,
        15,BUF_WIDTH*3+15,
        14,BUF_WIDTH*4+14,
        13,BUF_WIDTH*5+13,
        12,BUF_WIDTH*6+12,
        11,BUF_WIDTH*7+11,
        10,BUF_WIDTH*8+10,
        9,BUF_WIDTH*9+9,
        8,BUF_WIDTH*6+8,BUF_WIDTH*7+8,BUF_WIDTH*8+8,BUF_WIDTH*9+8,BUF_WIDTH*10+8,
        7,BUF_WIDTH*3+7,BUF_WIDTH*6+7,
        6,BUF_WIDTH*2+6,BUF_WIDTH*4+6,BUF_WIDTH*7+6,
        5,BUF_WIDTH+5,BUF_WIDTH*4+5,BUF_WIDTH*7+5,
        4,BUF_WIDTH*5+4,BUF_WIDTH*8+4,
        BUF_WIDTH*5+3,BUF_WIDTH*8+3,
        BUF_WIDTH*6+2,BUF_WIDTH*9+2,
        BUF_WIDTH*6+1,BUF_WIDTH*9+1,
        BUF_WIDTH*7,BUF_WIDTH*8,BUF_WIDTH*9};
const int cursorImgWV[58] = {
        BUF_WIDTH*1+16,
        BUF_WIDTH*1+15,BUF_WIDTH*2+15,
        BUF_WIDTH*1+14,BUF_WIDTH*2+14,BUF_WIDTH*3+14,
        BUF_WIDTH*1+13,BUF_WIDTH*2+13,BUF_WIDTH*3+13,BUF_WIDTH*4+13,
        BUF_WIDTH*1+12,BUF_WIDTH*2+12,BUF_WIDTH*3+12,BUF_WIDTH*4+12,BUF_WIDTH*5+12,
        BUF_WIDTH*1+11,BUF_WIDTH*2+11,BUF_WIDTH*3+11,BUF_WIDTH*4+11,BUF_WIDTH*5+11,BUF_WIDTH*6+11,
        BUF_WIDTH*1+10,BUF_WIDTH*2+10,BUF_WIDTH*3+10,BUF_WIDTH*4+10,BUF_WIDTH*5+10,BUF_WIDTH*6+10,BUF_WIDTH*7+10,
        BUF_WIDTH*1+9,BUF_WIDTH*2+9,BUF_WIDTH*3+9,BUF_WIDTH*4+9,BUF_WIDTH*5+9,BUF_WIDTH*6+9,BUF_WIDTH*7+9,BUF_WIDTH*8+9,
        BUF_WIDTH*1+8,BUF_WIDTH*2+8,BUF_WIDTH*3+8,BUF_WIDTH*4+8,BUF_WIDTH*5+8,
        BUF_WIDTH*1+7,BUF_WIDTH*2+7,BUF_WIDTH*4+7,BUF_WIDTH*5+7,
        BUF_WIDTH*1+6,BUF_WIDTH*5+6,BUF_WIDTH*6+6,
        BUF_WIDTH*5+5,BUF_WIDTH*6+5,
        BUF_WIDTH*6+4,BUF_WIDTH*7+4,
        BUF_WIDTH*6+3,BUF_WIDTH*7+3,
        BUF_WIDTH*7+2,BUF_WIDTH*8+2,
        BUF_WIDTH*7+1,BUF_WIDTH*8+1};

struct entityTag
{
    char* str;
    int len;
    int byte;
    char c1;
    char c2;
};

struct Vertex
{
    unsigned short u, v;
    unsigned short color;
    short x, y, z;
};
#define MAX_ENTITIES 12
struct entityTag entity[MAX_ENTITIES];
void pgEntitiesSet(void)
{
    entity[0].str = "&amp;"; entity[0].len = 4;entity[0].byte = 1;entity[0].c1 = '&'; entity[0].c2 = 0;
    entity[1].str = "&gt;";  entity[1].len = 3;entity[1].byte = 1;entity[1].c1 = '>'; entity[1].c2 = 0;
    entity[2].str = "&lt;";  entity[2].len = 3;entity[2].byte = 1;entity[2].c1 = '<'; entity[2].c2 = 0;
    entity[3].str = "&quot;";entity[3].len = 5;entity[3].byte = 1;entity[3].c1 = '"'; entity[3].c2 = 0;
    entity[4].str = "&nbsp;";entity[4].len = 5;entity[4].byte = 1;entity[4].c1 = ' '; entity[4].c2 = 0;
    entity[5].str = "&rarr;";entity[5].len = 5;entity[5].byte = 2;entity[5].c1 = 0x81;entity[5].c2 = 0xA8;
    entity[6].str = "&larr;";entity[6].len = 5;entity[6].byte = 2;entity[6].c1 = 0x81;entity[6].c2 = 0xA9;
    entity[7].str = "&sub;"; entity[7].len = 4;entity[7].byte = 2;entity[7].c1 = 0x81;entity[7].c2 = 0xBC;
    entity[8].str = "&and;"; entity[8].len = 4;entity[8].byte = 2;entity[8].c1 = 0x81;entity[8].c2 = 0xC8;
    entity[9].str = "&or;";  entity[9].len = 3;entity[9].byte = 2;entity[9].c1 = 0x81;entity[9].c2 = 0xC9;
    entity[10].str = "&uarr;";entity[10].len = 5;entity[10].byte = 2;entity[10].c1 = 0x81;entity[10].c2 = 0xAA;
    entity[11].str = "&darr;";entity[11].len = 5;entity[11].byte = 2;entity[11].c1 = 0x81;entity[11].c2 = 0xAB;
}

int pgExtraFontInit(void)
{
    SceUID fd;
    SceIoStat st;
    char path[256];
    int ret;

    sprintf(path, "%s/%s/%s", s2ch.cwDir, s2ch.fontDir, s2ch.font.fileA);
    ret = sceIoGetstat(path, &st);
    if (ret < 0)
    {
        psp2chErrorDialog("Getstat error font file\n%s", path);
        return -1;
    }
    size_fontA = st.st_size;
    free(fontA);
    fontA = (unsigned char*)malloc(st.st_size);
    if (fontA == NULL)
    {
        psp2chErrorDialog("memorry error\nfontA");
        return -1;
    }
    fd = sceIoOpen(path, PSP_O_RDONLY, 0777);
    if (fd < 0)
    {
        psp2chErrorDialog("Open error font file\n%s", path);
        return -1;
    }
    else
    {
        sceIoRead(fd, fontA, st.st_size);
        sceIoClose(fd);
    }
    sprintf(path, "%s/%s/%s", s2ch.cwDir, s2ch.fontDir, s2ch.font.fileJ);
    ret = sceIoGetstat(path, &st);
    if (ret < 0)
    {
        psp2chErrorDialog("Getstat error font file\n%s", path);
        return -1;
    }
    size_fontJ = st.st_size;
    free(fontJ);
    fontJ = (unsigned char*)malloc(st.st_size);
    if (fontJ == NULL)
    {
        psp2chErrorDialog("memorry error\nfontJ");
        return -1;
    }
    fd = sceIoOpen(path, PSP_O_RDONLY, 0777);
    if (fd < 0)
    {
        psp2chErrorDialog("Open error font file\n%s", path);
        return -1;
    }
    else
    {
        sceIoRead(fd, fontJ, st.st_size);
        sceIoClose(fd);
    }
    return 0;
}

void pgSetupGu(void)
{
    printBuf = pixels;
    pgEntitiesSet();
    intraFontInit();
    sceGuInit();

    sceGuStart(GU_DIRECT,list);
    sceGuDrawBuffer(GU_PSM_8888,(void*)0,BUF_WIDTH);
    sceGuDispBuffer(SCR_WIDTH,SCR_HEIGHT,(void*)0x88000,BUF_WIDTH);
    sceGuDepthBuffer((void*)0x110000,BUF_WIDTH);
    sceGuOffset(2048 - (SCR_WIDTH/2), 2048 - (SCR_HEIGHT/2));
    sceGuViewport(2048, 2048, SCR_WIDTH, SCR_HEIGHT);
    sceGuDepthRange(65535, 0);
    sceGuScissor(0, 0, SCR_WIDTH, SCR_HEIGHT);
    sceGuEnable(GU_SCISSOR_TEST);
    sceGuEnable(GU_TEXTURE_2D);
    sceGuDepthFunc(GU_GEQUAL);
    sceGuEnable(GU_DEPTH_TEST);
    sceGuFrontFace(GU_CW);
    sceGuShadeModel(GU_SMOOTH);
    sceGuEnable(GU_CULL_FACE);
    sceGuEnable(GU_CLIP_PLANES);
    sceGuEnable(GU_BLEND);
    sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
    sceGuFinish();
    sceGuSync(0,0);

    sceDisplayWaitVblankStart();
    sceGuDisplay(GU_TRUE);
}

/*****************************
内蔵フォントをロード
*****************************/
void pgFontLoad(void)
{
    if (pgExtraFontInit() < 0)
    {
        s2ch.running = 0;
        sceKernelExitGame();
    }
    pgFillvram(WHITE, 0, 0, SCR_WIDTH, SCR_HEIGHT, 2);
    pgPrintMonaWait();
    pgCopy(0, 0);
    framebuffer = sceGuSwapBuffers();
    jpn0 = intraFontLoad("flash0:/font/jpn0.pgf",0);
}

void pgTermGu(void)
{
    intraFontUnload(jpn0);
    intraFontShutdown();
    sceGuTerm();
}

void pgWaitVn(unsigned long count)
{
    while (count--) {
        sceDisplayWaitVblankStart();
    }
}

void pgWaitV()
{
    sceDisplayWaitVblankStart();
}

/*****************************
現在の書き込みバッファにおける座標x,yのアドレスを返す
printBufを変更することで書き込みバッファを変えられます
*****************************/
unsigned int* pgGetVramAddr(unsigned long x,unsigned long y, int w)
{
    return printBuf + x + y * BUF_WIDTH * w;
}

/*****************************
左上座標x1,y1、幅、高さw,h、色colorで四角形を塗りつぶす
*****************************/
void pgFillvram(int color, int x1, int y1, int w, int h, int wide)
{
    unsigned int *vptr0;       //pointer to vram
    unsigned int *vptr;
    unsigned long i, j;

    vptr0 = pgGetVramAddr(0, y1 & 0x01FF, wide) + x1;
    for (i = 0; i < h;) {
        vptr = vptr0;;
        for (j = 0; j < w; j++) {
            *vptr++ = color;
        }
        vptr0 += BUF_WIDTH * wide;
        if (((++i + y1)&0x01FF) == 0) {
            vptr0 -= ZBUF_SIZE * wide;
        }
    }
}

/*****************************
タイトルバーを表示
VRAMのバッファに直接書き込みます。
*****************************/
void pgTitleBar(char* ita, char* title)
{
    unsigned int *temp;
    int x, y;
    unsigned int *src;
    unsigned int *dst, *dst0;
    time_t timep;
    struct tm *t;
    char date[16];
    char buf[32];

    sceKernelLibcTime(&timep);
    timep += 9 * 60 * 60;
    t = localtime(&timep);
    sprintf(date, "%02d:%02d:%02d", t->tm_hour, t->tm_min, t->tm_sec);
    sprintf(buf, " [%s]", ita);
    temp = printBuf;
    printBuf = barPixels;
    s2ch.pgCursorX = 0;
    if (s2ch.tateFlag)
    {
        pgFillvram(s2ch.formColor.title_bg, 0, 0, SCR_HEIGHT, FONT_HEIGHT + LINE_PITCH + 1, 2);
        s2ch.pgCursorY = 1;
        pgPrint(buf, s2ch.formColor.ita, s2ch.formColor.title_bg, SCR_HEIGHT);
        s2ch.pgCursorX += 8;
        title = pgPrint(title, s2ch.formColor.title, s2ch.formColor.title_bg, SCR_HEIGHT);
        s2ch.pgCursorY += LINE_PITCH;
        if (title)
        {
            s2ch.pgCursorX = 0;
            pgPrint(title, s2ch.formColor.title, s2ch.formColor.title_bg, SCR_HEIGHT);
        }
        s2ch.pgCursorX = SCR_HEIGHT - FONT_HEIGHT * 4 - 2;
        pgPrint(date, s2ch.formColor.title, s2ch.formColor.title_bg, SCR_HEIGHT);
    }
    else
    {
        pgFillvram(s2ch.formColor.title_bg, 0, 0, SCR_WIDTH, FONT_HEIGHT, 2);
        s2ch.pgCursorY = 0;
        pgPrint(buf, s2ch.formColor.ita, s2ch.formColor.title_bg, SCR_WIDTH);
        s2ch.pgCursorX += 8;
        pgPrint(title, s2ch.formColor.title, s2ch.formColor.title_bg, SCR_WIDTH);
        s2ch.pgCursorX = SCR_WIDTH - FONT_HEIGHT * 4 - 2;
        pgPrint(date, s2ch.formColor.title, s2ch.formColor.title_bg, SCR_WIDTH);
    }
    if (s2ch.tateFlag)
    {
        src = printBuf;
        dst0 = (unsigned int*)(0x04000000+framebuffer) + SCR_WIDTH - 1;
        for (y = 0; y < FONT_HEIGHT + LINE_PITCH + 1; y++)
        {
            dst = dst0--;
            for (x = 0; x < SCR_HEIGHT; x++)
            {
                *dst = *src++;
                dst += BUF_WIDTH;
            }
            src += (BUF_WIDTH * 2 - SCR_HEIGHT);
        }
    }
    else
    {
        src = printBuf;
        dst = (unsigned int*)(0x04000000+framebuffer);
        for (y = 0; y < FONT_HEIGHT; y++)
        {
            for (x = 0; x < SCR_WIDTH; x++)
            {
                *dst++ = *src++;
            }
            dst += (BUF_WIDTH - SCR_WIDTH);
            src += (BUF_WIDTH * 2 - SCR_WIDTH);
        }
    }
    printBuf = temp;
}

/*****************************
メニューバーを表示
VRAMのバッファに直接書き込みます。
*****************************/
void pgMenuBar(char* str)
{
    unsigned int *temp;
    int battery;
    int batteryColor;
    int x, y;
    unsigned int *src;
    unsigned int *dst, *dst0;

    temp = printBuf;
    printBuf = barPixels;
    s2ch.pgCursorX = 0;
    if (s2ch.tateFlag)
    {
        pgFillvram(s2ch.menuColor.bg, 0, 0, SCR_HEIGHT, FONT_HEIGHT + LINE_PITCH, 2);
        s2ch.pgCursorY = 0;
        str = pgPrint(str, s2ch.menuColor.text, s2ch.menuColor.bg, SCR_HEIGHT);
        s2ch.pgCursorY += LINE_PITCH;
        if (str)
        {
            s2ch.pgCursorX = 0;
            pgPrint(str, s2ch.menuColor.text, s2ch.menuColor.bg, SCR_WIDTH);
        }
        s2ch.pgCursorX = SCR_HEIGHT - FONT_HEIGHT * 3;
    }
    else
    {
        pgFillvram(s2ch.menuColor.bg, 0, 0, SCR_WIDTH, FONT_HEIGHT, 2);
        s2ch.pgCursorY = 0;
        pgPrint(str, s2ch.menuColor.text, s2ch.menuColor.bg, SCR_WIDTH);
        s2ch.pgCursorX = SCR_WIDTH - FONT_HEIGHT * 3;
    }
    battery = scePowerGetBatteryLifePercent();
    if (battery < 0)
    {
        pgPrint("  ---", s2ch.menuColor.bat1, s2ch.menuColor.bg, SCR_WIDTH);
    }
    else
    {
        if (battery > 40)
        {
            batteryColor = s2ch.menuColor.bat1;
        }
        else if (battery > 20)
        {
            batteryColor = s2ch.menuColor.bat2;
        }
        else
        {
            batteryColor = s2ch.menuColor.bat3;
        }
        pgPrintNumber(battery, batteryColor, s2ch.menuColor.bg);
        pgPrint("%", batteryColor, s2ch.menuColor.bg, SCR_WIDTH);
    }
    if (s2ch.tateFlag)
    {
        src = printBuf;
        dst0 = (unsigned int*)(0x04000000+framebuffer) + FONT_HEIGHT + LINE_PITCH - 1;
        for (y = 0; y < FONT_HEIGHT + LINE_PITCH; y++)
        {
            dst = dst0--;
            for (x = 0; x < SCR_HEIGHT; x++)
            {
                *dst = *src++;
                dst += BUF_WIDTH;
            }
            src += (BUF_WIDTH * 2 - SCR_HEIGHT);
        }
    }
    else
    {
        src = printBuf;
        dst = (unsigned int*)(0x04000000+framebuffer)+(SCR_HEIGHT - FONT_HEIGHT)*BUF_WIDTH;
        for (y = 0; y < FONT_HEIGHT; y++)
        {
            for (x = 0; x < SCR_WIDTH; x++)
            {
                *dst++ = *src++;
            }
            dst += (BUF_WIDTH - SCR_WIDTH);
            src += (BUF_WIDTH * 2 - SCR_WIDTH);
        }
    }
    printBuf = temp;
}

/*****************************
枠付で四角形を塗りつぶす
*****************************/
void pgEditBox(int color, int x1, int y1, int x2, int y2)
{
    unsigned int *vptr0;       //pointer to vram
    unsigned long i, j;

    if (x1 > x2)
    {
        return;
    }
    if (y1 > y2)
    {
        return;
    }
    x1 -= 2;
    y1 -= 2;
    x2++;
    y2++;
    if (x1 < 0) {
        x1 = 0;
    }
    if (y1 < 0) {
        y1 = 0;
    }
    if (x2 > SCR_WIDTH) {
        x2 = SCR_WIDTH;
    }
    if (y2 > SCR_HEIGHT) {
        y2 = SCR_HEIGHT;
    }
    vptr0 = pgGetVramAddr(0, y1, 2);
    for (j = x1; j < x2; j++) {
        vptr0[j] = RGB(0x99, 0x99, 0x99);
    }
    vptr0 += BUF_WIDTH;
    vptr0[x1+0] = RGB(0x99, 0x99, 0x99);
    for (j = x1+1; j < x2-1; j++) {
        vptr0[j] = RGB(0x33, 0x33, 0x33);
    }
    vptr0[x2-1] = RGB(0x99, 0x99, 0x99);
    for (i = y1+2; i < y2-1; i++) {
        vptr0 += BUF_WIDTH;
        vptr0[x1+0] = RGB(0x99, 0x99, 0x99);
        vptr0[x1+1] = RGB(0x33, 0x33, 0x33);
        for (j = x1+2; j < x2-1; j++) {
            vptr0[j] = color;
        }
        vptr0[x2-1] = RGB(0x99, 0x99, 0x99);
    }
    vptr0 += BUF_WIDTH;
    for (j = x1; j < x2; j++) {
        vptr0[j] = RGB(0x99, 0x99, 0x99);
    }
}

/*****************************
枠だけVRAMに直接表示
*****************************/
void pgWindowFrame(int x1, int y1, int x2, int y2)
{
    unsigned int *vptr0;       //pointer to vram
    unsigned long i, j;
    int tmp;

    if (s2ch.tateFlag)
    {
        tmp = x1;
        x1 = SCR_WIDTH - y2 + 1;
        y2 = x2;
        x2 = SCR_WIDTH - y1;
        y1 = tmp;
    }
    if (x1 > x2)
    {
        return;
    }
    if (y1 > y2)
    {
        return;
    }
    x1 -= 2;
    y1 -= 2;
    x2++;
    y2++;
    if (x1 < 0) {
        x1 = 0;
    }
    if (y1 < 0) {
        y1 = 0;
    }
    if (x2 > SCR_WIDTH) {
        x2 = SCR_WIDTH;
    }
    if (y2 > SCR_HEIGHT) {
        y2 = SCR_HEIGHT;
    }
    vptr0 = (unsigned int*)(framebuffer + 0x04000000) + y1 * BUF_WIDTH;
    for (j = x1; j < x2; j++) {
        vptr0[j] = RGB(0x99, 0x99, 0x99);
    }
    vptr0 += BUF_WIDTH;
    vptr0[x1+0] = RGB(0x99, 0x99, 0x99);
    for (j = x1+1; j < x2-1; j++) {
        vptr0[j] = RGB(0x33, 0x33, 0x33);
    }
    vptr0[x2-1] = RGB(0x99, 0x99, 0x99);
    for (i = y1+2; i < y2-1; i++) {
        vptr0 += BUF_WIDTH;
        vptr0[x1+0] = RGB(0x99, 0x99, 0x99);
        vptr0[x1+1] = RGB(0x33, 0x33, 0x33);
        vptr0[x2-1] = RGB(0x99, 0x99, 0x99);
    }
    vptr0 += BUF_WIDTH;
    for (j = x1; j < x2; j++) {
        vptr0[j] = RGB(0x99, 0x99, 0x99);
    }
}

/*****************************
スクロールバー表示
VRAMバッファに直接書き込み
*****************************/
void pgScrollbar(S_SCROLLBAR bar, S_2CH_BAR_COLOR c)
{
    int sliderH, sliderY;
    printBuf = framebuffer + 0x04000000;

    sliderH = bar.view*bar.h/bar.total;
    sliderY = bar.start*bar.h/bar.total+bar.y;
    if (sliderH < 2) {
        sliderH = 2;
    }
    if (sliderY >= (bar.y+bar.h-1)) {
        sliderY = bar.y+bar.h-2;
    }
    if (s2ch.tateFlag)
    {
        pgFillvram(c.bg, SCR_WIDTH - bar.y - bar.h, bar.x, bar.h, bar.w, 1);
        pgFillvram(c.slider, SCR_WIDTH - sliderY - sliderH, bar.x + 1, sliderH, bar.w - 1, 1);
    }
    else
    {
        pgFillvram(c.bg, bar.x, bar.y, bar.w, bar.h, 1);
        pgFillvram(c.slider, bar.x+1, sliderY, bar.w-1, sliderH, 1);
    }
    printBuf = pixels;
}

/*****************************
矢印カーソルをVRAMバッファに直接表示
*****************************/
void pgPadCursor(int x, int y)
{
    unsigned int *vptr;
    int i;

    if (s2ch.tateFlag)
    {
        vptr = (unsigned int*)(0x04000000+framebuffer) + x * BUF_WIDTH + SCR_WIDTH - y - 19;
        for (i = 0; i < 48; i++)
        {
            vptr[cursorImgBV[i]] = BLACK;
        }
        for (i = 0; i < 58; i++)
        {
            vptr[cursorImgWV[i]] = WHITE;
        }
    }
    else
    {
        vptr = (unsigned int*)(0x04000000+framebuffer) + x + y * BUF_WIDTH;
        for (i = 0; i < 48; i++)
        {
            vptr[cursorImgB[i]] = BLACK;
        }
        for (i = 0; i < 58; i++)
        {
            vptr[cursorImgW[i]] = WHITE;
        }
    }
}

/*****************************
現在の書き込みバッファをoffsetの位置からVRAMバッファへウィンドウ転送
*****************************/
void pgCopyWindow(int offset, int x, int y, int w, int h)
{
    int i, j;
    unsigned int *src;
    unsigned int *dst, *dst0;

    if (s2ch.tateFlag)
    {
        src = printBuf + ((offset + y) & 0x01FF) * BUF_WIDTH * 2 + x;
        dst0 = (unsigned int*)(0x04000000 + framebuffer) + SCR_WIDTH - 1;
        dst0 += x * BUF_WIDTH - (y & 0x01FF);
        for (i = 0; i < h; i++)
        {
            dst = dst0--;
            for (j = 0; j < w; j++)
            {
                *dst = *src++;
                dst += BUF_WIDTH;
            }
            src += (BUF_WIDTH * 2 - w);
            if (src >= printBuf + ZBUF_SIZE * 2) {
                src -= ZBUF_SIZE * 2;
            }
        }
    }
    else
    {
        src = printBuf + ((offset + y) & 0x01FF) * BUF_WIDTH * 2 + x;
        dst = (unsigned int*)(0x04000000 + framebuffer);
        dst += (y & 0x01FF) * BUF_WIDTH + x;
        for (i = 0; i < h; i++)
        {
            for (j = 0; j < w; j++)
            {
                *dst++ = *src++;
            }
            dst += (BUF_WIDTH - w);
            src += (BUF_WIDTH * 2 - w);
            if (src >= printBuf + ZBUF_SIZE * 2) {
                src -= ZBUF_SIZE * 2;
            }
        }
    }
}

/*****************************
現在の書き込みバッファをoffsetの位置からVRAMバッファに全画面転送
*****************************/
void pgCopy(int offsetX, int offsetY)
{
    int x, y;
    unsigned int *src;
    unsigned int *dst, *dst0;

    offsetY &= 0x01FF;
    if (s2ch.tateFlag)
    {
        src = printBuf + offsetX + (offsetY * BUF_WIDTH * 2);
        dst0 = (unsigned int*)(0x04000000+framebuffer) + SCR_WIDTH - 1;
        for (y = 0; y < SCR_WIDTH; y++)
        {
            dst = dst0--;
            for (x = 0; x < SCR_HEIGHT; x++)
            {
                *dst = *src++;
                dst += BUF_WIDTH;
            }
            src += (BUF_WIDTH * 2 - SCR_HEIGHT);
            if (src >= printBuf + ZBUF_SIZE * 2) {
                src -= ZBUF_SIZE * 2;
            }
        }
    }
    else
    {
        src = printBuf + offsetX + (offsetY * BUF_WIDTH * 2);
        dst = (unsigned int*)(0x04000000+framebuffer);
        for (y = 0; y < SCR_HEIGHT; y++)
        {
            for (x = 0; x < SCR_WIDTH; x++)
            {
                *dst++ = *src++;
            }
            dst += (BUF_WIDTH - SCR_WIDTH);
            src += (BUF_WIDTH * 2 - SCR_WIDTH);
            if (src >= printBuf + ZBUF_SIZE * 2) {
                src -= ZBUF_SIZE * 2;
            }
        }
    }
}

/*****************************
数字を表示
*****************************/
void pgPrintNumber(int num, int color,int bgcolor)
{
    unsigned int *vptr0, *vptr;
    char buf[16];
    int i, j, cx, cy, b, count;
    unsigned short* font;

    s2ch.pgCursorY &= 0x01FF;
    sprintf(buf, "%d", num);
    count = strlen(buf);
    // '0'の文字幅を使う
    font = (unsigned short*)(fontA + (('0' - 0x20) << 5));
    cx = *font;
    s2ch.pgCursorX += (4 - count) * cx;
    for (j = 0; j < count;j++) {
        font = (unsigned short*)(fontA + ((buf[j] - 0x20) << 5));
        cx = *font++;
        vptr0 = pgGetVramAddr(s2ch.pgCursorX, s2ch.pgCursorY, 2);
        s2ch.pgCursorX += cx;
        for (cy = 0; cy < FONT_HEIGHT; cy++) {
            vptr = vptr0;
            b = 0x8000;
            for (i = 0; i < cx; i++) {
                if ((*font & b)!=0) {
                    *vptr=color;
                } else {
                    *vptr=bgcolor;
                }
                vptr++;
                b >>= 1;
            }
            vptr0 += BUF_WIDTH * 2;
            if (vptr0 >= printBuf + ZBUF_SIZE * 2) {
                vptr0 -= ZBUF_SIZE * 2;
            }
            font++;
        }
    }
}

/*****************************
1文字をフォントから読み込んでwidth内で表示可能なら表示して0を返す
widthを超える場合は表示しないで1を返す
*****************************/
int pgPutChar(unsigned char *cfont,int ch,int color,int bgcolor, int width)
{
    unsigned int *vptr0;       //pointer to vram
    unsigned int *vptr;        //pointer to vram
    unsigned short *font;
    int cx, cy, i, b;

    font = (unsigned short*)(cfont + (ch<<5));
    cx = *font++;
    if ((s2ch.pgCursorX + cx) >= width) {
        return 1;
    }
    s2ch.pgCursorY &= 0x01FF;
    vptr0 = pgGetVramAddr(s2ch.pgCursorX, s2ch.pgCursorY, 2);
    s2ch.pgCursorX += cx;
    for (cy = 0; cy < FONT_HEIGHT; cy++) {
        vptr = vptr0;
        b = 0x8000;
        for (i = 0; i < cx; i++) {
            if (*font & b) {
                *vptr=color;
            } else {
                *vptr=bgcolor;
            }
            vptr++;
            b >>= 1;
        }
        vptr0 += BUF_WIDTH * 2;
        if (vptr0 >= printBuf + ZBUF_SIZE * 2) {
            vptr0 -= ZBUF_SIZE * 2;
        }
        font++;
    }
    return 0;
}

int pgPutCharA(const unsigned char c,int color,int bgcolor, int width)
{
    unsigned long index;

    index = c;
    if (c < 0x20) {
        return 0;
    } else if (c < 0x80) {
        index -= 0x20;
    } else if (c > 0xa0) {
        index -= 0x41;
    } else {
        return 0;
    }
    if ((index << 5) >= size_fontA) {
        index = '?' - 0x20;
    }
    return pgPutChar(fontA, index, color, bgcolor, width);
}

int pgPutCharW(unsigned char hi,unsigned char lo,int color,int bgcolor, int width)
{
    unsigned long index;

    // sjis2jis
    hi -= (hi <= 0x9f) ? 0x71 : 0xb1;
    hi <<= 1;
    hi++;
    if (lo > 0x7f)
        lo--;
    if (lo >= 0x9e) {
        lo -= 0x7d;
        hi++;
    }
    else {
        lo -= 0x1f;
    }
    // hi : 0x21-0x7e, lo : 0x21-0x7e
    hi -= 0x21;
    lo -= 0x21;
    index = hi * (0x7e - 0x20);
    index += lo;
    if ((index << 5) >= size_fontJ) {
        index = 8; // '？'
    }
    return pgPutChar(fontJ, index, color, bgcolor, width);
}

int pgPutCharW2(unsigned char hi,unsigned char lo,int color,int bgcolor, int width, int code)
{
    unsigned long index;

    switch (code)
    {
    // sjis2jis
    case 0:
        hi -= (hi <= 0x9f) ? 0x71 : 0xb1;
        hi <<= 1;
        hi++;
        if (lo > 0x7f)
            lo--;
        if (lo >= 0x9e) {
            lo -= 0x7d;
            hi++;
        }
        else {
            lo -= 0x1f;
        }
        break;
    // euc2jis
    case 1:
        if (hi == 0x8E)
        {
            return pgPutCharA(lo, color, bgcolor, width);
        }
        hi &= 0x7F;
        lo &= 0x7F;
        break;
    // jis
    case 2:
        break;
    }
    // hi : 0x21-0x7e, lo : 0x21-0x7e
    hi -= 0x21;
    lo -= 0x21;
    index = hi * (0x7e - 0x20);
    index += lo;
    if ((index << 5) >= size_fontJ) {
        index = 8; // '？'
    }
    return pgPutChar(fontJ, index, color, bgcolor, width);
}

/*****************************
実体参照を変換
*****************************/
int pgSpecialChars(char** string,int color,int bgcolor, int width)
{
    int i, val;
    char* str;
    unsigned char hi, lo;

    str = *string + 1;
    // Unicode変換
    if (*str == '#') {
        str++;
        val = 0;
        for (i = 0; i < 6; i++) {
            if (*str == ';') {
                break;
            }
            else if (*str < '0' || *str > '9') {
                if (val) {
                    str--;
                }
                break;
            }
            else {
                val *= 10;
                val += *str - '0';
            }
            str++;
        }
        if (val) {
            *string = str;
            val = conv_wchar_sjiswin(val);
            lo = val >> 8;
            hi = val & 0xFF;
            if (lo) {
                return pgPutCharW(hi, lo, color, bgcolor, width);
            }
            else if (hi) {
                return pgPutCharA(hi, color, bgcolor, width);
            }
            // 変換できない文字
            else {
                return pgPutCharA('?', color, bgcolor, width);
            }
        }
    }
    else {
        for (i = 0; i < MAX_ENTITIES; i++) {
            if (strstr(*string, entity[i].str) == *string) {
                (*string) += entity[i].len;
                if (entity[i].byte == 1) {
                    return pgPutCharA(entity[i].c1, color, bgcolor, width);
                }
                else {
                    return pgPutCharW(entity[i].c1, entity[i].c2, color, bgcolor, width);
                }
            }
        }
    }
    return pgPutCharA('&', color, bgcolor, width);
}

/*****************************
文字列strを画面幅widthで1行分表示して改行部分のポインタを返す
strを全部表示したらNULLを返す
*****************************/
char* pgPrint(char *str,int color,int bgcolor, int width)
{
    unsigned char ch = 0,bef = 0;
    int ret = 0;

    while(*str) {
        ch = (unsigned char)*str;
        if (bef!=0) {
            ret = pgPutCharW(bef, ch, color, bgcolor, width);
            if (ret) { // 改行部の位置を返す
                return --str;
            }
            bef=0;
        } else {
            if (((ch>=0x80) && (ch<0xa0)) || (ch>=0xe0)) {
                bef = ch;
            } else {
                if (ch == '&') {
                    ret = pgSpecialChars((char**)(&str), color, bgcolor, width);
                }
                else if (ch == '\n') {
                    ret = 1;
                    str++;
                }
                else {
                    ret = pgPutCharA(ch, color, bgcolor, width);
                }
                if (ret) {
                    return str;
                }
                bef = 0;
            }
        }
        str++;
    }
    return NULL;
}

void resEnd(void)
{
    s2ch.resAnchor[s2ch.resAnchorCount].x2 = s2ch.pgCursorX + 6;
    s2ch.resAnchorCount++;
    if (s2ch.resAnchorCount >= 50)
    {
        s2ch.resAnchorCount = 0;
    }
    s2ch.resAnchor[s2ch.resAnchorCount].x1 = 0;
}

void urlEnd(void)
{
    s2ch.urlAnchor[s2ch.urlAnchorCount].x2 = s2ch.pgCursorX + 6;
    s2ch.urlAnchorCount++;
    if (s2ch.urlAnchorCount >= 50)
    {
        s2ch.urlAnchorCount = 0;
    }
    s2ch.urlAnchor[s2ch.urlAnchorCount].x1 = 0;
}

/*****************************
文字列strを画面幅widthで1行分表示して改行部分のポインタを返す
strを全部表示したらNULLを返す
<br>で改行
他のHTMLタグは削除
レスアンカーやURLアンカーがあれば位置情報を保存
*****************************/
char* pgPrintHtml(char *str, S_2CH_RES_COLOR c, int startX, int width,int drawLine)
{
    static int anchorOn = 0;
    unsigned char ch = 0,bef = 0;
    int ret = 0;
    int tcolor;
    char *p;
    int i, j, start, end;

    if (anchorOn)
    {
        tcolor = c.link;
    }
    else
    {
        tcolor = c.text;
    }
    while(*str) {
        ch = (unsigned char)*str;
        if (bef!=0) {
            ret = pgPutCharW(bef, ch, tcolor, c.bg, width);
            if (ret) {
                return --str;
            }
            bef=0;
        } else {
            if (((ch>=0x80) && (ch<0xa0)) || (ch>=0xe0)) {
                bef = ch;
                if (anchorOn == 2) {
                    resEnd();
                }
                else if (anchorOn == 1) {
                    urlEnd();
                }
                anchorOn = 0;
                tcolor = c.text;
            } else {
                if (ch >= 0xa0 && ch < 0xe0) {
                    anchorOn = 0;
                    tcolor = c.text;
                }
                else if (anchorOn == 2) {
                    if ((ch < '0' || ch > '9') && ch != '-' && ch != ',' && ch !='<') {
                        anchorOn = 0;
                        tcolor = c.text;
                        resEnd();
                    }
                }
                else if (anchorOn == 1) {
                    if ((ch >= '#' && ch <= '+') || (ch >= '-' && ch <= ';') || (ch >= '?' && ch <= 'Z') ||
                            (ch >= 'a' && ch <= 'z') || ch == '!' || ch == '=' || ch == '_' || ch == '~') {
                    }
                    else {
                        anchorOn = 0;
                        tcolor = c.text;
                        urlEnd();
                    }
                }

                if (strstr(str, "http://") == str) {
                    s2ch.urlAnchor[s2ch.urlAnchorCount].x1 = s2ch.pgCursorX;
                    s2ch.urlAnchor[s2ch.urlAnchorCount].line = drawLine;
                    tcolor = c.link;
                }
                else if (strstr(str, "ttp://") == str) {
                    if (s2ch.urlAnchor[s2ch.urlAnchorCount].x1 == 0) {
                        s2ch.urlAnchor[s2ch.urlAnchorCount].x1 = s2ch.pgCursorX;
                        s2ch.urlAnchor[s2ch.urlAnchorCount].line = drawLine;
                    }
                    p = str + 6;
                    j = 0;
                    while ((*p >= '#' && *p <= '+') || (*p >= '-' && *p <= ';') || (*p >= '?' && *p <= 'Z') ||
                            (*p >= 'a' && *p <= 'z') || *p == '!' || *p == '=' || *p == '_' || *p == '~') {
                        if (*p == '/') {
                            p++;
                            break;
                        }
                        s2ch.urlAnchor[s2ch.urlAnchorCount].host[j] = *p++;
                        j++;
                        if (j >= 63) {
                            break;
                        }
                    }
                    if (j < 64) {
                        s2ch.urlAnchor[s2ch.urlAnchorCount].host[j] = '\0';
                        j = 0;
                        while ((*p >= '#' && *p <= '+') || (*p >= '-' && *p <= ';') || (*p >= '?' && *p <= 'Z') ||
                                (*p >= 'a' && *p <= 'z') || *p == '!' || *p == '=' || *p == '_' || *p == '~') {
                            s2ch.urlAnchor[s2ch.urlAnchorCount].path[j] = *p++;
                            j++;
                            if (j >= 255) {
                                break;
                            }
                        }
                        if (j < 256) {
                            s2ch.urlAnchor[s2ch.urlAnchorCount].path[j] = '\0';
                        }
                        anchorOn = 1;
                        tcolor = c.link;
                    }
                }

                if (ch == '<') {
                    if (anchorOn == 2) {
                        resEnd();
                    }
                    else if (anchorOn == 1) {
                        urlEnd();
                    }
                    anchorOn = 0;
                    tcolor = c.text;
                    if (strstr(str, "<br>") == str) {
                        str +=4;
                        return str;
                    }
                    else {
                        str = strchr(str, '>');
                        str++;
                        continue;
                    }
                }
                else if (ch == '&') {
                    if (strstr(str, "&gt;") == str) {
                        if ((strstr((str + 4), "&gt;") == (str + 4)) && *(str + 8) >= '0' && *(str + 8) <= '9') {
                            s2ch.resAnchor[s2ch.resAnchorCount].x1 = s2ch.pgCursorX;
                            s2ch.resAnchor[s2ch.resAnchorCount].line = drawLine;
                            tcolor = c.link;
                        }
                        else if (*(str + 4) >= '0' && *(str + 4) <= '9') {
                            if (s2ch.resAnchor[s2ch.resAnchorCount].x1 == 0) {
                                s2ch.resAnchor[s2ch.resAnchorCount].x1 = s2ch.pgCursorX;
                                s2ch.resAnchor[s2ch.resAnchorCount].line = drawLine;
                            }
                            j = 0;
                            p = str + 4;
                            while ((*p >= '0' && *p <= '9') || *p == '-' || *p == ',' || *p == '<') {
                                start = 0;
                                while (*p >= '0' && *p <= '9') {
                                    start = start * 10 + *p - '0';
                                    p++;
                                }
                                if (start <= s2ch.res.count && start > 0) {
                                    s2ch.resAnchor[s2ch.resAnchorCount].res[j] = start - 1;
                                    j++;
                                }
                                if (*p == '-') {
                                    end = 0;
                                    p++;
                                    while (*p >= '0' && *p <= '9') {
                                        end = end * 10 + *p - '0';
                                        p++;
                                    }
                                    for (i = start; i < end; i++) {
                                        if (i >= s2ch.res.count) {
                                            break;
                                        }
                                        if (i > 1) {
                                            s2ch.resAnchor[s2ch.resAnchorCount].res[j] = i;
                                            j++;
                                        }
                                    }
                                }
                                if (*p == '<') {
                                    p = strchr(p, '>');
                                    p++;
                                }
                                if (*p != ',') {
                                    break;
                                }
                                p++;
                            }
                            s2ch.resAnchor[s2ch.resAnchorCount].resCount = j;
                            anchorOn = 2;
                            tcolor = c.link;
                        }
                    }
                    ret = pgSpecialChars((char**)(&str), tcolor, c.bg, width);
                }
                else if (ch == ' ' || ch == '\n') {
                    while (ch == ' ' || ch == '\n') {
                        ch = *(++str);
                    }
                    anchorOn = 0;
                    tcolor = c.text;
                    str--;
                    ret = pgPutCharA(' ', tcolor, c.bg, width);
                }
                else {
                    ret = pgPutCharA(ch, tcolor, c.bg, width);
                }
                if (ret) {
                    if (anchorOn == 2) {
                        resEnd();
                        for (i = 0; i < s2ch.resAnchor[s2ch.resAnchorCount-1].resCount; i++) {
                            s2ch.resAnchor[s2ch.resAnchorCount].res[i] = s2ch.resAnchor[s2ch.resAnchorCount-1].res[i];
                        }
                        s2ch.resAnchor[s2ch.resAnchorCount].resCount = s2ch.resAnchor[s2ch.resAnchorCount-1].resCount;
                        s2ch.resAnchor[s2ch.resAnchorCount].line = drawLine+1;
                        s2ch.resAnchor[s2ch.resAnchorCount].x1 = startX;
                    }
                    else if (anchorOn == 1) {
                        urlEnd();
                        s2ch.urlAnchor[s2ch.urlAnchorCount].line = drawLine+1;
                        s2ch.urlAnchor[s2ch.urlAnchorCount].x1 = startX;
                        strcpy(s2ch.urlAnchor[s2ch.urlAnchorCount].host, s2ch.urlAnchor[s2ch.urlAnchorCount-1].host);
                        strcpy(s2ch.urlAnchor[s2ch.urlAnchorCount].path, s2ch.urlAnchor[s2ch.urlAnchorCount-1].path);
                    }
                    return str;
                }
            }
        }
        str++;
    }
    return NULL;
}

int pgCountCharA(const unsigned char c, int width)
{
    unsigned long index;
    unsigned short *font;

    index = c;
    if (c < 0x20) {
        return 0;
    } else if (c < 0x80) {
        index -= 0x20;
    } else if (c > 0xa0) {
        index -= 0x41;
    } else {
        return 0;
    }
    if ((index<<5) >= size_fontA) {
        index = '?' - 0x20;
    }
    font = (unsigned short*)(fontA + (index<<5));
    if ((s2ch.pgCursorX + *font) >= width) {
        return 1;
    }
    s2ch.pgCursorX += *font;
    return 0;
}

int pgCountCharW(unsigned char hi,unsigned char lo, int width)
{
    unsigned long index;
    unsigned short *font;

    // sjis2jis
    hi -= (hi <= 0x9f) ? 0x71 : 0xb1;
    hi <<= 1;
    hi++;
    if (lo > 0x7f)
        lo--;
    if (lo >= 0x9e) {
        lo -= 0x7d;
        hi++;
    }
    else {
        lo -= 0x1f;
    }
    // hi : 0x21-0x7e, lo : 0x21-0x7e
    hi -= 0x21;
    lo -= 0x21;
    index = hi * (0x7e - 0x20);
    index += lo;
    if ((index<<5) >= size_fontJ) {
        index = 8; // '？'
    }
    font = (unsigned short*)(fontJ + (index<<5));
    if ((s2ch.pgCursorX + *font) >= width) {
        return 1;
    }
    s2ch.pgCursorX += *font;
    return 0;
}

int pgCountCharW2(unsigned char hi,unsigned char lo, int width, int code)
{
    unsigned long index;
    unsigned short *font;

    switch (code)
    {
    // sjis2jis
    case 0:
        hi -= (hi <= 0x9f) ? 0x71 : 0xb1;
        hi <<= 1;
        hi++;
        if (lo > 0x7f)
            lo--;
        if (lo >= 0x9e) {
            lo -= 0x7d;
            hi++;
        }
        else {
            lo -= 0x1f;
        }
        break;
    // euc2jis
    case 1:
        if (hi == 0x8E)
        {
            return pgCountCharA(lo, width);
        }
        hi &= 0x7F;
        lo &= 0x7F;
        break;
    // utf82jis
    case 2:
        break;
    }
    // hi : 0x21-0x7e, lo : 0x21-0x7e
    hi -= 0x21;
    lo -= 0x21;
    index = hi * (0x7e - 0x20);
    index += lo;
    if ((index<<5) >= size_fontJ) {
        index = 8; // '？'
    }
    font = (unsigned short*)(fontJ + (index<<5));
    if ((s2ch.pgCursorX + *font) >= width) {
        return 1;
    }
    s2ch.pgCursorX += *font;
    return 0;
}

int pgCountSpecialChars(char** string, int width)
{
    int i, val;
    char* str;
    unsigned char hi, lo;

    str = *string + 1;
    if (*str == '#') {
        str++;
        val = 0;
        for (i = 0; i < 6; i++) {
            if (*str == ';') {
                break;
            }
            else if (*str < '0' || *str > '9') {
                if (val) {
                    str--;
                }
                break;
            }
            else {
                val *= 10;
                val += *str - '0';
            }
            str++;
        }
        if (val) {
            *string = str;
            val = conv_wchar_sjiswin(val);
            lo = val >> 8;
            hi = val & 0xFF;
            if (lo) {
                return pgCountCharW(hi, lo, width);
            }
            else if (hi) {
                return pgCountCharA(hi, width);
            }
            else {
                return pgCountCharA('?', width);
            }
        }
    }
    else {
        for (i = 0; i < MAX_ENTITIES; i++) {
            if (strstr(*string, entity[i].str) == *string) {
                (*string) += entity[i].len;
                if (entity[i].byte == 1) {
                    return pgCountCharA(entity[i].c1, width);
                }
                else {
                    return pgCountCharW(entity[i].c1, entity[i].c2, width);
                }
            }
        }
    }
    return pgCountCharA('&', width);
}

/*****************************
strを画面幅widthで表示したときの行数を数えるのに使う関数
表示は行われない
*****************************/
char* pgCountHtml(char *str, int width, int specialchar)
{
    unsigned char ch = 0,bef = 0;
    int ret = 0;

    while(*str) {
        ch = (unsigned char)*str;
        if (bef!=0) {
            ret = pgCountCharW(bef, ch, width);
            if (ret) {
                return --str;
            }
            bef=0;
        } else {
            if (((ch>=0x80) && (ch<0xa0)) || (ch>=0xe0)) {
                bef = ch;
            } else {
                if (ch == '<') {
                    if (strstr(str, "<br>") == str) {
                        str +=4;
                        return str;
                    }
                    else {
                        str = strchr(str, '>');
                        str++;
                        continue;
                    }
                }
                else if (ch == '&') {
                    ret = pgCountSpecialChars((char**)(&str), width);
                }
                else if (ch == ' ') {
                    while (ch == ' ') {
                        ch = *(++str);
                    }
                    str--;
                    ret = pgCountCharA(' ', width);
                }
                else {
                    ret = pgCountCharA(ch, width);
                }
                if (ret) {
                    return str;
                }
            }
        }
        str++;
    }
    return NULL;
}

void pgHome(void)
{
    s2ch.pgCursorX = 0;
    s2ch.pgCursorY = 0;
}

void pgPrintMona(void)
{
    pgFillvram(WHITE, 0, 0, SCR_WIDTH, SCR_HEIGHT, 2);
    s2ch.pgCursorY = LINE_PITCH;
    s2ch.pgCursorX = 60;
    pgPrint("ｵﾜﾀブラウザ", RED, WHITE, SCR_WIDTH);
    s2ch.pgCursorX = 24;
    s2ch.pgCursorY += LINE_PITCH;
    pgPrint(" 2ちゃんねるブラウザ for PSP ", BLUE, GREEN, SCR_WIDTH);
    s2ch.pgCursorX = 0;
    s2ch.pgCursorY += LINE_PITCH*2;
    pgPrint("　　　 ∧＿∧　　／￣￣￣￣￣￣￣￣￣￣", BLACK, WHITE, SCR_WIDTH);
    s2ch.pgCursorX = 0;
    s2ch.pgCursorY += LINE_PITCH;
    pgPrint("　　　（　´∀｀）＜　", BLACK, WHITE, SCR_WIDTH);
    pgPrint("スタート", RED, WHITE, SCR_WIDTH);
    pgPrint("ボタンで始まるよ", BLACK, WHITE, SCR_WIDTH);
    s2ch.pgCursorX = 0;
    s2ch.pgCursorY += LINE_PITCH;
    pgPrint("　　　（　　　　） 　＼＿＿＿＿＿＿＿＿＿＿", BLACK, WHITE, SCR_WIDTH);
    s2ch.pgCursorX = 0;
    s2ch.pgCursorY += LINE_PITCH;
    pgPrint("　　　｜ ｜　|", BLACK, WHITE, SCR_WIDTH);
    s2ch.pgCursorX = 0;
    s2ch.pgCursorY += LINE_PITCH;
    pgPrint("　　　（＿_）＿）", BLACK, WHITE, SCR_WIDTH);
    s2ch.pgCursorX = 200;
    pgPrint("　 (ﾟдﾟ) < ", BLACK, WHITE, SCR_WIDTH);
    pgPrint("○", RED, WHITE, SCR_WIDTH);
    pgPrint("ボタンでお気に入り", BLACK, WHITE, SCR_WIDTH);
    s2ch.pgCursorX = 200;
    s2ch.pgCursorY += LINE_PITCH;
    pgPrint("　ﾟ(　)−", BLACK, WHITE, SCR_WIDTH);
    s2ch.pgCursorX = 200;
    s2ch.pgCursorY += LINE_PITCH;
    pgPrint("./　>", BLACK, WHITE, SCR_WIDTH);
    s2ch.pgCursorX = 0;
    s2ch.pgCursorY += LINE_PITCH * 2;
    pgPrint("　　　＿＿＿_∧∧　　／￣￣￣￣￣￣￣￣", BLACK, WHITE, SCR_WIDTH);
    s2ch.pgCursorX = 0;
    s2ch.pgCursorY += LINE_PITCH;
    pgPrint("　〜'　＿＿__(,,ﾟДﾟ)＜　", BLACK, WHITE, SCR_WIDTH);
    pgPrint("×", RED, WHITE, SCR_WIDTH);
    pgPrint("ボタンで逝ってよし", BLACK, WHITE, SCR_WIDTH);
    s2ch.pgCursorX = 0;
    s2ch.pgCursorY += LINE_PITCH;
    pgPrint("　　 ＵU 　 　Ｕ U　　　＼＿＿＿＿＿＿＿＿", BLACK, WHITE, SCR_WIDTH);
    s2ch.pgCursorX = 0;
    s2ch.pgCursorY += LINE_PITCH;
}

void pgPrintMonaWait(void)
{
    pgFillvram(WHITE, 0, 0, SCR_WIDTH, SCR_HEIGHT, 2);
    s2ch.pgCursorY = LINE_PITCH;
    s2ch.pgCursorX = 60;
    pgPrint("ｵﾜﾀブラウザ", RED, WHITE, SCR_WIDTH);
    s2ch.pgCursorX = 24;
    s2ch.pgCursorY += LINE_PITCH;
    pgPrint(" 2ちゃんねるブラウザ for PSP ", BLUE, GREEN, SCR_WIDTH);
    s2ch.pgCursorX = 0;
    s2ch.pgCursorY += LINE_PITCH*2;
    pgPrint("  　　 ∧＿∧　　／￣￣￣￣￣￣￣￣￣￣", BLACK, WHITE, SCR_WIDTH);
    s2ch.pgCursorX = 0;
    s2ch.pgCursorY += LINE_PITCH;
    pgPrint("  　　（　´∀｀）＜　", BLACK, WHITE, SCR_WIDTH);
    pgPrint("フォントロード中", RED, WHITE, SCR_WIDTH);
    s2ch.pgCursorX = 0;
    s2ch.pgCursorY += LINE_PITCH;
    pgPrint("  　　（　　　　） 　＼＿＿＿＿＿＿＿＿＿＿", BLACK, WHITE, SCR_WIDTH);
    s2ch.pgCursorX = 0;
    s2ch.pgCursorY += LINE_PITCH;
    pgPrint("  　　｜ ｜　|", BLACK, WHITE, SCR_WIDTH);
    s2ch.pgCursorX = 0;
    s2ch.pgCursorY += LINE_PITCH;
    pgPrint("  　　（＿_）＿）", BLACK, WHITE, SCR_WIDTH);
    s2ch.pgCursorX = 0;
    s2ch.pgCursorY += LINE_PITCH * 4;
    pgPrint("　　　＿＿＿_∧∧　　／￣￣￣￣￣￣￣￣", BLACK, WHITE, SCR_WIDTH);
    s2ch.pgCursorX = 0;
    s2ch.pgCursorY += LINE_PITCH;
    pgPrint("　〜'　＿＿__(,,ﾟДﾟ)＜　", BLACK, WHITE, SCR_WIDTH);
    pgPrint("ちょっと待て", BLACK, WHITE, SCR_WIDTH);
    s2ch.pgCursorX = 0;
    s2ch.pgCursorY += LINE_PITCH;
    pgPrint("　　 ＵU 　 　Ｕ U　　　＼＿＿＿＿＿＿＿＿", BLACK, WHITE, SCR_WIDTH);
}

void pgPrintOwata(void)
{
    pgFillvram(WHITE, 0, 0, SCR_WIDTH, SCR_HEIGHT, 2);
    s2ch.pgCursorX = 160;
    s2ch.pgCursorY = 80;
    pgPrint("人生ｵﾜﾀ＼(^o^)／", BLACK, WHITE, SCR_WIDTH);
}

void pgPrintTate(void)
{
    pgFillvram(WHITE, 0, 0, SCR_WIDTH, SCR_HEIGHT, 2);
    s2ch.pgCursorY = LINE_PITCH;
    s2ch.pgCursorX = 60;
    pgPrint("ｵﾜﾀブラウザ", RED, WHITE, SCR_WIDTH);
    s2ch.pgCursorX = 0;
    s2ch.pgCursorY += LINE_PITCH*4;
    pgPrint("　　 _,,....,,_　 ＿人人人人人人人人人人人人人人人人人＿", BLACK, WHITE, SCR_WIDTH);
    s2ch.pgCursorX = 0;
    s2ch.pgCursorY += LINE_PITCH;
    pgPrint("-''\":::::::::::::｀''＞　SELECTボタンで縦表\示切り替えです！＜", BLACK, WHITE, SCR_WIDTH);
    s2ch.pgCursorX = 0;
    s2ch.pgCursorY += LINE_PITCH;
    pgPrint("ヽ:::::::::::::::::::::￣^Ｙ^Ｙ^Ｙ^Ｙ^Ｙ^Ｙ^Ｙ^Ｙ^Ｙ^Ｙ^Ｙ^Ｙ^Ｙ^Ｙ^Ｙ^Ｙ^￣", BLACK, WHITE, SCR_WIDTH);
    s2ch.pgCursorX = 0;
    s2ch.pgCursorY += LINE_PITCH;
    pgPrint("　|::::::;ノ´￣＼:::::::::::＼_,. -‐ｧ　　　　　＿_　　 _____　　 ＿_____", BLACK, WHITE, SCR_WIDTH);
    s2ch.pgCursorX = 0;
    s2ch.pgCursorY += LINE_PITCH;
    pgPrint("　|::::ﾉ　　　ヽ､ヽr-r'\"´　　（.__　　　　,´　_,, '-´￣￣｀-ゝ 、_ イ、", BLACK, WHITE, SCR_WIDTH);
    s2ch.pgCursorX = 0;
    s2ch.pgCursorY += LINE_PITCH;
    pgPrint("_,.!イ_　　_,.ﾍｰｧ'二ﾊ二ヽ､へ,_7　　　'r ´　　　　　　　　　　ヽ、ﾝ、", BLACK, WHITE, SCR_WIDTH);
    s2ch.pgCursorX = 0;
    s2ch.pgCursorY += LINE_PITCH;
    pgPrint("::::::rｰ''7ｺ-‐'\"´　 　 ;　 ',　｀ヽ/｀7　,'＝=─-　　　 　 -─=＝',　i", BLACK, WHITE, SCR_WIDTH);
    s2ch.pgCursorX = 0;
    s2ch.pgCursorY += LINE_PITCH;
    pgPrint("r-'ｧ'\"´/　 /!　ﾊ 　ハ　 !　　iヾ_ﾉ　i　ｲ　iゝ、ｲ人レ／_ルヽｲ i　|", BLACK, WHITE, SCR_WIDTH);
    s2ch.pgCursorX = 0;
    s2ch.pgCursorY += LINE_PITCH;
    pgPrint("!イ´ ,' |　/__,.!/　V　､!__ﾊ　 ,'　,ゝ　ﾚﾘｲi (ﾋ_] 　　 　ﾋ_ﾝ ).| .|、i .||", BLACK, WHITE, SCR_WIDTH);
    s2ch.pgCursorX = 0;
    s2ch.pgCursorY += LINE_PITCH;
    pgPrint("`! 　!/ﾚi'　(ﾋ_] 　　 　ﾋ_ﾝ ﾚ'i　ﾉ　　　!Y!\"\"　 ,＿__, 　 \"\" 「 !ﾉ i　|", BLACK, WHITE, SCR_WIDTH);
    s2ch.pgCursorX = 0;
    s2ch.pgCursorY += LINE_PITCH;
    pgPrint(",'　 ﾉ 　 !'\"　 　 ,＿__,　 \"' i .ﾚ'　　　　L.',.　 　ヽ _ﾝ　　　　L」 ﾉ| .|", BLACK, WHITE, SCR_WIDTH);
    s2ch.pgCursorX = 0;
    s2ch.pgCursorY += LINE_PITCH;
    pgPrint("　（　　,ﾊ　　　　ヽ _ﾝ　 　人! 　　　　 | ||ヽ、　　　　　　 ,ｲ| ||ｲ| /", BLACK, WHITE, SCR_WIDTH);
    s2ch.pgCursorX = 0;
    s2ch.pgCursorY += LINE_PITCH;
    pgPrint(",.ﾍ,）､　　）＞,､ _____,　,.イ　 ハ　　　　レ ル｀ ー--─ ´ルﾚ　ﾚ´", BLACK, WHITE, SCR_WIDTH);
}
