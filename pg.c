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

extern S_2CH s2ch; // psp2chRes.c

TEX tex = {BUF_WIDTH*2, BUF_HEIGHT, BUF_WIDTH*2};
TEX cur = {32, 32, 32};
RECT barSrcRectH; // psp2chInit(psp2ch.c)で初期化
RECT barSrcRectV; // psp2chInit(psp2ch.c)で初期化
RECT menuDstRectH; // psp2chInit(psp2ch.c)で初期化
RECT menuDstRectV; // psp2chInit(psp2ch.c)で初期化
RECT titleDstRectH; // psp2chInit(psp2ch.c)で初期化
RECT titleDstRectV; // psp2chInit(psp2ch.c)で初期化
static unsigned char *fontA, *fontJ;
static int size_fontA, size_fontJ;
#define MAX_ENTITIES 12
static struct entityTag entity[MAX_ENTITIES];
static S_PUTCHAR sChar;

unsigned int   __attribute__((aligned(16))) list[512*512];
unsigned short __attribute__((aligned(16))) pixels[BUF_WIDTH*BUF_HEIGHT*2];
unsigned short __attribute__((aligned(16))) winPixels[BUF_WIDTH*BUF_HEIGHT*2];
unsigned short __attribute__((aligned(16))) barPixels[BUF_WIDTH*32*2];
unsigned short __attribute__((aligned(16))) titlePixels[BUF_WIDTH*32*2];
unsigned short* printBuf;
void* framebuffer;
intraFont* jpn0;

unsigned short __attribute__((aligned(16))) cursorImg[32*45];
#define O 0
#define B 1
#define W 2
#define G 3
unsigned short cursorFont[32*45] = {
    O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
    O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
    O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
    O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
    O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
    O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
    O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
    O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
    O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
    O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
    O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
    O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
    O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
    B,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
    B,B,O,G,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
    B,W,B,G,G,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
    B,W,W,B,G,G,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
    B,W,W,W,B,G,G,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
    B,W,W,W,W,B,G,G,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
    B,W,W,W,W,W,B,G,G,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
    B,W,W,W,W,W,W,B,G,G,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
    B,W,W,W,W,W,W,W,B,G,G,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
    B,W,W,W,W,W,W,W,W,B,G,G,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
    B,W,W,W,W,W,B,B,B,B,B,G,G,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
    B,W,W,B,W,W,B,G,G,G,G,G,G,G,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
    B,W,B,G,B,W,W,B,G,G,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
    B,B,G,G,B,W,W,B,G,G,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
    B,O,G,O,O,B,W,W,B,G,G,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
    O,O,O,O,O,B,W,W,B,G,G,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
    O,O,O,O,O,O,B,W,W,B,G,G,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
    O,O,O,O,O,O,B,W,W,B,G,G,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
    O,O,O,O,O,O,O,B,B,B,G,G,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
    O,O,O,O,O,O,O,O,0,G,G,G,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
    O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
    O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
    O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
    O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
    O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
    O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
    O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
    O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
    O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
    O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
    O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
    O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O
};
#undef O
#undef B
#undef W
#undef G

/*************************
カーソルのフォントからビットマップ画像を作る
**************************/
void pgCursorColorSet(void)
{
    int i;
    for (i = 0; i < 32*45; i++)
    {
        if (cursorFont[i] == 1)
        {
            cursorImg[i] = s2ch.cursorColor.arrow1;
        }
        else if (cursorFont[i] == 2)
        {
            cursorImg[i] = s2ch.cursorColor.arrow2;
        }
        else if (cursorFont[i] == 3)
        {
            cursorImg[i] = 0x3000;
        }
        else
        {
            cursorImg[i] = 0;
        }
    }
}

