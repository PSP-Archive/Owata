
#include <pspkernel.h>
#include <pspgu.h>
#include <pspdisplay.h>
#include <psppower.h>
#include <stdio.h>
#include <string.h>
#include "pg.h"
#include "monafontA.h"
#include "monafontW.h"
#include "intraFont.h"

unsigned int __attribute__((aligned(16))) list[512*512];
unsigned int __attribute__((aligned(16))) winPixels[BUF_WIDTH*BUF_HEIGHT];
unsigned int __attribute__((aligned(16))) pixels[BUF_WIDTH*BUF_HEIGHT];
unsigned int __attribute__((aligned(16))) barPixels[BUF_WIDTH*25];
unsigned long pgCursorX = 0, pgCursorY = 0;
unsigned int* printBuf;
void* framebuffer;
intraFont* jpn0;

extern S_2CH_SCREEN res; // psp2chRes.c
extern S_2CH_RES_ANCHOR resAnchor[50]; // psp2chRes.c
extern int resAnchorCount; //psp2chRes.c
extern S_2CH_URL_ANCHOR urlAnchor[50]; // psp2chRes.c
extern int urlAnchorCount; //psp2chRes.c
extern S_2CH_MENU_COLOR menuColor;
extern int tateFlag;

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
#define MAX_ENTITIES 11
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
    entity[10].str = "&#160;";entity[10].len = 5;entity[10].byte = 1;entity[10].c1 = ' '; entity[10].c2 = 0;
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
�����t�H���g�����[�h
*****************************/
void pgFontLoad(void)
{
    pgFillvram(WHITE, 0, 0, SCR_WIDTH, SCR_HEIGHT);
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
���݂̏������݃o�b�t�@�ɂ�������Wx,y�̃A�h���X��Ԃ�
printBuf��ύX���邱�Ƃŏ������݃o�b�t�@��ς����܂�
*****************************/
unsigned int* pgGetVramAddr(unsigned long x,unsigned long y)
{
    return printBuf + x + y * BUF_WIDTH;
}

/*****************************
������Wx1,y1�A���A����w,h�A�Fcolor�Ŏl�p�`��h��Ԃ�
*****************************/
void pgFillvram(int color, int x1, int y1, int w, int h)
{
    unsigned int *vptr0;       //pointer to vram
    unsigned int *vptr;
    unsigned long i, j;

    vptr0 = pgGetVramAddr(0, y1 & 0x01FF) + x1;
    for (i = 0; i < h;) {
        vptr = vptr0;;
        for (j = 0; j < w; j++) {
            *vptr++ = color;
        }
        vptr0 += BUF_WIDTH;
        if (((++i + y1)&0x01FF) == 0) {
            vptr0 -= ZBUF_SIZE;
        }
    }
}

/*****************************
���j���[�o�[��\��
VRAM�̃o�b�t�@�ɒ��ڏ������݂܂��B
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
    pgCursorX = 0;
    if (tateFlag)
    {
        pgFillvram(menuColor.bg, 0, 0, SCR_HEIGHT, 25);
        pgCursorY = 0;
        str = pgPrint(str, menuColor.text, menuColor.bg, SCR_HEIGHT);
        pgCursorY += LINE_PITCH;
        if (str)
        {
            pgCursorX = 0;
            pgPrint(str, menuColor.text, menuColor.bg, SCR_WIDTH);
        }
        pgCursorX = SCR_HEIGHT - 32;
    }
    else
    {
        pgFillvram(menuColor.bg, 0, 0, SCR_WIDTH, 13);
        pgCursorY = 1;
        pgPrint(str, menuColor.text, menuColor.bg, SCR_WIDTH);
        pgCursorX = SCR_WIDTH - 32;
    }
    battery = scePowerGetBatteryLifePercent();
    if (battery < 0)
    {
        pgPrint("  ---", menuColor.bat1, menuColor.bg, SCR_WIDTH);
    }
    else
    {
        if (battery > 40)
        {
            batteryColor = menuColor.bat1;
        }
        else if (battery > 20)
        {
            batteryColor = menuColor.bat2;
        }
        else
        {
            batteryColor = menuColor.bat3;
        }
        pgPrintNumber(battery, batteryColor, menuColor.bg);
        pgPrint("%", batteryColor, menuColor.bg, SCR_WIDTH);
    }
    if (tateFlag)
    {
        src = printBuf;
        dst0 = (unsigned int*)(0x04000000+framebuffer) + 24;
        for (y = 0; y < 25; y++)
        {
            dst = dst0--;
            for (x = 0; x < SCR_HEIGHT; x++)
            {
                *dst = *src++;
                dst += BUF_WIDTH;
            }
            src += (BUF_WIDTH - SCR_HEIGHT);
        }
    }
    else
    {
        src = printBuf;
        dst = (unsigned int*)(0x04000000+framebuffer)+259*BUF_WIDTH;
        for (y = 0; y < 13; y++)
        {
            for (x = 0; x < SCR_WIDTH; x++)
            {
                *dst++ = *src++;
            }
            dst += (BUF_WIDTH - SCR_WIDTH);
            src += (BUF_WIDTH - SCR_WIDTH);
        }
    }
    printBuf = temp;
}

/*****************************
�g�t�Ŏl�p�`��h��Ԃ�
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
    vptr0 = pgGetVramAddr(0, y1);
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
�g����VRAM�ɒ��ڕ\��
*****************************/
void pgWindowFrame(int x1, int y1, int x2, int y2)
{
    unsigned int *vptr0;       //pointer to vram
    unsigned long i, j;
    int tmp;

    if (tateFlag)
    {
        tmp = x1;
        x1 = SCR_WIDTH - y2;
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
�X�N���[���o�[�\��
VRAM�o�b�t�@�ɒ��ڏ�������
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
    if (tateFlag)
    {
        pgFillvram(c.bg, SCR_WIDTH - bar.y - bar.h, bar.x, bar.h, bar.w);
        pgFillvram(c.slider, SCR_WIDTH - sliderY - sliderH, bar.x + 1, sliderH, bar.w - 1);
    }
    else
    {
        pgFillvram(c.bg, bar.x, bar.y, bar.w, bar.h);
        pgFillvram(c.slider, bar.x+1, sliderY, bar.w-1, sliderH);
    }
    printBuf = pixels;
}

/*****************************
���J�[�\����VRAM�o�b�t�@�ɒ��ڕ\��
*****************************/
void pgPadCursor(int x, int y)
{
    unsigned int *vptr;
    int i;

    if (tateFlag)
    {
        vptr = (unsigned int*)(0x04000000+framebuffer) + x * BUF_WIDTH + SCR_WIDTH - y - 18;
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
���݂̏������݃o�b�t�@��offset�̈ʒu����VRAM�o�b�t�@�փE�B���h�E�]��
*****************************/
void pgCopyWindow(int offset, int x, int y, int w, int h)
{
    int i, j;
    unsigned int *src;
    unsigned int *dst, *dst0;

    if (tateFlag)
    {
        src = printBuf + ((offset + y) & 0x01FF) * BUF_WIDTH + x;
        dst0 = (unsigned int*)(0x04000000 + framebuffer) + SCR_WIDTH;
        dst0 += x * BUF_WIDTH - (y & 0x01FF);
        for (i = 0; i < h; i++)
        {
            dst = dst0--;
            for (j = 0; j < w; j++)
            {
                *dst = *src++;
                dst += BUF_WIDTH;
            }
            src += (BUF_WIDTH - w);
            if (src >= printBuf + ZBUF_SIZE) {
                src -= ZBUF_SIZE;
            }
        }
    }
    else
    {
        src = printBuf + ((offset + y) & 0x01FF) * BUF_WIDTH + x;
        dst = (unsigned int*)(0x04000000 + framebuffer);
        dst += (y & 0x01FF) * BUF_WIDTH + x;
        for (i = 0; i < h; i++)
        {
            for (j = 0; j < w; j++)
            {
                *dst++ = *src++;
            }
            dst += (BUF_WIDTH - w);
            src += (BUF_WIDTH - w);
            if (src >= printBuf + ZBUF_SIZE) {
                src -= ZBUF_SIZE;
            }
        }
    }
}

/*****************************
���݂̏������݃o�b�t�@��offset�̈ʒu����VRAM�o�b�t�@�ɑS��ʓ]��
*****************************/
void pgCopy(int offsetX, int offsetY)
{
    int x, y;
    unsigned int *src;
    unsigned int *dst, *dst0;

    offsetY &= 0x01FF;
    if (tateFlag)
    {
        src = printBuf + offsetX + (offsetY * BUF_WIDTH);
        dst0 = (unsigned int*)(0x04000000+framebuffer) + SCR_WIDTH;
        for (y = 0; y < SCR_WIDTH; y++)
        {
            dst = dst0--;
            for (x = 0; x < SCR_HEIGHT; x++)
            {
                *dst = *src++;
                dst += BUF_WIDTH;
            }
            src += (BUF_WIDTH - SCR_HEIGHT);
            if (src >= printBuf + ZBUF_SIZE) {
                src -= ZBUF_SIZE;
            }
        }
    }
    else
    {
        src = printBuf + offsetX + (offsetY * BUF_WIDTH);
        dst = (unsigned int*)(0x04000000+framebuffer);
        for (y = 0; y < SCR_HEIGHT; y++)
        {
            for (x = 0; x < SCR_WIDTH; x++)
            {
                *dst++ = *src++;
            }
            dst += (BUF_WIDTH - SCR_WIDTH);
            src += (BUF_WIDTH - SCR_WIDTH);
            if (src >= printBuf + ZBUF_SIZE) {
                src -= ZBUF_SIZE;
            }
        }
    }
}

/*****************************
�����𓙕�(6pixels)�ŕ\��
"1"��1�h�b�g�E�ɂ��炵��
*****************************/
void pgPrintNumber(int num, int color,int bgcolor)
{
    unsigned int *vptr0, *vptr;
    char buf[16];
    int i, j, cy, b;
    unsigned short* font;

    pgCursorY &= 0x01FF;
    sprintf(buf, "%4d", num);
    for (j = 0; j < 4;j++) {
        font = (unsigned short*)(monafontA + ((buf[j] - 0x20) << 5)) + 1;
        vptr0 = pgGetVramAddr(pgCursorX, pgCursorY);
        pgCursorX += 6;
        for (cy = 0; cy < 12; cy++) {
            vptr = vptr0;
            b = 0x8000;
            for (i = 0; i < 6; i++) {
                if (buf[j] == '1' && i == 0) {
                    *vptr++ = bgcolor;
                    i++;
                }
                if ((*font & b)!=0) {
                    *vptr=color;
                } else {
                    *vptr=bgcolor;
                }
                vptr++;
                b >>= 1;
            }
            vptr0 += BUF_WIDTH;
            if (vptr0 >= printBuf + ZBUF_SIZE) {
                vptr0 -= ZBUF_SIZE;
            }
            font++;
        }
    }
}

/*****************************
1�������t�H���g����ǂݍ����width���ŕ\���\�Ȃ�\������0��Ԃ�
width�𒴂���ꍇ�͕\�����Ȃ���1��Ԃ�
*****************************/
int pgPutChar(unsigned char *cfont,int ch,int color,int bgcolor, int width)
{
    unsigned int *vptr0;       //pointer to vram
    unsigned int *vptr;        //pointer to vram
    unsigned short *font;
    int cx, cy, i, b;

    font = (unsigned short*)(cfont + (ch<<5));
    cx = *font++;
    if ((pgCursorX + cx) >= width) {
        return 1;
    }
    pgCursorY &= 0x01FF;
    vptr0 = pgGetVramAddr(pgCursorX, pgCursorY);
    pgCursorX += cx;
    for (cy = 0; cy < 12; cy++) {
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
        vptr0 += BUF_WIDTH;
        if (vptr0 >= printBuf + ZBUF_SIZE) {
            vptr0 -= ZBUF_SIZE;
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
    if ((index << 5) >= size_monafontA) {
        index = '?' - 0x20;
    }
    return pgPutChar(monafontA, index, color, bgcolor, width);
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
    if ((index << 5) >= size_monafontW) {
        index = 8; // '�H'
    }
    return pgPutChar(monafontW, index, color, bgcolor, width);
}

/*****************************
���̎Q�Ƃ�ϊ�
*****************************/
int pgSpecialChars(char** string,int color,int bgcolor, int width)
{
    int i;

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
    return pgPutCharA('&', color, bgcolor, width);
}

/*****************************
������str����ʕ�width��1�s���\�����ĉ��s�����̃|�C���^��Ԃ�
str��S���\��������NULL��Ԃ�
*****************************/
char* pgPrint(char *str,int color,int bgcolor, int width)
{
    unsigned char ch = 0,bef = 0;
    int ret = 0;

    while(*str) {
        ch = (unsigned char)*str;
        if (bef!=0) {
            ret = pgPutCharW(bef, ch, color, bgcolor, width);
            if (ret) { // ���s���̈ʒu��Ԃ�
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

/*****************************
������str����ʕ�width��1�s���\�����ĉ��s�����̃|�C���^��Ԃ�
str��S���\��������NULL��Ԃ�
<br>�ŉ��s
����HTML�^�O�͍폜
���X�A���J�[��URL�A���J�[������Έʒu����ۑ�
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
                    resAnchor[resAnchorCount].x2 = pgCursorX + 6;
                    resAnchorCount++;
                    if (resAnchorCount >= 50)
                    {
                        resAnchorCount = 0;
                    }
                    resAnchor[resAnchorCount].x1 = 0;
                }
                else if (anchorOn == 1) {
                    urlAnchor[urlAnchorCount].x2 = pgCursorX + 6;
                    urlAnchorCount++;
                    if (urlAnchorCount >= 50)
                    {
                        urlAnchorCount = 0;
                    }
                    urlAnchor[urlAnchorCount].x1 = 0;
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
                        resAnchor[resAnchorCount].x2 = pgCursorX + 6;
                        resAnchorCount++;
                        if (resAnchorCount >= 50)
                        {
                            resAnchorCount = 0;
                        }
                        resAnchor[resAnchorCount].x1 = 0;
                    }
                }
                else if (anchorOn == 1) {
                    if ((ch >= '#' && ch <= '+') || (ch >= '-' && ch <= ';') || (ch >= '?' && ch <= 'Z') ||
                            (ch >= 'a' && ch <= 'z') || ch == '!' || ch == '=' || ch == '_' || ch == '~') {
                    }
                    else {
                        anchorOn = 0;
                        tcolor = c.text;
                        urlAnchor[urlAnchorCount].x2 = pgCursorX + 6;
                        urlAnchorCount++;
                        if (urlAnchorCount >= 50)
                        {
                            urlAnchorCount = 0;
                        }
                        urlAnchor[urlAnchorCount].x1 = 0;
                    }
                }

                if (strstr(str, "http://") == str) {
                    urlAnchor[urlAnchorCount].x1 = pgCursorX;
                    urlAnchor[urlAnchorCount].line = drawLine;
                    tcolor = c.link;
                }
                else if (strstr(str, "ttp://") == str) {
                    if (urlAnchor[urlAnchorCount].x1 == 0) {
                        urlAnchor[urlAnchorCount].x1 = pgCursorX;
                        urlAnchor[urlAnchorCount].line = drawLine;
                    }
                    p = str + 6;
                    j = 0;
                    while ((*p >= '#' && *p <= '+') || (*p >= '-' && *p <= ';') || (*p >= '?' && *p <= 'Z') ||
                            (*p >= 'a' && *p <= 'z') || *p == '!' || *p == '=' || *p == '_' || *p == '~') {
                        if (*p == '/') {
                            p++;
                            break;
                        }
                        urlAnchor[urlAnchorCount].host[j] = *p++;
                        j++;
                        if (j >= 63) {
                            break;
                        }
                    }
                    if (j < 64) {
                        urlAnchor[urlAnchorCount].host[j] = '\0';
                        j = 0;
                        while ((*p >= '#' && *p <= '+') || (*p >= '-' && *p <= ';') || (*p >= '?' && *p <= 'Z') ||
                                (*p >= 'a' && *p <= 'z') || *p == '!' || *p == '=' || *p == '_' || *p == '~') {
                            urlAnchor[urlAnchorCount].path[j] = *p++;
                            j++;
                            if (j >= 255) {
                                break;
                            }
                        }
                        if (j < 256) {
                            urlAnchor[urlAnchorCount].path[j] = '\0';
                        }
                        anchorOn = 1;
                        tcolor = c.link;
                    }
                }

                if (ch == '<') {
                    if (strstr(str, "<br>") == str) {
                        str +=4;
                        anchorOn = 0;
                        tcolor = c.text;
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
                            resAnchor[resAnchorCount].x1 = pgCursorX;
                            resAnchor[resAnchorCount].line = drawLine;
                            tcolor = c.link;
                        }
                        else if (*(str + 4) >= '0' && *(str + 4) <= '9') {
                            if (resAnchor[resAnchorCount].x1 == 0) {
                                resAnchor[resAnchorCount].x1 = pgCursorX;
                                resAnchor[resAnchorCount].line = drawLine;
                            }
                            j = 0;
                            p = str + 4;
                            while ((*p >= '0' && *p <= '9') || *p == '-' || *p == ',' || *p == '<') {
                                start = 0;
                                while (*p >= '0' && *p <= '9') {
                                    start = start * 10 + *p - '0';
                                    p++;
                                }
                                if (start <= res.count && start > 0) {
                                    resAnchor[resAnchorCount].res[j] = start - 1;
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
                                        if (i >= res.count) {
                                            break;
                                        }
                                        if (i > 1) {
                                            resAnchor[resAnchorCount].res[j] = i;
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
                            resAnchor[resAnchorCount].resCount = j;
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
                        resAnchor[resAnchorCount].x2 = pgCursorX;
                        resAnchorCount++;
                        if (resAnchorCount >= 50) {
                            resAnchorCount = 0;
                        }
                        for (i = 0; i < resAnchor[resAnchorCount-1].resCount; i++) {
                            resAnchor[resAnchorCount].res[i] = resAnchor[resAnchorCount-1].res[i];
                        }
                        resAnchor[resAnchorCount].resCount = resAnchor[resAnchorCount-1].resCount;
                        resAnchor[resAnchorCount].line = drawLine+1;
                        resAnchor[resAnchorCount].x1 = startX;
                    }
                    else if (anchorOn == 1) {
                        urlAnchor[urlAnchorCount].x2 = pgCursorX;
                        urlAnchorCount++;
                        if (urlAnchorCount >= 50) {
                            urlAnchorCount = 0;
                        }
                        urlAnchor[urlAnchorCount].line = drawLine+1;
                        urlAnchor[urlAnchorCount].x1 = startX;
                        strcpy(urlAnchor[urlAnchorCount].host, urlAnchor[urlAnchorCount-1].host);
                        strcpy(urlAnchor[urlAnchorCount].path, urlAnchor[urlAnchorCount-1].path);
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
    if ((index<<5) >= size_monafontA) {
        index = '?' - 0x20;
    }
    font = (unsigned short*)(monafontA + (index<<5));
    if ((pgCursorX + *font) >= width) {
        return 1;
    }
    pgCursorX += *font;
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
    if ((index<<5) >= size_monafontW) {
        index = 8; // '�H'
    }
    font = (unsigned short*)(monafontW + (index<<5));
    if ((pgCursorX + *font) >= width) {
        return 1;
    }
    pgCursorX += *font;
    return 0;
}

int pgCountSpecialChars(char** string, int width)
{
    int i;

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
    return pgCountCharA('&', width);
}

/*****************************
str����ʕ�width�ŕ\�������Ƃ��̍s���𐔂���̂Ɏg���֐�
�\���͍s���Ȃ�
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
    pgCursorX = 0;
    pgCursorY = 0;
}

void pgPrintMona(void)
{
    pgFillvram(WHITE, 0, 0, SCR_WIDTH, SCR_HEIGHT);
    pgCursorY = LINE_PITCH;
    pgCursorX = 60;
    pgPrint("����u���E�U", RED, WHITE, SCR_WIDTH);
    pgCursorX = 24;
    pgCursorY += LINE_PITCH;
    pgPrint(" 2�����˂�u���E�U for PSP ", BLUE, GREEN, SCR_WIDTH);
    pgCursorX = 0;
    pgCursorY += LINE_PITCH*2;
    pgPrint("�@�@�@ �ȁQ�ȁ@�@�^�P�P�P�P�P�P�P�P�P�P", BLACK, WHITE, SCR_WIDTH);
    pgCursorX = 0;
    pgCursorY += LINE_PITCH;
    pgPrint("�@�@�@�i�@�L�́M�j���@", BLACK, WHITE, SCR_WIDTH);
    pgPrint("�X�^�[�g", RED, WHITE, SCR_WIDTH);
    pgPrint("�{�^���Ŏn�܂��", BLACK, WHITE, SCR_WIDTH);
    pgCursorX = 0;
    pgCursorY += LINE_PITCH;
    pgPrint("�@�@�@�i�@�@�@�@�j �@�_�Q�Q�Q�Q�Q�Q�Q�Q�Q�Q", BLACK, WHITE, SCR_WIDTH);
    pgCursorX = 0;
    pgCursorY += LINE_PITCH;
    pgPrint("�@�@�@�b �b�@|", BLACK, WHITE, SCR_WIDTH);
    pgCursorX = 0;
    pgCursorY += LINE_PITCH;
    pgPrint("�@�@�@�i�Q_�j�Q�j", BLACK, WHITE, SCR_WIDTH);
    pgCursorX = 200;
    pgPrint("�@ (߄t�) < ", BLACK, WHITE, SCR_WIDTH);
    pgPrint("��", RED, WHITE, SCR_WIDTH);
    pgPrint("�{�^���ł��C�ɓ���", BLACK, WHITE, SCR_WIDTH);
    pgCursorX = 200;
    pgCursorY += LINE_PITCH;
    pgPrint("�@�(�@)�|", BLACK, WHITE, SCR_WIDTH);
    pgCursorX = 200;
    pgCursorY += LINE_PITCH;
    pgPrint("./�@>", BLACK, WHITE, SCR_WIDTH);
    pgCursorX = 0;
    pgCursorY += LINE_PITCH * 2;
    pgPrint("�@�@�@�Q�Q�Q_�ȁȁ@�@�^�P�P�P�P�P�P�P�P", BLACK, WHITE, SCR_WIDTH);
    pgCursorX = 0;
    pgCursorY += LINE_PITCH;
    pgPrint("�@�`'�@�Q�Q__(,,߄D�)���@", BLACK, WHITE, SCR_WIDTH);
    pgPrint("�~", RED, WHITE, SCR_WIDTH);
    pgPrint("�{�^���Ő����Ă悵", BLACK, WHITE, SCR_WIDTH);
    pgCursorX = 0;
    pgCursorY += LINE_PITCH;
    pgPrint("�@�@ �tU �@ �@�t U�@�@�@�_�Q�Q�Q�Q�Q�Q�Q�Q", BLACK, WHITE, SCR_WIDTH);
    pgCursorX = 0;
    pgCursorY += LINE_PITCH;
}

void pgPrintMonaWait(void)
{
    pgFillvram(WHITE, 0, 0, SCR_WIDTH, SCR_HEIGHT);
    pgCursorY = LINE_PITCH;
    pgCursorX = 60;
    pgPrint("����u���E�U", RED, WHITE, SCR_WIDTH);
    pgCursorX = 24;
    pgCursorY += LINE_PITCH;
    pgPrint(" 2�����˂�u���E�U for PSP ", BLUE, GREEN, SCR_WIDTH);
    pgCursorX = 0;
    pgCursorY += LINE_PITCH*2;
    pgPrint("  �@�@ �ȁQ�ȁ@�@�^�P�P�P�P�P�P�P�P�P�P", BLACK, WHITE, SCR_WIDTH);
    pgCursorX = 0;
    pgCursorY += LINE_PITCH;
    pgPrint("  �@�@�i�@�L�́M�j���@", BLACK, WHITE, SCR_WIDTH);
    pgPrint("�t�H���g���[�h��", RED, WHITE, SCR_WIDTH);
    pgCursorX = 0;
    pgCursorY += LINE_PITCH;
    pgPrint("  �@�@�i�@�@�@�@�j �@�_�Q�Q�Q�Q�Q�Q�Q�Q�Q�Q", BLACK, WHITE, SCR_WIDTH);
    pgCursorX = 0;
    pgCursorY += LINE_PITCH;
    pgPrint("  �@�@�b �b�@|", BLACK, WHITE, SCR_WIDTH);
    pgCursorX = 0;
    pgCursorY += LINE_PITCH;
    pgPrint("  �@�@�i�Q_�j�Q�j", BLACK, WHITE, SCR_WIDTH);
    pgCursorX = 0;
    pgCursorY += LINE_PITCH * 4;
    pgPrint("�@�@�@�Q�Q�Q_�ȁȁ@�@�^�P�P�P�P�P�P�P�P", BLACK, WHITE, SCR_WIDTH);
    pgCursorX = 0;
    pgCursorY += LINE_PITCH;
    pgPrint("�@�`'�@�Q�Q__(,,߄D�)���@", BLACK, WHITE, SCR_WIDTH);
    pgPrint("������Ƒ҂�", BLACK, WHITE, SCR_WIDTH);
    pgCursorX = 0;
    pgCursorY += LINE_PITCH;
    pgPrint("�@�@ �tU �@ �@�t U�@�@�@�_�Q�Q�Q�Q�Q�Q�Q�Q", BLACK, WHITE, SCR_WIDTH);
}

void pgPrintOwata(void)
{
    pgFillvram(WHITE, 0, 0, SCR_WIDTH, SCR_HEIGHT);
    pgCursorX = 160;
    pgCursorY = 80;
    pgPrint("�l������_(^o^)�^", BLACK, WHITE, SCR_WIDTH);
}

void pgPrintTate(void)
{
    pgFillvram(WHITE, 0, 0, SCR_WIDTH, SCR_HEIGHT);
    pgCursorY = LINE_PITCH;
    pgCursorX = 60;
    pgPrint("����u���E�U", RED, WHITE, SCR_WIDTH);
    pgCursorX = 0;
    pgCursorY += LINE_PITCH*4;
    pgPrint("�@�@ _,,....,,_�@ �Q�l�l�l�l�l�l�l�l�l�l�l�l�l�l�l�l�l�Q", BLACK, WHITE, SCR_WIDTH);
    pgCursorX = 0;
    pgCursorY += LINE_PITCH;
    pgPrint("-''\":::::::::::::�M''���@SELECT�{�^���ŏc�\\���؂�ւ��ł��I��", BLACK, WHITE, SCR_WIDTH);
    pgCursorX = 0;
    pgCursorY += LINE_PITCH;
    pgPrint("�R:::::::::::::::::::::�P^�x^�x^�x^�x^�x^�x^�x^�x^�x^�x^�x^�x^�x^�x^�x^�x^�P", BLACK, WHITE, SCR_WIDTH);
    pgCursorX = 0;
    pgCursorY += LINE_PITCH;
    pgPrint("�@|::::::;�m�L�P�_:::::::::::�__,. -�]��@�@�@�@�@�Q_�@�@ _____�@�@ �Q_____", BLACK, WHITE, SCR_WIDTH);
    pgCursorX = 0;
    pgCursorY += LINE_PITCH;
    pgPrint("�@|::::Ɂ@�@�@�R��Rr-r'\"�L�@�@�i.__�@�@�@�@,�L�@_,, '-�L�P�P�M-�T �A_ �C�A", BLACK, WHITE, SCR_WIDTH);
    pgCursorX = 0;
    pgCursorY += LINE_PITCH;
    pgPrint("_,.!�C_�@�@_,.Ͱ�'��ʓ�R���,_7�@�@�@'r �L�@�@�@�@�@�@�@�@�@�@�R�A݁A", BLACK, WHITE, SCR_WIDTH);
    pgCursorX = 0;
    pgCursorY += LINE_PITCH;
    pgPrint("::::::r�''7�-�]'\"�L�@ �@ ;�@ ',�@�M�R/�M7�@,'��=��-�@�@�@ �@ -��=��',�@i", BLACK, WHITE, SCR_WIDTH);
    pgCursorX = 0;
    pgCursorY += LINE_PITCH;
    pgPrint("r-'�'\"�L/�@ /!�@� �@�n�@ !�@�@i�S_Ɂ@i�@��@i�T�A��l���^_���R� i�@|", BLACK, WHITE, SCR_WIDTH);
    pgCursorX = 0;
    pgCursorY += LINE_PITCH;
    pgPrint("!�C�L ,' |�@/__,.!/�@V�@�!__ʁ@ ,'�@,�T�@�زi (�_] �@�@ �@�_� ).| .|�Ai .||", BLACK, WHITE, SCR_WIDTH);
    pgCursorX = 0;
    pgCursorY += LINE_PITCH;
    pgPrint("`! �@!/�i'�@(�_] �@�@ �@�_� �'i�@Ɂ@�@�@!Y!\"\"�@ ,�Q__, �@ \"\" �u !� i�@|", BLACK, WHITE, SCR_WIDTH);
    pgCursorX = 0;
    pgCursorY += LINE_PITCH;
    pgPrint(",'�@ � �@ !'\"�@ �@ ,�Q__,�@ \"' i .�'�@�@�@�@L.',.�@ �@�R _݁@�@�@�@L�v �| .|", BLACK, WHITE, SCR_WIDTH);
    pgCursorX = 0;
    pgCursorY += LINE_PITCH;
    pgPrint("�@�i�@�@,ʁ@�@�@�@�R _݁@ �@�l! �@�@�@�@ | ||�R�A�@�@�@�@�@�@ ,�| ||�| /", BLACK, WHITE, SCR_WIDTH);
    pgCursorX = 0;
    pgCursorY += LINE_PITCH;
    pgPrint(",.�,�j��@�@�j��,� _____,�@,.�C�@ �n�@�@�@�@�� ���M �[--�� �L��ځ@ځL", BLACK, WHITE, SCR_WIDTH);
}
