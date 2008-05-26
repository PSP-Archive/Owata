/*
* $Id$
*/

#include <stdio.h>
#include <malloc.h>
#include "pg.h"
#include "psp2ch.h"
#include "psp2chNet.h"
#include "psp2chRes.h"
#include "psp2chResWindow.h"
#include "psp2chImageView.h"
#include "psp2chTinyBrowser.h"

extern S_2CH s2ch; // psp2ch.c
extern int preLine; // psp2chRes.c
extern unsigned int pixels[BUF_WIDTH*BUF_HEIGHT]; // pg.c
extern unsigned int winPixels[BUF_WIDTH*BUF_HEIGHT]; // pg.c
extern unsigned int* printBuf; // pg.c

static const char* cacheDir = "CACHE";

/*****************************
レスのウィンドウ表示
lineFlag:0=変更なし; 1=下に1行追加; 2=上に1行追加; その他:全画面再描画
psp2chResAnchor()とpsp2chIdAnchor()で使用
*****************************/
void psp2chDrawResAnchor(S_2CH_RES_ANCHOR a, S_2CH_SCREEN anchor, int lineFlag)
{
    int re;
    int skip;
    int line = 0, i, drawLine;
    int startX, startY, scrW, lineEnd;

    if (s2ch.tateFlag)
    {
        startX = RES_A_X_V;
        scrW = RES_A_WIDTH_V;
        startY = RES_A_Y_V+1;
        lineEnd = RES_A_LINE_V;
    }
    else
    {
        startX = RES_A_X;
        scrW = RES_A_WIDTH;
        startY = RES_A_Y;
        lineEnd = RES_A_LINE;
    }
    //pspDebugScreenInit();
    //pspDebugScreenPrintf("res=%d\n", a.res[re]);
    //pgWaitVn(200);
    drawLine = anchor.select;
    if (lineFlag == 0 && drawLine)
    {
        return;
    }
    else if (lineFlag == 1)
    {
        drawLine += lineEnd-1;
        skip = drawLine;
        i = 0;
        while (skip >= 0)
        {
            if (s2ch.resList[a.res[i]].ng == 0)
            {
                skip -= psp2chCountRes(a.res[i], scrW);
                skip--;
            }
            i++;
            if (i >= a.resCount)
            {
                break;
            }
        }
        re = --i;
        skip++;
        skip += psp2chCountRes(a.res[i], scrW);
        s2ch.pgCursorX = startX;
        s2ch.pgCursorY = (LINE_PITCH*drawLine+startY)&0x01FF;
        line = psp2chDrawResHeader(a.res[re], &skip, lineEnd, lineEnd, startX, scrW+startX, s2ch.resAColor, s2ch.resAHeaderColor, &drawLine);
        if (line > lineEnd)
        {
            return;
        }
        line = psp2chDrawResText(a.res[re], &skip, lineEnd, lineEnd, startX, scrW+startX, s2ch.resAColor, &drawLine);
    }
    else if (lineFlag == 2)
    {
        skip = drawLine;
        i = 0;
        while (skip >= 0)
        {
            if (s2ch.resList[a.res[i]].ng == 0)
            {
                skip -= psp2chCountRes(a.res[i], scrW);
                skip--;
            }
            i++;
            if (i >= a.resCount)
            {
                break;
            }
        }
        re = --i;
        skip++;
        skip += psp2chCountRes(a.res[i], scrW);
        s2ch.pgCursorX = startX;
        s2ch.pgCursorY = (LINE_PITCH*drawLine+startY)&0x01FF;
        line = psp2chDrawResHeader(a.res[re], &skip, lineEnd, lineEnd, startX, scrW+startX, s2ch.resAColor, s2ch.resAHeaderColor, &drawLine);
        if (line > lineEnd)
        {
            return;
        }
        line = psp2chDrawResText(a.res[re], &skip, lineEnd, lineEnd, startX, scrW+startX, s2ch.resAColor, &drawLine);
    }
    else
    {
        skip = drawLine;
        i = 0;
        while (skip >= 0)
        {
            if (s2ch.resList[a.res[i]].ng == 0)
            {
                skip -= psp2chCountRes(a.res[i], scrW);
                skip--;
            }
            i++;
            if (i >= a.resCount)
            {
                break;
            }
        }
        re = --i;
        skip++;
        skip += psp2chCountRes(a.res[i], scrW);
        s2ch.pgCursorX = startX;
        s2ch.pgCursorY = (LINE_PITCH*drawLine+startY)&0x01FF;
        s2ch.resAnchorCount = 0;
        s2ch.resAnchor[0].x1 = 0;
        line = 0;
        if (s2ch.resList[a.res[re]].ng)
        {
            pgFillvram(s2ch.resAColor.bg, startX-2, s2ch.pgCursorY, scrW+2, (lineEnd - line + 1)*LINE_PITCH, 2);
            return;
        }
        while (line <= lineEnd)
        {
            line = psp2chDrawResHeader(a.res[re], &skip, line, lineEnd, startX, scrW+startX, s2ch.resAColor, s2ch.resAHeaderColor, &drawLine);
            if (line > lineEnd)
            {
                break;
            }
            line = psp2chDrawResText(a.res[re], &skip, line, lineEnd, startX, scrW+startX, s2ch.resAColor, &drawLine);
            while (++re < a.resCount && a.res[re] < s2ch.res.count && s2ch.resList[a.res[re]].ng)
            {
            }
            if (re >= a.resCount || a.res[re] >= s2ch.res.count)
            {
                pgFillvram(s2ch.resAColor.bg, startX-2, s2ch.pgCursorY, scrW+2, (lineEnd - line + 1)*LINE_PITCH, 2);
                break;
            }
        }
    }
}

