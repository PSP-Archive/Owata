/*
* $Id$
*/

#include "pspdialogs.h"
#include <stdio.h>
#include <pspctrl.h>
#include "pg.h"
#include "psp2chRes.h"
#include "psp2chResWindow.h"
#include "psp2chImageView.h"

extern int running; //main.c
extern char cwDir[256]; //main.c
extern unsigned long pgCursorX, pgCursorY; // pg.c
extern unsigned int pixels[BUF_WIDTH*BUF_HEIGHT]; // pg.c
extern unsigned int winPixels[BUF_WIDTH*BUF_HEIGHT]; // pg.c
extern unsigned int* printBuf; // pg.c
extern void* framebuffer; // pg.c
extern int tateFlag; // psp2ch.c
extern SceCtrlData pad; // psp2ch.c
extern SceCtrlData oldPad; // psp2ch.c
extern MESSAGE_HELPER mh; // psp2ch.c
extern S_2CH_HEADER_COLOR resAHeaderColor; // psp2ch.c
extern S_2CH_RES_COLOR resAColor; // psp2ch.c
extern S_2CH_BAR_COLOR resABarColor; // psp2ch.c
extern int preLine; // psp2chRes.c
extern S_2CH_RES* resList; // psp2chRes.c
extern S_2CH_SCREEN res; // psp2chRes.c
extern S_2CH_URL_ANCHOR urlAnchor[50]; // psp2chRes.c
extern S_2CH_RES_ANCHOR resAnchor[50]; // psp2chRes.c
extern S_2CH_ID_ANCHOR idAnchor[40]; // psp2chRes.c
extern int urlAnchorCount; // psp2chRes.c
extern int resAnchorCount; // psp2chRes.c
extern int idAnchorCount; // psp2chRes.c

static const char* cacheDir = "CACHE";