void pgEntitiesSet(void)
{
    entity[0].str  = "&amp;"; entity[0].len  = 4;entity[0].byte  = 1;entity[0].c1  = '&'; entity[0].c2  = 0;
    entity[1].str  = "&gt;";  entity[1].len  = 3;entity[1].byte  = 1;entity[1].c1  = '>'; entity[1].c2  = 0;
    entity[2].str  = "&lt;";  entity[2].len  = 3;entity[2].byte  = 1;entity[2].c1  = '<'; entity[2].c2  = 0;
    entity[3].str  = "&quot;";entity[3].len  = 5;entity[3].byte  = 1;entity[3].c1  = '"'; entity[3].c2  = 0;
    entity[4].str  = "&nbsp;";entity[4].len  = 5;entity[4].byte  = 1;entity[4].c1  = ' '; entity[4].c2  = 0;
    entity[5].str  = "&rarr;";entity[5].len  = 5;entity[5].byte  = 2;entity[5].c1  = 0x81;entity[5].c2  = 0xA8;
    entity[6].str  = "&larr;";entity[6].len  = 5;entity[6].byte  = 2;entity[6].c1  = 0x81;entity[6].c2  = 0xA9;
    entity[7].str  = "&sub;"; entity[7].len  = 4;entity[7].byte  = 2;entity[7].c1  = 0x81;entity[7].c2  = 0xBC;
    entity[8].str  = "&and;"; entity[8].len  = 4;entity[8].byte  = 2;entity[8].c1  = 0x81;entity[8].c2  = 0xC8;
    entity[9].str  = "&or;";  entity[9].len  = 3;entity[9].byte  = 2;entity[9].c1  = 0x81;entity[9].c2  = 0xC9;
    entity[10].str = "&uarr;";entity[10].len = 5;entity[10].byte = 2;entity[10].c1 = 0x81;entity[10].c2 = 0xAA;
    entity[11].str = "&darr;";entity[11].len = 5;entity[11].byte = 2;entity[11].c1 = 0x81;entity[11].c2 = 0xAB;
}

/*****************************
外部フォントファイルを読み込む
*****************************/
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
    framebuffer = (void*)0;
    printBuf = pixels;
    pgEntitiesSet();
    intraFontInit();
    sceGuDisplay(GU_FALSE);
    sceGuInit();
    sceGuStart(GU_DIRECT,list);
    sceGuDrawBuffer(GU_PSM_8888,framebuffer,BUF_WIDTH);
    sceGuDispBuffer(SCR_WIDTH,SCR_HEIGHT,(void*)0x88000,BUF_WIDTH);
    sceGuDepthBuffer((void*)0x110000,BUF_WIDTH);
    sceGuOffset(2048 - (SCR_WIDTH/2), 2048 - (SCR_HEIGHT/2));
    sceGuViewport(2048, 2048, SCR_WIDTH, SCR_HEIGHT);
    /*
    sceGuFrontFace(GU_CW);
    sceGuShadeModel(GU_SMOOTH);
    sceGuDisable(GU_DEPTH_TEST);
    sceGuDepthRange(65535, 0);
    sceGuDepthFunc(GU_GEQUAL);
    sceGuEnable(GU_CULL_FACE);
    sceGuEnable(GU_CLIP_PLANES);
    */
    sceGuEnable(GU_SCISSOR_TEST);
    sceGuScissor(0, 0, SCR_WIDTH, SCR_HEIGHT);
    sceGuEnable(GU_BLEND);
    sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
    sceGuEnable(GU_TEXTURE_2D);
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
    pgWaitVn(20);
    pgCopy(0, 0);
    sceDisplayWaitVblankStart();
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
unsigned short* pgGetVramAddr(unsigned long x,unsigned long y, int w)
{
    return printBuf + x + y * BUF_WIDTH * w;
}