/**************
レスアンカー表示
***************/
void psp2chResAnchor(int anc)
{
    int i, j, lineFlag, drawLine, change;
    S_SCROLLBAR bar;
    S_2CH_SCREEN anchor;
    S_2CH_RES_ANCHOR a = s2ch.resAnchor[anc];
    int startX, startY, scrX, scrY, lineEnd, barW;

    if (a.resCount <= 0)
    {
        return;
    }
    if (s2ch.tateFlag)
    {
        startX = RES_A_X_V;
        scrX = RES_A_WIDTH_V;
        startY = RES_A_Y_V;
        scrY = RES_A_HEIGHT_V;
        lineEnd = RES_A_LINE_V;
        barW = RES_BAR_WIDTH_V;
        bar.view = RES_A_HEIGHT_V;
        bar.x = startX + scrX;
        bar.y = RES_A_Y_V;
        bar.w = RES_BAR_WIDTH_V;
        bar.h = RES_A_HEIGHT_V;
    }
    else
    {
        startX = RES_A_X;
        scrX = RES_A_WIDTH;
        startY = RES_A_Y;
        scrY = RES_A_HEIGHT;
        lineEnd = RES_A_LINE;
        barW = RES_BAR_WIDTH;
        bar.view = RES_A_HEIGHT;
        bar.x = startX + scrX;
        bar.y = RES_A_Y;
        bar.w = RES_BAR_WIDTH;
        bar.h = RES_A_HEIGHT;
    }
    for (i = 0, j=0; i < a.resCount; i++)
    {
        j += psp2chCountRes(a.res[i], scrX);
        j++;
    }
    if (j < lineEnd)
    {
        j = lineEnd;
    }
    anchor.count = j;
    bar.total = j * LINE_PITCH;
    bar.start = 0;
    anchor.start = 0;
    anchor.select = 0;
    printBuf = winPixels;
    drawLine = anchor.select;
    pgPrintMenuBar("　× : 戻る　　　");
    while (s2ch.running)
    {
        if(sceCtrlPeekBufferPositive(&s2ch.pad, 1))
        {
            if (s2ch.tateFlag)
            {
                psp2chCursorSet(&anchor, lineEnd, s2ch.btnResV.change, &change);
            }
            else
            {
                psp2chCursorSet(&anchor, lineEnd, s2ch.btnResH.change, &change);
            }
            if (anchor.select > anchor.count - lineEnd)
            {
                anchor.select = anchor.count - lineEnd;
            }
            anchor.start = anchor.select;
            if(anchor.select == drawLine)
            {
                lineFlag = 0;
            }
            else if(anchor.select == drawLine + 1)
            {
                lineFlag = 1;
            }
            else if(anchor.select == drawLine - 1)
            {
                lineFlag = 2;
            }
            else
            {
                lineFlag = 3;
            }
            drawLine = anchor.select;
            if (s2ch.pad.Buttons != s2ch.oldPad.Buttons)
            {
                s2ch.oldPad = s2ch.pad;
                if(s2ch.pad.Buttons & PSP_CTRL_CROSS)
                {
                    printBuf = pixels;
                    preLine = -2;
                    return;
                }
            }
			if (change)
			{
	            psp2chDrawResAnchor(a, anchor, lineFlag);
			}
            pgCopyWindow(anchor.start * LINE_PITCH, startX, startY, scrX, scrY);
            pgWindowFrame(startX, startY, startX + scrX + barW, startY + scrY);
            bar.start = anchor.start * LINE_PITCH;
            pgScrollbar(bar, s2ch.resABarColor);
            pgCopyMenuBar();
            sceDisplayWaitVblankStart();
            framebuffer = sceGuSwapBuffers();
        }
    }
    return;
}

