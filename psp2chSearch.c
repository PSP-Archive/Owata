/*
* $Id$
*/

#include "pspdialogs.h"
#include <stdio.h>
#include <malloc.h>
#include <time.h>
#include <pspctrl.h>
#include "pg.h"
#include "psp2chSearch.h"
#include "psp2chIta.h"
#include "psp2chThread.h"
#include "psp2chFavorite.h"
#include "psp2chRes.h"
#include "psp2chForm.h"

extern int sel; // psp2ch.c
extern int tateFlag; // psp2ch.c
extern SceCtrlData pad; // psp2ch.c
extern SceCtrlData oldPad; // psp2ch.c
extern MESSAGE_HELPER mh; // psp2ch.c
extern void* framebuffer; // pg.c
extern unsigned long pgCursorX, pgCursorY; // pg.c
extern char keyWords[128]; // psp2chThread.c
extern int preLine; // psp2chRes.c
extern S_2CH_RES* resList; // psp2chRes.c
extern S_2CH_ITA* itaList; // psp2chIta.c
extern S_2CH_SCREEN ita; // psp2chIta.c
extern S_2CH_THREAD_COLOR threadColor; // psp2ch.c

S_2CH_FAVORITE* findList;
S_2CH_SCREEN find;

int psp2chSearch(int retSel)
{
    static int scrollX = 0;
    static char* menuStr = "";
    static int ret = 0;
    int rMenu;

    if (findList == NULL)
    {
        if (psp2chSearchList() < 0)
        {
            sel = retSel;
            return 0;
        }
        ret = retSel;
        find.start = 0;
        find.select = 0;
    }
    if(sceCtrlPeekBufferPositive(&pad, 1))
    {
        rMenu = psp2chCursorSet(&find);
        if (rMenu)
        {
            menuStr = "　↑ : 先頭　　　↓ : 最後　　　　□ : 全板検索";
        }
        else
        {
            menuStr = "　○ : 決定　　　　　□ : 板一覧　　　　△ : お気に入り　　　　× : 戻る　　　R : メニュー切替";
        }
        if (pad.Buttons != oldPad.Buttons)
        {
            oldPad = pad;
            if (pad.Buttons & PSP_CTRL_SELECT)
            {
                tateFlag = (tateFlag) ? 0 : 1;
            }
            else if(pad.Buttons & PSP_CTRL_CIRCLE)
            {
                if (rMenu)
                {
                }
                else
                {
                    free(resList);
                    resList = NULL;
                    preLine = -2;
                    pgFillvram(WHITE, 0, 0, SCR_WIDTH, BUF_HEIGHT);
                    sel = 8;
                    return 0;
                }
            }
            else if(pad.Buttons & PSP_CTRL_CROSS)
            {
                if (rMenu)
                {
                }
                else
                {
                    sel = ret;
                }
            }
            else if(pad.Buttons & PSP_CTRL_TRIANGLE)
            {
                if (rMenu)
                {
                }
                else
                {
                    sel = 1;
                }
            }
            else if(pad.Buttons & PSP_CTRL_SQUARE)
            {
                if (rMenu)
                {
                    if (psp2chThreadSearch() == 0 && keyWords[0])
                    {
                        if (findList)
                        {
                            free(findList);
                            findList = NULL;
                        }
                        return 0;
                    }
                }
                else
                {
                    sel = 2;
                }
            }
        }
        scrollX = psp2chPadSet(scrollX);
        psp2chDrawSearch(scrollX);
        pgCopy(scrollX, 0);
        pgMenuBar(menuStr);
        sceDisplayWaitVblankStart();
        framebuffer = sceGuSwapBuffers();
    }
    return 0;
}