/*****************************
左上座標x1,y1、幅、高さw,h、色colorで四角形を塗りつぶす
*****************************/
void pgFillvram(unsigned short color, int x1, int y1, int w, int h, int wide)
{
    unsigned short *vptr0;       //pointer to vram
    unsigned short *vptr;
    unsigned long i, j;

    vptr0 = pgGetVramAddr(x1, y1 & 0x01FF, wide);
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

/*--------------------------------------------------------
    矩形範囲をコピー
--------------------------------------------------------*/
#define SLICE_SIZE 64
void pgCopyRect(void *src, TEX *tex, RECT *src_rect, RECT *dst_rect)
{
    int j, sw, dw, sh, dh;
    struct Vertex *vertices;

    sw = src_rect->right - src_rect->left;
    dw = dst_rect->right - dst_rect->left;
    sh = src_rect->bottom - src_rect->top;
    dh = dst_rect->bottom - dst_rect->top;

    sceGuStart(GU_DIRECT, list);
    //sceGuDrawBufferList(GU_PSM_8888, framebuffer, BUF_WIDTH);
    sceGuScissor(dst_rect->left, dst_rect->top, dst_rect->right, dst_rect->bottom);
    sceGuTexMode(GU_PSM_4444, 0, 0, GU_FALSE);
    sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
    sceGuTexWrap(GU_REPEAT, GU_REPEAT);
    if (sw == dw && sh == dh)
        sceGuTexFilter(GU_NEAREST, GU_NEAREST);
    else
        sceGuTexFilter(GU_LINEAR, GU_LINEAR);
    for (j = 0; (j + SLICE_SIZE) < sw; j = j + SLICE_SIZE)
    {
        sceGuTexImage(0, tex->w, tex->h, tex->tb, (short*)src + j);
        vertices = (struct Vertex *)sceGuGetMemory(2 * sizeof(struct Vertex));
        vertices[0].u = src_rect->left;
        vertices[0].v = src_rect->top;
        vertices[0].x = dst_rect->left + j * dw / sw;
        vertices[0].y = dst_rect->top;
        vertices[1].u = src_rect->left + SLICE_SIZE;
        vertices[1].v = src_rect->bottom;
        vertices[1].x = dst_rect->left + (j + SLICE_SIZE) * dw / sw;
        vertices[1].y = dst_rect->bottom;
        sceGuDrawArray(GU_SPRITES, GU_TEXTURE_16BIT | GU_COLOR_4444 | GU_VERTEX_16BIT | GU_TRANSFORM_2D, 2, NULL, vertices);
    }
    if (j < sw)
    {
        sceGuTexImage(0, tex->w, tex->h, tex->tb, (short*)src + j);
        vertices = (struct Vertex *)sceGuGetMemory(2 * sizeof(struct Vertex));
        vertices[0].u = src_rect->left;
        vertices[0].v = src_rect->top;
        vertices[0].x = dst_rect->left + j * dw / sw;
        vertices[0].y = dst_rect->top;
        vertices[1].u = src_rect->left + sw - j;
        vertices[1].v = src_rect->bottom;
        vertices[1].x = dst_rect->right;
        vertices[1].y = dst_rect->bottom;
        sceGuDrawArray(GU_SPRITES, GU_TEXTURE_16BIT | GU_COLOR_4444 | GU_VERTEX_16BIT | GU_TRANSFORM_2D, 2, NULL, vertices);
    }
    sceGuFinish();
    sceGuSync(0, GU_SYNC_FINISH);
}

/*--------------------------------------------------------
    矩形範囲を90度回転してコピー
--------------------------------------------------------*/
void pgCopyRectRotate(void *src, TEX *tex, RECT *src_rect, RECT *dst_rect)
{
    short j, sw, dw, sh, dh;
    struct Vertex *vertices;

    sw = src_rect->right - src_rect->left;
    dw = dst_rect->right - dst_rect->left;
    sh = src_rect->bottom - src_rect->top;
    dh = dst_rect->bottom - dst_rect->top;

    sceGuStart(GU_DIRECT, list);
    //sceGuDrawBufferList(GU_PSM_8888, framebuffer, BUF_WIDTH);
    sceGuScissor(dst_rect->left, dst_rect->top, dst_rect->right, dst_rect->bottom);
    sceGuTexMode(GU_PSM_4444, 0, 0, GU_FALSE);
    sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
    sceGuTexWrap(GU_REPEAT, GU_REPEAT);
    if (sw == dh && sh == dw)
        sceGuTexFilter(GU_NEAREST, GU_NEAREST);
    else
        sceGuTexFilter(GU_LINEAR, GU_LINEAR);
    for (j = 0; (j + SLICE_SIZE) < sw; j = j + SLICE_SIZE)
    {
        sceGuTexImage(0, tex->w, tex->h, tex->tb, (short*)src + j);
        vertices = (struct Vertex *)sceGuGetMemory(2 * sizeof(struct Vertex));
        vertices[0].u = src_rect->left;
        vertices[0].v = src_rect->top;
        vertices[0].x = dst_rect->right;
        vertices[0].y = dst_rect->top + j * dh / sw;
        vertices[1].u = src_rect->left + SLICE_SIZE;
        vertices[1].v = src_rect->bottom;
        vertices[1].x = dst_rect->left;
        vertices[1].y = dst_rect->top + (j + SLICE_SIZE) * dh / sw;
        sceGuDrawArray(GU_SPRITES, GU_TEXTURE_16BIT | GU_COLOR_4444 | GU_VERTEX_16BIT | GU_TRANSFORM_2D, 2, NULL, vertices);
    }
    if (j < sw)
    {
        sceGuTexImage(0, tex->w, tex->h, tex->tb, (short*)src + j);
        vertices = (struct Vertex *)sceGuGetMemory(2 * sizeof(struct Vertex));
        vertices[0].u = src_rect->left;
        vertices[0].v = src_rect->top;
        vertices[0].x = dst_rect->right;
        vertices[0].y = dst_rect->top + j * dh / sw;
        vertices[1].u = src_rect->left + sw - j;
        vertices[1].v = src_rect->bottom;
        vertices[1].x = dst_rect->left;
        vertices[1].y = dst_rect->bottom;
        sceGuDrawArray(GU_SPRITES, GU_TEXTURE_16BIT | GU_COLOR_4444 | GU_VERTEX_16BIT | GU_TRANSFORM_2D, 2, NULL, vertices);
    }
    sceGuFinish();
    sceGuSync(0, GU_SYNC_FINISH);
}

/*--------------------------------------------------------
    指定した矩形範囲を塗りつぶし
--------------------------------------------------------*/
void pgFillRect(unsigned short color, RECT *rect)
{
    unsigned int r, g, b, a, c;
    // RGBA4444 => RGBA8888へ
    r = color & 0x0F;
    g = (color >> 4) & 0x0F;
    b = (color >> 8) & 0x0F;
    a = (color >> 12) & 0x0F;
    r = (r << 4) | r;
    g = (g << 4) | g;
    b = (b << 4) | b;
    a = (a << 4) | a;
    c = GU_RGBA(r,g,b,a);
    sceGuStart(GU_DIRECT, list);
    sceGuScissor(rect->left, rect->top, rect->right, rect->bottom);
    sceGuClearColor(c);
    sceGuClear(GU_COLOR_BUFFER_BIT);
    sceGuFinish();
    sceGuSync(0, GU_SYNC_FINISH);
}

/*****************************
タイトルバーを描画
*****************************/
void pgPrintTitleBar(char* ita, char* title)
{
    static struct tm pre;
    unsigned short *temp;
    time_t timep;
    struct tm *t;
    char date[16];
    char buf[32];

    sceKernelLibcTime(&timep);
    timep += 9 * 60 * 60;
    t = localtime(&timep);
    if (t->tm_hour == pre.tm_hour && t->tm_min == pre.tm_min && t->tm_sec == pre.tm_sec)
    {
        return;
    }
    pre.tm_hour = t->tm_hour;
    pre.tm_min = t->tm_min;
    pre.tm_sec = t->tm_sec;
    sprintf(date, "%02d:%02d:%02d", t->tm_hour, t->tm_min, t->tm_sec);
    sprintf(buf, " [%s]", ita);
    temp = printBuf;
    printBuf = titlePixels; 
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
    printBuf = temp;
    pgWaitVn(6);
}
/*****************************
タイトルバーを表示
*****************************/
void pgCopyTitleBar(void)
{
    if (s2ch.tateFlag)
    {
        pgCopyRectRotate(titlePixels, &tex, &barSrcRectV, &titleDstRectV);
    }
    else
    {
        pgCopyRect(titlePixels, &tex, &barSrcRectH, &titleDstRectH);
    }
}

/*****************************
メニューバーを描画
*****************************/
void pgPrintMenuBar(char* str)
{
    unsigned short *temp;
    int battery;
    int batteryColor;

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
    printBuf = temp;
}
/*****************************
メニューバーを表示
*****************************/
void pgCopyMenuBar(void)
{
    if (s2ch.tateFlag)
    {
        pgCopyRectRotate(barPixels, &tex, &barSrcRectV, &menuDstRectV);
    }
    else
    {
        pgCopyRect(barPixels, &tex, &barSrcRectH, &menuDstRectH);
    }
}

/*****************************
枠付で四角形を塗りつぶす
*****************************/
void pgEditBox(int color, int x1, int y1, int x2, int y2)
{
    unsigned short *vptr0;       //pointer to vram
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
    vptr0 += BUF_WIDTH * 2;
    vptr0[x1+0] = RGB(0x99, 0x99, 0x99);
    for (j = x1+1; j < x2-1; j++) {
        vptr0[j] = RGB(0x33, 0x33, 0x33);
    }
    vptr0[x2-1] = RGB(0x99, 0x99, 0x99);
    for (i = y1+2; i < y2-1; i++) {
        vptr0 += BUF_WIDTH * 2;
        vptr0[x1+0] = RGB(0x99, 0x99, 0x99);
        vptr0[x1+1] = RGB(0x33, 0x33, 0x33);
        for (j = x1+2; j < x2-1; j++) {
            vptr0[j] = color;
        }
        vptr0[x2-1] = RGB(0x99, 0x99, 0x99);
    }
    vptr0 += BUF_WIDTH * 2;
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
        vptr0[j] = GU_RGBA(0x99, 0x99, 0x99, 0xFF);
    }
    vptr0 += BUF_WIDTH;
    vptr0[x1+0] = GU_RGBA(0x99, 0x99, 0x99, 0xFF);
    for (j = x1+1; j < x2-1; j++) {
        vptr0[j] = GU_RGBA(0x33, 0x33, 0x33, 0xFF);
    }
    vptr0[x2-1] = GU_RGBA(0x99, 0x99, 0x99, 0xFF);
    for (i = y1+2; i < y2-1; i++) {
        vptr0 += BUF_WIDTH;
        vptr0[x1+0] = GU_RGBA(0x99, 0x99, 0x99, 0xFF);
        vptr0[x1+1] = GU_RGBA(0x33, 0x33, 0x33, 0xFF);
        vptr0[x2-1] = GU_RGBA(0x99, 0x99, 0x99, 0xFF);
    }
    vptr0 += BUF_WIDTH;
    for (j = x1; j < x2; j++) {
        vptr0[j] = GU_RGBA(0x99, 0x99, 0x99, 0xFF);
    }
}