/**************
ID抽出
***************/
void psp2chIdAnchor(int anc)
{
    int i, j, lineFlag, drawLine, change;
    S_SCROLLBAR bar;
    S_2CH_SCREEN anchor;
    S_2CH_RES_ANCHOR a;
    int startX, startY, scrX, scrY, lineEnd, barW;

    if (s2ch.tateFlag)
    {
        startX = RES_A_X_V;
        scrX = RES_A_WIDTH_V;
        startY = RES_A_Y_V;
        scrY = RES_A_HEIGHT_V;
        lineEnd = RES_A_LINE_V;
        barW = RES_BAR_WIDTH_V;
        bar.view = RES_A_HEIGHT_V;
        bar.x = startX + scrX;
        bar.y = RES_A_Y_V;
        bar.w = RES_BAR_WIDTH_V;
        bar.h = RES_A_HEIGHT_V;
    }
    else
    {
        startX = RES_A_X;
        scrX = RES_A_WIDTH;
        startY = RES_A_Y;
        scrY = RES_A_HEIGHT;
        lineEnd = RES_A_LINE;
        barW = RES_BAR_WIDTH;
        bar.view = RES_A_HEIGHT;
        bar.x = startX + scrX;
        bar.y = RES_A_Y;
        bar.w = RES_BAR_WIDTH;
        bar.h = RES_A_HEIGHT;
    }
    a.resCount = 0;
    for (i = 0; i < s2ch.res.count; i++)
    {
        if (s2ch.resList[i].id && strcmp(s2ch.resList[i].id, s2ch.idAnchor[anc].id) == 0)
        {
            a.res[a.resCount] = i;
            a.resCount++;
        }
    }
    for (i = 0, j=0; i < a.resCount; i++)
    {
        j += psp2chCountRes(a.res[i], scrX);
        j++;
    }
    if (j < lineEnd)
    {
        j = lineEnd;
    }
    anchor.count = j;
    bar.total = j * LINE_PITCH;
    bar.start = 0;
    anchor.start = 0;
    anchor.select = 0;
    printBuf = winPixels;
    drawLine = anchor.select;
    pgPrintMenuBar("　× : 戻る　　　");
    while (s2ch.running)
    {
        if(sceCtrlPeekBufferPositive(&s2ch.pad, 1))
        {
            psp2chCursorSet(&anchor, lineEnd, 0, &change);
            if (anchor.select > anchor.count - lineEnd)
            {
                anchor.select = anchor.count - lineEnd;
            }
            anchor.start = anchor.select;
            if(anchor.select == drawLine)
            {
                lineFlag = 0;
            }
            else if(anchor.select == drawLine + 1)
            {
                lineFlag = 1;
            }
            else if(anchor.select == drawLine - 1)
            {
                lineFlag = 2;
            }
            else
            {
                lineFlag = 3;
            }
            drawLine = anchor.select;
            if (s2ch.pad.Buttons != s2ch.oldPad.Buttons)
            {
                s2ch.oldPad = s2ch.pad;
                if(s2ch.pad.Buttons & PSP_CTRL_CROSS)
                {
                    printBuf = pixels;
                    preLine = -2;
                    return;
                }
            }
			if (change)
			{
	            psp2chDrawResAnchor(a, anchor, lineFlag);
			}
            pgCopyWindow(anchor.start * LINE_PITCH, startX, startY, scrX, scrY);
            pgWindowFrame(startX, startY, startX + scrX + barW, startY + scrY);
            bar.start = anchor.start * LINE_PITCH;
            pgScrollbar(bar, s2ch.resABarColor);
            pgCopyMenuBar();
            sceDisplayWaitVblankStart();
            framebuffer = sceGuSwapBuffers();
        }
    }
    return;
}