void psp2chSjisToEuc(char* dst, char* src)
{
    unsigned char hi, lo;

    while (*src)
    {
        // 半角カナ
        if ((unsigned char)*src > 0xA0 && (unsigned char)*src < 0xE0)
        {
            *dst++ = 0x8E;
            *dst++ = *src++;
        }
        // 2バイト文字
        else if ((unsigned char)*src > 0x7F)
        {
            hi = *src++;
            lo = *src++;
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
            *dst++ = hi | 0x80;
            *dst++ = lo | 0x80;
        }
        // ASCII文字
        else
        {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
}

void psp2chEucToSjis(char* dst, char* src)
{
    unsigned char hi, lo;

    while (*src)
    {
        // 半角カナ
        if ((unsigned char)*src == 0x8E)
        {
            src++;
            *dst = *src++;
        }
        // 2バイト文字
        else if ((unsigned char)*src > 0x7F)
        {
            hi = *src++ & 0x7F;
            lo = *src++ & 0x7F;
            hi -= 0x21;
            if (hi & 1)
            {
                lo += 0x7E;
            }
            else
            {
                lo += 0x1F;
                if (lo>= 0x7F)
                {
                    lo++;
                }
            }
            hi >>= 1;
            hi += (hi <= 0x1E) ? 0x81 : 0xC1;
            *dst++ = hi;
            *dst++ = lo;
        }
        // ASCII文字
        else
        {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
}

#define FIND_MAX_COUNT (50)
int psp2chSearchList(void)
{
    const char* findHost = "find.2ch.net";
    char euc[128];
    char buf[128*3];
    char query[512];
    int count, offset;
    int mySocket, ret, i;
    HTTP_HEADERS resHeader;
    char in;
    char *p, *q;

    count = FIND_MAX_COUNT;
    offset = 0;
    keyWords[127] = '\0';
    psp2chSjisToEuc(euc, keyWords);
    psp2chUrlEncode(buf, euc);
    sprintf(query, "?STR=%s&COUNT=%d&OFFSET=%d", buf, count, offset);
    mySocket = psp2chRequest(findHost, query, "");
    if (mySocket < 0)
    {
        return -1;
    }
    ret = psp2chGetStatusLine(mySocket);
    switch(ret)
    {
        case 200: // OK
            break;
        default:
            memset(&mh,0,sizeof(MESSAGE_HELPER));
            sprintf(mh.message, "HTTP error\nhost %s\nStatus code %d", findHost, ret);
            pspShowMessageDialog(&mh, DIALOG_LANGUAGE_AUTO);
            sceCtrlPeekBufferPositive(&oldPad, 1);
            psp2chCloseSocket(mySocket);
            return -1;
    }
    ret = psp2chGetHttpHeaders(mySocket, &resHeader);
    findList = (S_2CH_FAVORITE*)realloc(findList, sizeof(S_2CH_FAVORITE) * FIND_MAX_COUNT);
    if (findList == NULL)
    {
        memset(&mh,0,sizeof(MESSAGE_HELPER));
        strcpy(mh.message, "memorry error");
        pspShowMessageDialog(&mh, DIALOG_LANGUAGE_AUTO);
        sceCtrlPeekBufferPositive(&oldPad, 1);
        psp2chCloseSocket(mySocket);
        return -1;
    }
    find.count = 0;
    sprintf(buf, "http://%s/ からデータを転送しています...", findHost);
    pgMenuBar(buf);
    sceDisplayWaitVblankStart();
    framebuffer = sceGuSwapBuffers();
    while(recv(mySocket, &in, 1, 0) > 0)
    {
        if (in != '<') continue;
        if (recv(mySocket, &in, 1, 0) <= 0) break;
        if (in != 'd') continue;
        if (recv(mySocket, &in, 1, 0) <= 0) break;
        if (in != 't') continue;
        if (recv(mySocket, &in, 1, 0) <= 0) break;
        if (in != '>') continue;
        if (recv(mySocket, &in, 1, 0) <= 0) break;
        if (in != '<') continue;
        if (recv(mySocket, &in, 1, 0) <= 0) break;
        if (in != 'a') continue;
        // parse html
        i = 0;
        memset(query, 0, sizeof(query));
        while(recv(mySocket, &in, 1, 0) > 0)
        {
            query[i] = in;
            i++;
            if (strstr(query, "</dt>") || i >= 512)
            {
                break;
            }
        }
        query[i] = '\0';
        // host
        p = strstr(query, "http://");
        if (p == NULL) break;
        p += 7;
        q = strchr(p, '/');
        if (q == NULL) break;
        *q = '\0';
        strcpy(findList[find.count].host, p);
        // dir
        q++;
        p = strstr(q, "read.cgi/");
        if (p == NULL) break;
        p += 9;
        q = strchr(p, '/');
        if (q == NULL) break;
        *q = '\0';
        strcpy(findList[find.count].dir, p);
        // dat
        q++;
        sscanf(q, "%d", &findList[find.count].dat);
        // subject
        p = strchr(q, '>');
        if (p == NULL) break;
        p++;
        q = strchr(p, '<');
        if (q == NULL) break;
        *q = '\0';
        psp2chEucToSjis(findList[find.count].subject, p);
        // ita title
        q++;
        p = strstr(q, "<a");
        if (p == NULL) break;
        p = strchr(p, '>');
        if (p == NULL) break;
        p++;
        q = strchr(p, '<');
        if (q == NULL) break;
        q -= 2; // 板を削除
        *q = '\0';
        psp2chEucToSjis(findList[find.count].title, p);
        find.count++;
        if (find.count >= FIND_MAX_COUNT)
        {
            break;
        }
    }
    psp2chCloseSocket(mySocket);
    return 0;
}

/**********************
**********************/
void psp2chDrawSearch(int scrollX)
{
    int start;
    int i;
    char buf[32];
    int lineEnd, scrW, scrH;

    if (tateFlag)
    {
        lineEnd = 35;
        scrW = SCR_HEIGHT + scrollX;
        scrH = SCR_WIDTH;
    }
    else
    {
        lineEnd = 20;
        scrW = SCR_WIDTH + scrollX;
        scrH = SCR_HEIGHT;
    }
    start = find.start;
    if (start + lineEnd > find.count)
    {
        start = find.count - lineEnd;
    }
    if (start < 0)
    {
        start = 0;
    }
    pgFillvram(threadColor.bg, 0, 0, BUF_WIDTH, BUF_HEIGHT);
    pgCursorY = 0;
    for (i = start; i < start + lineEnd; i++)
    {
        if (i >= find.count)
        {
            return;
        }
        pgCursorX = 0;
        sprintf(buf, "%4d", i + 1);
        if (i == find.select)
        {
            pgFillvram(threadColor.s_bg, 0, pgCursorY, BUF_WIDTH, LINE_PITCH);
            pgPrintNumber(i + 1, threadColor.s_num, threadColor.s_bg);
        }
        else
        {
            pgPrintNumber(i + 1, threadColor.num, threadColor.bg);
        }
        pgCursorX = THREAD_ID;
        if (i == find.select)
        {
            pgPrint(findList[i].title, threadColor.s_category, threadColor.s_bg, scrW);
            pgCursorX += 8;
            pgPrint(findList[i].subject, threadColor.s_text1, threadColor.s_bg, scrW);
        }
        else
        {
            pgPrint(findList[i].title, threadColor.category, threadColor.bg, scrW);
            pgCursorX += 8;
            pgPrint(findList[i].subject, threadColor.text1, threadColor.bg, scrW);
        }
        pgCursorY += LINE_PITCH;
    }
}
