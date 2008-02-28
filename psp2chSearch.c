/*
* $Id$
*/

#include <stdio.h>
#include <malloc.h>
#include <time.h>
#include "pg.h"
#include "psp2ch.h"
#include "psp2chSearch.h"
#include "psp2chIta.h"
#include "psp2chThread.h"
#include "psp2chFavorite.h"
#include "psp2chRes.h"
#include "psp2chForm.h"

extern S_2CH s2ch; // psp2ch.c
extern char keyWords[128]; // psp2chThread.c
extern int preLine; // psp2chRes.c
extern const char *sBtnH[];
extern const char *sBtnV[];

/*********************
メニュー文字列の作成
**********************/
#define getIndex(X, Y) \
    tmp = (X);\
    (Y) = 0;\
    for (i = 0; i < 16; i++)\
    {\
        if (tmp & 1)\
        {\
            break;\
        }\
        (Y)++;\
        tmp >>= 1;\
    }

void psp2chSearchSetMenuString(void)
{
    int index1, index2, index3, index4, index5;
    int i, tmp;

    getIndex(s2ch.findH.ok, index1);
    getIndex(s2ch.findH.ita, index2);
    getIndex(s2ch.findH.fav, index3);
    getIndex(s2ch.findH.esc, index4);
    getIndex(s2ch.findH.shift, index5);
    sprintf(s2ch.menuSearchH.main, "　%s : 決定　　　　　%s : 板一覧　　　　%s : お気に入り　　　　%s : 戻る　　　%s : メニュー切替",
            sBtnH[index1], sBtnH[index2], sBtnH[index3], sBtnH[index4], sBtnH[index5]);

    getIndex(s2ch.listH.top, index1);
    getIndex(s2ch.listH.end, index2);
    getIndex(s2ch.findH.search2ch, index3);
    sprintf(s2ch.menuSearchH.sub, "　%s : 先頭　　　%s : 最後　　　　%s : 全板検索",
            sBtnH[index1], sBtnH[index2], sBtnH[index3]);

    getIndex(s2ch.findV.ok, index1);
    getIndex(s2ch.findV.ita, index2);
    getIndex(s2ch.findV.fav, index3);
    getIndex(s2ch.findV.esc, index4);
    getIndex(s2ch.findV.shift, index5);
    sprintf(s2ch.menuSearchV.main, "　%s : 決定　　　　　%s : 板一覧　　　　%s : お気に入り　　　　%s : 戻る　　　%s : メニュー切替",
            sBtnV[index1], sBtnV[index2], sBtnV[index3], sBtnV[index4], sBtnV[index5]);

    getIndex(s2ch.listV.top, index1);
    getIndex(s2ch.listV.end, index2);
    getIndex(s2ch.findV.search2ch, index3);
    sprintf(s2ch.menuSearchV.sub, "　%s : 先頭　　　%s : 最後　　　　%s : 全板検索",
            sBtnV[index1], sBtnV[index2], sBtnV[index3]);
}