/**************
URL
***************/
int psp2chUrlAnchor(int anchor, int offset)
{
    SceUID fd;
    int i, ret;
    S_NET net;
    char path[256], buf[256];
    char ext[8], tmp[4];
    unsigned char digest[16];
    char *p;

    sprintf(path, "%s/%s", s2ch.cwDir, cacheDir);
    if ((fd = sceIoDopen(path)) < 0)
    {
        if (sceIoMkdir(path, 0777) < 0)
        {
            psp2chErrorDialog("Make dir error\n%s", path);
            return -1;
        }
    }
    else
    {
        sceIoDclose(fd);
    }
    if (stricmp(s2ch.urlAnchor[anchor].host, "imepita.jp") == 0)
    {
        return psp2chImepita(anchor, offset);
    }
    p = strrchr(s2ch.urlAnchor[anchor].path, '#');
    if (p)
    {
        *p = '\0';
    }
    p = strrchr(s2ch.urlAnchor[anchor].path, '.');
    if (p && strlen(p) < 8)
    {
        strcpy(ext, p);
    }
    else
    {
        ext[0] = '\0';
    }
    sprintf(path, "%s/%s", s2ch.urlAnchor[anchor].host, s2ch.urlAnchor[anchor].path);
    sceKernelUtilsMd5Digest((u8*)path, strlen(path), digest);
    sprintf(path, "%s/%s/", s2ch.cwDir, cacheDir);
    for (i = 0; i < 16; i++)
    {
        sprintf(tmp, "%02x", digest[i]);
        strcat(path, tmp);
    }
    strcat(path, ext);
    fd = sceIoOpen(path, PSP_O_RDONLY, 0777);
    if (fd >= 0)
    {
        sceIoClose(fd);
        if (stricmp(ext, ".jpg") == 0)
        {
            psp2chImageViewJpeg(path);
            sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
        }
        else if (stricmp(ext, ".png") == 0)
        {
            psp2chImageViewPng(path);
            sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
        }
        else if (stricmp(ext, ".bmp") == 0)
        {
            psp2chImageViewBmp(path);
            sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
        }
        else if (stricmp(ext, ".gif") == 0)
        {
            psp2chImageViewGif(path);
            sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
        }
        else
        {
            psp2chTinyBrowser(path);
        }
        return 0;
    }
    ret = psp2chGet(s2ch.urlAnchor[anchor].host, s2ch.urlAnchor[anchor].path, "", NULL, &net);
    if (ret < 0)
    {
        return ret;
    }
    switch(net.status)
    {
    case 200: // OK
        break;
    default:
        /*
        psp2chErrorDialog("HTTP error\nhost %s path %s\nStatus code %d", s2ch.urlAnchor[anchor].host, s2ch.urlAnchor[anchor].path, ret);
        pgWaitVn(60);
        */
        free(net.body);
        return -1;
    }
    // save
    fd = sceIoOpen(path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
    if (fd < 0)
    {
        free(net.body);
        psp2chErrorDialog("File open error\n%s", path);
        return fd;
    }
    strcpy(buf, "表\示します");
    pgCopy(0, offset);
    pgPrintMenuBar(buf);
	pgCopyMenuBar();
    sceDisplayWaitVblankStart();
    framebuffer = sceGuSwapBuffers();
    pgCopy(0, offset);
    pgCopyMenuBar();
    sceDisplayWaitVblankStart();
    framebuffer = sceGuSwapBuffers();
    sceIoWrite(fd, net.body, net.length);
    free(net.body);
    sceIoClose(fd);
    if (stricmp(ext, ".jpg") == 0)
    {
        psp2chImageViewJpeg(path);
        sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
    }
    else if (stricmp(ext, ".png") == 0)
    {
        psp2chImageViewPng(path);
        sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
    }
    else if (stricmp(ext, ".bmp") == 0)
    {
        psp2chImageViewBmp(path);
        sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
    }
    else if (stricmp(ext, ".gif") == 0)
    {
        psp2chImageViewGif(path);
        sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
    }
    else
    {
        psp2chTinyBrowser(path);
    }
    return 0;
}

/**************
imepita.jpの時は画像を表示
***************/
int psp2chImepita(int anchor, int offset)
{
    SceUID fd;
    int i, ret;
    S_NET net;
    char cookie[256];
    char header[512];
    char path[256];
    char file[256];
    char tmp[4];
    unsigned char digest[16];

    sprintf(file, "%s/%s", s2ch.urlAnchor[anchor].host, s2ch.urlAnchor[anchor].path);
    sceKernelUtilsMd5Digest((u8*)file, strlen(file), digest);
    sprintf(file, "%s/%s/", s2ch.cwDir, cacheDir);
    for (i = 0; i < 16; i++)
    {
        sprintf(tmp, "%02x", digest[i]);
        strcat(file, tmp);
    }
    fd = sceIoOpen(file, PSP_O_RDONLY, 0777);
    if (fd >= 0)
    {
        sceIoClose(fd);
        // 拡張子がないので総当りで表示
        if (psp2chImageViewJpeg(file) == 0)
        {
            sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
            return 0;
        }
        if (psp2chImageViewPng(file) == 0)
        {
            sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
            return 0;
        }
        if (psp2chImageViewGif(file) == 0)
        {
            sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
            return 0;
        }
        return -1;
    }
    ret = psp2chGet(s2ch.urlAnchor[anchor].host, s2ch.urlAnchor[anchor].path, "", cookie, &net);
    if (ret < 0)
    {
        return ret;
    }
    free(net.body);
    switch(net.status)
    {
    case 200: // OK
        break;
    default:
        return -1;
    }
    sprintf(path, "image/%s", s2ch.urlAnchor[anchor].path);
    sprintf(header, "Referer: http://%s/%s\r\nCookie: %s\r\n", s2ch.urlAnchor[anchor].host, s2ch.urlAnchor[anchor].path, cookie);
    ret = psp2chGet(s2ch.urlAnchor[anchor].host, path, header, NULL, &net);
    if (ret < 0)
    {
        return ret;
    }
    switch(net.status)
    {
    case 200: // OK
        break;
    default:
        free(net.body);
        return -1;
    }
    // save
    fd = sceIoOpen(file, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
    if (fd < 0)
    {
        free(net.body);
        psp2chErrorDialog("File open error\n%s", file);
        return fd;
    }
    strcpy(header, "表\示します");
    pgCopy(0, offset);
    pgPrintMenuBar(header);
    pgCopyMenuBar();
    sceDisplayWaitVblankStart();
    framebuffer = sceGuSwapBuffers();
    pgCopy(0, offset);
    pgCopyMenuBar();
    sceDisplayWaitVblankStart();
    framebuffer = sceGuSwapBuffers();
    sceIoWrite(fd, net.body, net.length);
    free(net.body);
    sceIoClose(fd);
    if (psp2chImageViewJpeg(file) == 0)
    {
        sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
        return 0;
    }
    if (psp2chImageViewPng(file) == 0)
    {
        sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
        return 0;
    }
    if (psp2chImageViewGif(file) == 0)
    {
        sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
        return 0;
    }
    return -1;
}