/*****************************
���X�̃E�B���h�E�\��
lineFlag:0=�ύX�Ȃ�; 1=����1�s�ǉ�; 2=���1�s�ǉ�; ���̑�:�S��ʍĕ`��
psp2chResAnchor()��psp2chIdAnchor()�Ŏg�p
*****************************/
void psp2chDrawResAnchor(S_2CH_RES_ANCHOR a, S_2CH_SCREEN anchor, int lineFlag)
{
    int re;
    int skip;
    int line = 0, i, drawLine;
    int startX, startY, scrW, lineEnd;

    if (tateFlag)
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
            if (resList[a.res[i]].ng == 0)
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
        pgCursorX = startX;
        pgCursorY = (LINE_PITCH*drawLine+startY)&0x01FF;
        line = psp2chDrawResHeader(a.res[re], &skip, lineEnd, lineEnd, startX, scrW+startX, resAColor, resAHeaderColor, &drawLine);
        if (line > lineEnd)
        {
            return;
        }
        line = psp2chDrawResText(a.res[re], &skip, lineEnd, lineEnd, startX, scrW+startX, resAColor, &drawLine);
    }
    else if (lineFlag == 2)
    {
        skip = drawLine;
        i = 0;
        while (skip >= 0)
        {
            if (resList[a.res[i]].ng == 0)
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
        pgCursorX = startX;
        pgCursorY = (LINE_PITCH*drawLine+startY)&0x01FF;
        line = psp2chDrawResHeader(a.res[re], &skip, lineEnd, lineEnd, startX, scrW+startX, resAColor, resAHeaderColor, &drawLine);
        if (line > lineEnd)
        {
            return;
        }
        line = psp2chDrawResText(a.res[re], &skip, lineEnd, lineEnd, startX, scrW+startX, resAColor, &drawLine);
    }
    else
    {
        skip = drawLine;
        i = 0;
        while (skip >= 0)
        {
            if (resList[a.res[i]].ng == 0)
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
        pgCursorX = startX;
        pgCursorY = (LINE_PITCH*drawLine+startY)&0x01FF;
        resAnchorCount = 0;
        resAnchor[0].x1 = 0;
        line = 0;
        if (resList[a.res[re]].ng)
        {
            pgFillvram(resAColor.bg, startX-2, pgCursorY, scrW+2, (lineEnd - line + 1)*LINE_PITCH);
            return;
        }
        while (line <= lineEnd)
        {
            line = psp2chDrawResHeader(a.res[re], &skip, line, lineEnd, startX, scrW+startX, resAColor, resAHeaderColor, &drawLine);
            if (line > lineEnd)
            {
                break;
            }
            line = psp2chDrawResText(a.res[re], &skip, line, lineEnd, startX, scrW+startX, resAColor, &drawLine);
            while (++re < a.resCount && a.res[re] < res.count && resList[a.res[re]].ng)
            {
            }
            if (re >= a.resCount || a.res[re] >= res.count)
            {
                pgFillvram(resAColor.bg, startX-2, pgCursorY, scrW+2, (lineEnd - line + 1)*LINE_PITCH);
                break;
            }
        }
    }
}

/**************
���X�A���J�[�\��
***************/
void psp2chResAnchor(int anc)
{
    int i, j, lineFlag, drawLine;
    S_SCROLLBAR bar;
    S_2CH_SCREEN anchor;
    S_2CH_RES_ANCHOR a = resAnchor[anc];
    int startX, startY, scrX, scrY, lineEnd, barW;

    if (a.resCount <= 0)
    {
        return;
    }
    if (tateFlag)
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
    while (running)
    {
        if(sceCtrlPeekBufferPositive(&pad, 1))
        {
            psp2chCursorSet(&anchor, lineEnd);
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
            if (pad.Buttons != oldPad.Buttons)
            {
                oldPad = pad;
                if(pad.Buttons & PSP_CTRL_CROSS)
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
            pgScrollbar(bar, resABarColor);
            pgMenuBar("�@�~ : �߂�@�@�@");
            sceDisplayWaitVblankStart();
            framebuffer = sceGuSwapBuffers();
        }
    }
    return;
}

/**************
ID���o
***************/
void psp2chIdAnchor(int anc)
{
    int i, j, lineFlag, drawLine;
    S_SCROLLBAR bar;
    S_2CH_SCREEN anchor;
    S_2CH_RES_ANCHOR a;
    int startX, startY, scrX, scrY, lineEnd, barW;

    if (tateFlag)
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
    for (i = 0; i < res.count; i++)
    {
        if (resList[i].id && strcmp(resList[i].id, idAnchor[anc].id) == 0)
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
    while (running)
    {
        if(sceCtrlPeekBufferPositive(&pad, 1))
        {
            psp2chCursorSet(&anchor, lineEnd);
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
            if (pad.Buttons != oldPad.Buttons)
            {
                oldPad = pad;
                if(pad.Buttons & PSP_CTRL_CROSS)
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
            pgScrollbar(bar, resABarColor);
            pgMenuBar("�@�~ : �߂�@�@�@");
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

    sprintf(path, "%s/%s", cwDir, cacheDir);
    if ((fd = sceIoDopen(path)) < 0)
    {
        if (sceIoMkdir(path, 0777) < 0)
        {
            memset(&mh,0,sizeof(MESSAGE_HELPER));
            sprintf(mh.message, "Make dir error\n%s", path);
            pspShowMessageDialog(&mh, DIALOG_LANGUAGE_AUTO);
            sceCtrlPeekBufferPositive(&oldPad, 1);
            return -1;
        }
    }
    else
    {
        sceIoDclose(fd);
    }
    sprintf(path, "%s/%s/%s", cwDir, cacheDir, title);
    if ((fd = sceIoDopen(path)) < 0)
    {
        if (sceIoMkdir(path, 0777) < 0)
        {
            memset(&mh,0,sizeof(MESSAGE_HELPER));
            sprintf(mh.message, "Make dir error\n%s", path);
            pspShowMessageDialog(&mh, DIALOG_LANGUAGE_AUTO);
            sceCtrlPeekBufferPositive(&oldPad, 1);
            return -1;
        }
    }
    else
    {
        sceIoDclose(fd);
    }
    p = strrchr(urlAnchor[anchor].path, '#');
    if (p)
    {
        *p = '\0';
    }
    p = strrchr(urlAnchor[anchor].path, '.');
    if (p)
    {
        memcpy(ext, p, 15);
        ext[15] = '\0';
    }
    else
    {
        ext[0] = '\0';
    }
    sprintf(path, "%s/%s/%s/%X_%X_%X_%d%s", cwDir, cacheDir, title, dat, urlAnchor[anchor].line, urlAnchor[anchor].x1, tateFlag, ext);
    fd = sceIoOpen(path, PSP_O_RDONLY, 0777);
    if (fd >= 0)
    {
        sceIoClose(fd);
        if ((ext[1] == 'j' || ext[1] == 'J') && (ext[2] == 'p' || ext[2] == 'P') && (ext[3] == 'g' || ext[3] == 'G'))
        {
            psp2chImageViewJpeg(path);
            sceCtrlPeekBufferPositive(&oldPad, 1);
        }
        else if ((ext[1] == 'p' || ext[1] == 'P') && (ext[2] == 'n' || ext[2] == 'N') && (ext[3] == 'g' || ext[3] == 'G'))
        {
            psp2chImageViewPng(path);
            sceCtrlPeekBufferPositive(&oldPad, 1);
        }
        return 0;
    }
    mySocket = psp2chRequest(urlAnchor[anchor].host, urlAnchor[anchor].path, "");
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
            memset(&mh,0,sizeof(MESSAGE_HELPER));
            sprintf(mh.message, "HTTP error\nhost %s path %s\nStatus code %d", urlAnchor[anchor].host, urlAnchor[anchor].path, ret);
            pspShowMessageDialog(&mh, DIALOG_LANGUAGE_AUTO);
            sceCtrlPeekBufferPositive(&oldPad, 1);
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
        memset(&mh,0,sizeof(MESSAGE_HELPER));
        sprintf(mh.message, "File open error\n dat %d\n", dat);
        pspShowMessageDialog(&mh, DIALOG_LANGUAGE_AUTO);
        sceCtrlPeekBufferPositive(&oldPad, 1);
        return fd;
    }
    sprintf(path, "http://%s/%s ����f�[�^��]�����Ă��܂�...", urlAnchor[anchor].host, urlAnchor[anchor].path);
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
    sprintf(path, "%s/%s/%s/%X_%X_%X_%d%s", cwDir, cacheDir, title, dat, urlAnchor[anchor].line, urlAnchor[anchor].x1, tateFlag, ext);
    if ((ext[1] == 'j' || ext[1] == 'J') && (ext[2] == 'p' || ext[2] == 'P') && (ext[3] == 'g' || ext[3] == 'G'))
    {
        psp2chImageViewJpeg(path);
        sceCtrlPeekBufferPositive(&oldPad, 1);
    }
    else if ((ext[1] == 'p' || ext[1] == 'P') && (ext[2] == 'n' || ext[2] == 'N') && (ext[3] == 'g' || ext[3] == 'G'))
    {
        psp2chImageViewPng(path);
        sceCtrlPeekBufferPositive(&oldPad, 1);
    }
    return 0;
}