/*****************************
スクロールバー表示
*****************************/
void pgScrollbar(S_SCROLLBAR* bar, S_2CH_BAR_COLOR c)
{
    unsigned short *temp;
    int sliderH, sliderY;
    RECT rect;

    temp = printBuf;
    // winPixelsの余ってる部分を使用
    printBuf = winPixels + BUF_WIDTH * 2 -32;

    sliderH = bar->view * bar->h / bar->total;
    sliderY = bar->start * bar->h / bar->total + bar->y;
    if (sliderH < 2) {
        sliderH = 2;
    }
    if (sliderY >= (bar->y + bar->h - 1)) {
        sliderY = bar->y + bar->h - 2;
    }
    if (s2ch.tateFlag)
    {
        rect.left = SCR_WIDTH - bar->y - bar->h;
        rect.top = bar->x;
        rect.right = SCR_WIDTH - bar->y;
        rect.bottom = bar->x + bar->w;
        pgFillRect(c.bg, &rect);
        rect.left = SCR_WIDTH - sliderY - sliderH;
        rect.top = bar->x + 1;
        rect.right = SCR_WIDTH - sliderY;
        rect.bottom = bar->x + bar->w;
        pgFillRect(c.slider, &rect);
    }
    else
    {
        rect.left = bar->x;
        rect.top = bar->y;
        rect.right = bar->x + bar->w;
        rect.bottom = bar->y + bar->h;
        pgFillRect(c.bg, &rect);
        rect.left = bar->x + 1;
        rect.top = sliderY;
        rect.right = bar->x + bar->w;
        rect.bottom = sliderY + sliderH;
        pgFillRect(c.slider, &rect);
    }
    printBuf = temp;
}

