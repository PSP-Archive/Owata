/*
* $Id$
*/

#include <stdio.h>
#include "pg.h"
#include "psp2ch.h"
#include "psp2chRes.h"
#include "psp2chResWindow.h"
#include "psp2chImageView.h"

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
            pgFillvram(s2ch.resAColor.bg, startX-2, s2ch.pgCursorY, scrW+2, (lineEnd - line + 1)*LINE_PITCH);
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
                pgFillvram(s2ch.resAColor.bg, startX-2, s2ch.pgCursorY, scrW+2, (lineEnd - line + 1)*LINE_PITCH);
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
    int i, j, lineFlag, drawLine;
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
    while (s2ch.running)
    {
        if(sceCtrlPeekBufferPositive(&s2ch.pad, 1))
        {
            if (s2ch.tateFlag)
            {
                psp2chCursorSet(&anchor, lineEnd, s2ch.btnResV.change);
            }
            else
            {
                psp2chCursorSet(&anchor, lineEnd, s2ch.btnResH.change);
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
            psp2chDrawResAnchor(a, anchor, lineFlag);
            pgCopyWindow(anchor.start * LINE_PITCH, startX, startY, scrX, scrY);
            pgWindowFrame(startX, startY, startX + scrX + barW, startY + scrY);
            bar.start = anchor.start * LINE_PITCH;
            pgScrollbar(bar, s2ch.resABarColor);
            pgMenuBar("　× : 戻る　　　");
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
    int i, j, lineFlag, drawLine;
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
    while (s2ch.running)
    {
        if(sceCtrlPeekBufferPositive(&s2ch.pad, 1))
        {
            psp2chCursorSet(&anchor, lineEnd, 0);
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
            psp2chDrawResAnchor(a, anchor, lineFlag);
            pgCopyWindow(anchor.start * LINE_PITCH, startX, startY, scrX, scrY);
            pgWindowFrame(startX, startY, startX + scrX + barW, startY + scrY);
            bar.start = anchor.start * LINE_PITCH;
            pgScrollbar(bar, s2ch.resABarColor);
            pgMenuBar("　× : 戻る　　　");
            sceDisplayWaitVblankStart();
            framebuffer = sceGuSwapBuffers();
        }
    }
    return;
}

int psp2chUrlAnchor(int anchor, char* title, int dat, int offset)
{
    SceUID fd;
    int ret, mySocket, contentLength;
    HTTP_HEADERS resHeader;
    char path[256];
    char ext[16];
    char *p;

    sprintf(path, "%s/%s", s2ch.cwDir, cacheDir);
    if ((fd = sceIoDopen(path)) < 0)
    {
        if (sceIoMkdir(path, 0777) < 0)
        {
            memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
            sprintf(s2ch.mh.message, "Make dir error\n%s", path);
            pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
            sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
            return -1;
        }
    }
    else
    {
        sceIoDclose(fd);
    }
    sprintf(path, "%s/%s/%s", s2ch.cwDir, cacheDir, title);
    if ((fd = sceIoDopen(path)) < 0)
    {
        if (sceIoMkdir(path, 0777) < 0)
        {
            memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
            sprintf(s2ch.mh.message, "Make dir error\n%s", path);
            pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
            sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
            return -1;
        }
    }
    else
    {
        sceIoDclose(fd);
    }
    p = strrchr(s2ch.urlAnchor[anchor].path, '#');
    if (p)
    {
        *p = '\0';
    }
    p = strrchr(s2ch.urlAnchor[anchor].path, '.');
    if (p)
    {
        memcpy(ext, p, 15);
        ext[15] = '\0';
    }
    else
    {
        ext[0] = '\0';
    }
    sprintf(path, "%s/%s/%s/%X_%X_%X_%d%s", s2ch.cwDir, cacheDir, title, dat, s2ch.urlAnchor[anchor].line, s2ch.urlAnchor[anchor].x1, s2ch.tateFlag, ext);
    fd = sceIoOpen(path, PSP_O_RDONLY, 0777);
    if (fd >= 0)
    {
        sceIoClose(fd);
        if ((ext[1] == 'j' || ext[1] == 'J') && (ext[2] == 'p' || ext[2] == 'P') && (ext[3] == 'g' || ext[3] == 'G'))
        {
            psp2chImageViewJpeg(path);
            sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
        }
        else if ((ext[1] == 'p' || ext[1] == 'P') && (ext[2] == 'n' || ext[2] == 'N') && (ext[3] == 'g' || ext[3] == 'G'))
        {
            psp2chImageViewPng(path);
            sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
        }
        return 0;
    }
    mySocket = psp2chRequest(s2ch.urlAnchor[anchor].host, s2ch.urlAnchor[anchor].path, "");
    if (mySocket < 0)
    {
        return mySocket;
    }
    ret = psp2chGetStatusLine(mySocket);
    switch(ret)
    {
        case 200: // OK
            break;
        default:
            /*
            memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
            sprintf(s2ch.mh.message, "HTTP error\nhost %s path %s\nStatus code %d", s2ch.urlAnchor[anchor].host, s2ch.urlAnchor[anchor].path, ret);
            pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
            sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
            */
            psp2chCloseSocket(mySocket);
            pgWaitVn(60);
            return -1;
    }
    // Receive and Save dat
    contentLength = psp2chGetHttpHeaders(mySocket, &resHeader);
    if (contentLength < 0)
    {
        psp2chCloseSocket(mySocket);
        return -1;
    }
    // save
    fd = sceIoOpen(path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
    if (fd < 0)
    {
        psp2chCloseSocket(mySocket);
        memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
        sprintf(s2ch.mh.message, "File open error\n dat %d\n", dat);
        pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
        sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
        return fd;
    }
    sprintf(path, "http://%s/%s からデータを転送しています...", s2ch.urlAnchor[anchor].host, s2ch.urlAnchor[anchor].path);
    pgCopy(0, offset);
    pgMenuBar(path);
    sceDisplayWaitVblankStart();
    framebuffer = sceGuSwapBuffers();
    while((ret = recv(mySocket, path, sizeof(path), 0)) > 0)
    {
        sceIoWrite(fd, path, ret);
    }
    psp2chCloseSocket(mySocket);
    sceIoClose(fd);
    sprintf(path, "%s/%s/%s/%X_%X_%X_%d%s", s2ch.cwDir, cacheDir, title, dat, s2ch.urlAnchor[anchor].line, s2ch.urlAnchor[anchor].x1, s2ch.tateFlag, ext);
    if ((ext[1] == 'j' || ext[1] == 'J') && (ext[2] == 'p' || ext[2] == 'P') && (ext[3] == 'g' || ext[3] == 'G'))
    {
        psp2chImageViewJpeg(path);
        sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
    }
    else if ((ext[1] == 'p' || ext[1] == 'P') && (ext[2] == 'n' || ext[2] == 'N') && (ext[3] == 'g' || ext[3] == 'G'))
    {
        psp2chImageViewPng(path);
        sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
    }
    return 0;
}