int psp2chSearch(int retSel)
{
    static int scrollX = 0;
    static char* menuStr = "";
    static int ret = 0;
    int lineEnd, rMenu;

    if (s2ch.findList == NULL)
    {
        if (psp2chSearchList() < 0)
        {
            s2ch.sel = retSel;
            return 0;
        }
        ret = retSel;
        s2ch.find.start = 0;
        s2ch.find.select = 0;
    }
    if (s2ch.tateFlag)
    {
        lineEnd = 35;
    }
    else
    {
        lineEnd = 20;
    }
    if(sceCtrlPeekBufferPositive(&s2ch.pad, 1))
    {
        if (s2ch.tateFlag)
        {
            rMenu = psp2chCursorSet(&s2ch.find, lineEnd, s2ch.findV.shift);
        }
        else
        {
            rMenu = psp2chCursorSet(&s2ch.find, lineEnd, s2ch.findH.shift);
        }
        if (rMenu)
        {
            if (s2ch.tateFlag)
            {
                menuStr = s2ch.menuSearchV.sub;
            }
            else
            {
                menuStr = s2ch.menuSearchH.sub;
            }
        }
        else
        {
            if (s2ch.tateFlag)
            {
                menuStr = s2ch.menuSearchV.main;
            }
            else
            {
                menuStr = s2ch.menuSearchH.main;
            }
        }
        if (s2ch.pad.Buttons != s2ch.oldPad.Buttons)
        {
            s2ch.oldPad = s2ch.pad;
            if (s2ch.pad.Buttons & PSP_CTRL_SELECT)
            {
                s2ch.tateFlag = (s2ch.tateFlag) ? 0 : 1;
            }
            if (rMenu)
            {
                if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.findH.search2ch) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.findV.search2ch))
                {
                    if (psp2chThreadSearch() == 0 && keyWords[0])
                    {
                        if (s2ch.findList)
                        {
                            free(s2ch.findList);
                            s2ch.findList = NULL;
                        }
                        return 0;
                    }
                }
            }
            else
            {
                if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.findH.ok) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.findV.ok))
                {
                    free(s2ch.resList);
                    s2ch.resList = NULL;
                    preLine = -2;
                    pgFillvram(WHITE, 0, 0, SCR_WIDTH, BUF_HEIGHT);
                    s2ch.sel = 8;
                    return 0;
                }
                if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.findH.esc) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.findV.esc))
                {
                    s2ch.sel = ret;
                }
                if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.findH.fav) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.findV.fav))
                {
                    s2ch.sel = 1;
                }
                if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.findH.ita) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.findV.ita))
                {
                    s2ch.sel = 2;
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
            memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
            sprintf(s2ch.mh.message, "HTTP error\nhost %s\nStatus code %d", findHost, ret);
            pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
            sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
            psp2chCloseSocket(mySocket);
            return -1;
    }
    ret = psp2chGetHttpHeaders(mySocket, &resHeader);
    s2ch.findList = (S_2CH_FAVORITE*)realloc(s2ch.findList, sizeof(S_2CH_FAVORITE) * FIND_MAX_COUNT);
    if (s2ch.findList == NULL)
    {
        memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
        strcpy(s2ch.mh.message, "memorry error");
        pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
        sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
        psp2chCloseSocket(mySocket);
        return -1;
    }
    s2ch.find.count = 0;
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
        strcpy(s2ch.findList[s2ch.find.count].host, p);
        // dir
        q++;
        p = strstr(q, "read.cgi/");
        if (p == NULL) break;
        p += 9;
        q = strchr(p, '/');
        if (q == NULL) break;
        *q = '\0';
        strcpy(s2ch.findList[s2ch.find.count].dir, p);
        // dat
        q++;
        sscanf(q, "%d", &s2ch.findList[s2ch.find.count].dat);
        // subject
        p = strchr(q, '>');
        if (p == NULL) break;
        p++;
        q = strchr(p, '<');
        if (q == NULL) break;
        *q = '\0';
        psp2chEucToSjis(s2ch.findList[s2ch.find.count].subject, p);
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
        psp2chEucToSjis(s2ch.findList[s2ch.find.count].title, p);
        s2ch.find.count++;
        if (s2ch.find.count >= FIND_MAX_COUNT)
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

    if (s2ch.tateFlag)
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
    start = s2ch.find.start;
    if (start + lineEnd > s2ch.find.count)
    {
        start = s2ch.find.count - lineEnd;
    }
    if (start < 0)
    {
        start = 0;
    }
    pgFillvram(s2ch.threadColor.bg, 0, 0, BUF_WIDTH, BUF_HEIGHT);
    s2ch.pgCursorY = 0;
    for (i = start; i < start + lineEnd; i++)
    {
        if (i >= s2ch.find.count)
        {
            return;
        }
        s2ch.pgCursorX = 0;
        sprintf(buf, "%4d", i + 1);
        if (i == s2ch.find.select)
        {
            pgFillvram(s2ch.threadColor.s_bg, 0, s2ch.pgCursorY, BUF_WIDTH, LINE_PITCH);
            pgPrintNumber(i + 1, s2ch.threadColor.s_num, s2ch.threadColor.s_bg);
        }
        else
        {
            pgPrintNumber(i + 1, s2ch.threadColor.num, s2ch.threadColor.bg);
        }
        s2ch.pgCursorX = THREAD_ID;
        if (i == s2ch.find.select)
        {
            pgPrint(s2ch.findList[i].title, s2ch.threadColor.s_category, s2ch.threadColor.s_bg, scrW);
            s2ch.pgCursorX += 8;
            pgPrint(s2ch.findList[i].subject, s2ch.threadColor.s_text1, s2ch.threadColor.s_bg, scrW);
        }
        else
        {
            pgPrint(s2ch.findList[i].title, s2ch.threadColor.category, s2ch.threadColor.bg, scrW);
            s2ch.pgCursorX += 8;
            pgPrint(s2ch.findList[i].subject, s2ch.threadColor.text1, s2ch.threadColor.bg, scrW);
        }
        s2ch.pgCursorY += LINE_PITCH;
    }
}