/*****************************
矢印カーソルを表示
*****************************/
void pgPadCursor(int x, int y)
{
    RECT src_rect, dst_rect;
    
    if (s2ch.tateFlag)
    {
        src_rect.left = 0;
        src_rect.top = 0;
        src_rect.right = cur.w;
        src_rect.bottom = cur.h;
        dst_rect.left = SCR_WIDTH - y - cur.h + 13;
        dst_rect.top = x;
        dst_rect.right = SCR_WIDTH - y + 13;
        dst_rect.bottom = x + cur.w;
        pgCopyRectRotate(cursorImg, &cur, &src_rect, &dst_rect);
    }
    else
    {
        src_rect.left = 0;
        src_rect.top = 13;
        src_rect.right = cur.w;
        src_rect.bottom = 13+cur.h;
        dst_rect.left = x;
        dst_rect.top = y;
        dst_rect.right = x + cur.w;
        dst_rect.bottom = y + cur.h;
        pgCopyRect(cursorImg, &cur, &src_rect, &dst_rect);
    }
}

/*****************************
現在の書き込みバッファをoffsetの位置からVRAMバッファへウィンドウ転送
*****************************/
void pgCopyWindow(int offset, int x, int y, int w, int h)
{
    int offsetY = (offset + y) & 0x01FF;
    RECT src_rect, dst_rect;

    offsetY &= 0x01FF;
    if (s2ch.tateFlag)
    {
        src_rect.left = x;
        src_rect.top = offsetY;
        src_rect.right = x + w;
        src_rect.bottom = offsetY + h;
        dst_rect.left = SCR_WIDTH - y - h;
        dst_rect.top = x;
        dst_rect.right = SCR_WIDTH - y;
        dst_rect.bottom = x + w;
        pgCopyRectRotate(printBuf, &tex, &src_rect, &dst_rect);
    }
    else
    {
        src_rect.left = x;
        src_rect.top = offsetY;
        src_rect.right = x + w;
        src_rect.bottom = offsetY + h;
        dst_rect.left = x;
        dst_rect.top = y;
        dst_rect.right = x + w;
        dst_rect.bottom = y + h;
        pgCopyRect(printBuf, &tex, &src_rect, &dst_rect);
    }
}

/*****************************
現在の書き込みバッファをoffsetの位置からVRAMバッファに全画面転送
*****************************/

void pgCopy(int offsetX, int offsetY)
{
    RECT src_rect, dst_rect;
    int sx;

    sx = offsetX & 0x1F;
    offsetX &= 0xFFFFFFE0;
    offsetY &= 0x01FF;
    if (s2ch.tateFlag)
    {
        src_rect.left = sx;
        src_rect.top = offsetY;
        src_rect.right = sx + SCR_HEIGHT;
        src_rect.bottom = offsetY + SCR_WIDTH;
        dst_rect.left = 0;
        dst_rect.top = 0;
        dst_rect.right = SCR_WIDTH;
        dst_rect.bottom = SCR_HEIGHT;
        pgCopyRectRotate(printBuf + offsetX, &tex, &src_rect, &dst_rect);
    }
    else
    {
        src_rect.left = sx;
        src_rect.top = offsetY;
        src_rect.right = sx + SCR_WIDTH;
        src_rect.bottom = offsetY + SCR_HEIGHT;
        dst_rect.left = 0;
        dst_rect.top = 0;
        dst_rect.right = SCR_WIDTH;
        dst_rect.bottom = SCR_HEIGHT;
        pgCopyRect(printBuf + offsetX, &tex, &src_rect, &dst_rect);
    }
}

/*****************************
数字を表示
*****************************/
void pgPrintNumber(int num, int color,int bgcolor)
{
    unsigned short *vptr0, *vptr;
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
// 1バイト文字(ASCII, 半角カナ
int pgPutCharA(const unsigned char c)
{
    unsigned long index;
    unsigned short *vptr0;       //pointer to vram
    unsigned short *vptr;        //pointer to vram
    unsigned short *font;
    int cx, cy, i, b;

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
    font = (unsigned short*)(fontA + (index<<5));
    cx = *font++;
    if ((s2ch.pgCursorX + cx) >= sChar.width) {
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
                *vptr=sChar.color;
            } else {
                *vptr=sChar.bgcolor;
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
// 2バイト文字
int pgPutCharW(unsigned char hi,unsigned char lo)
{
    unsigned long index;
    unsigned short *vptr0;
    unsigned short *vptr;
    unsigned short *font;
    int cx, cy, i, b;

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

    font = (unsigned short*)(fontJ + (index<<5));
    cx = *font++;
    if ((s2ch.pgCursorX + cx) >= sChar.width) {
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
                *vptr=sChar.color;
            } else {
                *vptr=sChar.bgcolor;
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

/*****************************
実体参照を変換
*****************************/
int pgSpecialChars(char** string)
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
                return pgPutCharW(hi, lo);
            }
            else if (hi) {
                return pgPutCharA(hi);
            }
            // 変換できない文字
            else {
                return pgPutCharA('?');
            }
        }
    }
    else {
        for (i = 0; i < MAX_ENTITIES; i++) {
            if (strstr(*string, entity[i].str) == *string) {
                (*string) += entity[i].len;
                if (entity[i].byte == 1) {
                    return pgPutCharA(entity[i].c1);
                }
                else {
                    return pgPutCharW(entity[i].c1, entity[i].c2);
                }
            }
        }
    }
    return pgPutCharA('&');
}

/*****************************
文字列strを画面幅widthで1行分表示して改行部分のポインタを返す
strを全部表示したらNULLを返す
*****************************/
char* pgPrint(char *str,unsigned short color,unsigned short bgcolor, int width)
{
    unsigned char ch = 0,bef = 0;
    int ret = 0;

    sChar.color = color;
    sChar.bgcolor = bgcolor;
    sChar.width = width;
    while(*str) {
        ch = (unsigned char)*str;
        if (bef!=0) {
            ret = pgPutCharW(bef, ch);
            if (ret) { // 改行部の位置を返す
                return --str;
            }
            bef=0;
        } else {
            if (((ch>=0x80) && (ch<0xa0)) || (ch>=0xe0)) {
                bef = ch;
            } else {
                if (ch == '&') {
                    ret = pgSpecialChars((char**)(&str));
                }
                else if (ch == '\n') {
                    ret = 1;
                    str++;
                }
                else {
                    ret = pgPutCharA(ch);
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
char* pgPrintHtml(char *str, S_2CH_RES_COLOR *c, int startX, int width,int drawLine)
{
    static int anchorOn = 0;
    unsigned char ch = 0,bef = 0;
    int ret = 0;
    char *p;
    int i, j, start, end;

    sChar.bgcolor = c->bg;
    sChar.width = width;
    if (anchorOn)
    {
        sChar.color = c->link;
    }
    else
    {
        sChar.color = c->text;
    }
    while(*str) {
        ch = (unsigned char)*str;
        if (bef!=0) {
            ret = pgPutCharW(bef, ch);
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
                sChar.color = c->text;
            } else {
                if (ch >= 0xa0 && ch < 0xe0) {
                    anchorOn = 0;
                    sChar.color = c->text;
                }
                else if (anchorOn == 2) {
                    if ((ch < '0' || ch > '9') && ch != '-' && ch != ',' && ch !='<') {
                        anchorOn = 0;
                        sChar.color = c->text;
                        resEnd();
                    }
                }
                else if (anchorOn == 1) {
                    if ((ch >= '#' && ch <= '+') || (ch >= '-' && ch <= ';') || (ch >= '?' && ch <= 'Z') ||
                            (ch >= 'a' && ch <= 'z') || ch == '!' || ch == '=' || ch == '_' || ch == '~') {
                    }
                    else {
                        anchorOn = 0;
                        sChar.color = c->text;
                        urlEnd();
                    }
                }

                if (strstr(str, "http://") == str) {
                    s2ch.urlAnchor[s2ch.urlAnchorCount].x1 = s2ch.pgCursorX;
                    s2ch.urlAnchor[s2ch.urlAnchorCount].line = drawLine;
                    sChar.color = c->link;
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
                        sChar.color = c->link;
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
                    sChar.color = c->text;
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
                            sChar.color = c->link;
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
                            sChar.color = c->link;
                        }
                    }
                    ret = pgSpecialChars((char**)(&str));
                }
                else if (ch == ' ' || ch == '\n') {
                    while (ch == ' ' || ch == '\n') {
                        ch = *(++str);
                    }
                    anchorOn = 0;
                    sChar.color = c->text;
                    str--;
                    ret = pgPutCharA(' ');
                }
                else {
                    ret = pgPutCharA(ch);
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
    pgPrint("　ﾟ(　)－", BLACK, WHITE, SCR_WIDTH);
    s2ch.pgCursorX = 200;
    s2ch.pgCursorY += LINE_PITCH;
    pgPrint("./　>", BLACK, WHITE, SCR_WIDTH);
    s2ch.pgCursorX = 0;
    s2ch.pgCursorY += LINE_PITCH * 2;
    pgPrint("　　　＿＿＿_∧∧　　／￣￣￣￣￣￣￣￣", BLACK, WHITE, SCR_WIDTH);
    s2ch.pgCursorX = 0;
    s2ch.pgCursorY += LINE_PITCH;
    pgPrint("　～'　＿＿__(,,ﾟДﾟ)＜　", BLACK, WHITE, SCR_WIDTH);
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
    pgPrint("　～'　＿＿__(,,ﾟДﾟ)＜　", BLACK, WHITE, SCR_WIDTH);
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
