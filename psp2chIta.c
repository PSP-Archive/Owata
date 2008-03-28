/*
* $Id$
*/

#include <stdio.h>
#include <malloc.h>
#include <time.h>
#include <pspdebug.h>
#include "psp2ch.h"
#include "psp2chNet.h"
#include "psp2chIta.h"
#include "psp2chThread.h"
#include "psp2chFavorite.h"
#include "psp2chMenu.h"
#include "utf8.h"
#include "pg.h"

extern S_2CH s2ch; // psp2ch.c
extern char keyWords[128]; // psp2chThread.c
extern const char *sBtnH[]; // psp2chRes.c
extern const char *sBtnV[]; // psp2chRes.c

static const char* boardFile = "2channel.brd";
static const char* boardFile2 = "my.brd";

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

void psp2chItaSetMenuString(void)
{
    int index1, index2, index3, index4, index5;
    int i, tmp;

    getIndex(s2ch.itaH.ok, index1);
    getIndex(s2ch.itaH.esc, index2);
    getIndex(s2ch.itaH.move, index3);
    getIndex(s2ch.itaH.reload, index4);
    getIndex(s2ch.itaH.shift, index5);
    sprintf(s2ch.menuCateH.main, "　%s : 決定　　%s : 終了　　%s : お気に入り　　%s : 更新　　%s : メニュー切替",
            sBtnH[index1], sBtnH[index2], sBtnH[index3], sBtnH[index4], sBtnH[index5]);
    sprintf(s2ch.menuItaH.main, "　%s : 決定　　%s : 戻る　　%s : お気に入り　　%s : 更新　　%s : メニュー切替",
            sBtnH[index1], sBtnH[index2], sBtnH[index3], sBtnH[index4], sBtnH[index5]);

    getIndex(s2ch.listH.top, index1);
    getIndex(s2ch.listH.end, index2);
    getIndex(s2ch.itaH.search2ch, index3);
    getIndex(s2ch.itaH.addFav, index4);
    sprintf(s2ch.menuCateH.sub, "　%s : 先頭　　　%s : 最後　　　%s : 全板検索",
            sBtnH[index1], sBtnH[index2], sBtnH[index3]);
    sprintf(s2ch.menuItaH.sub, "　%s : 先頭　　　%s : 最後　　　%s : 全板検索　　　%s : お気に入りに追加",
            sBtnH[index1], sBtnH[index2], sBtnH[index3], sBtnH[index4]);

    getIndex(s2ch.itaV.ok, index1);
    getIndex(s2ch.itaV.esc, index2);
    getIndex(s2ch.itaV.move, index3);
    getIndex(s2ch.itaV.reload, index4);
    getIndex(s2ch.itaV.shift, index5);
    sprintf(s2ch.menuCateV.main, "　%s : 決定　　　　%s : 終了　　　%s : お気に入り\n　%s : 更新　　　%s : メニュー切替",
            sBtnV[index1], sBtnV[index2], sBtnV[index3], sBtnV[index4], sBtnV[index5]);
    sprintf(s2ch.menuItaV.main, "　%s : 決定　　　　%s : 戻る　　　%s : お気に入り\n　%s : 更新　　　%s : メニュー切替",
            sBtnV[index1], sBtnV[index2], sBtnV[index3], sBtnV[index4], sBtnV[index5]);

    getIndex(s2ch.listV.top, index1);
    getIndex(s2ch.listV.end, index2);
    getIndex(s2ch.itaV.search2ch, index3);
    getIndex(s2ch.itaV.addFav, index4);
    sprintf(s2ch.menuCateV.sub, "　%s : 先頭　　　%s : 最後　　　%s : 全板検索",
            sBtnV[index1], sBtnV[index2], sBtnV[index3]);
    sprintf(s2ch.menuItaV.sub, "　%s : 先頭　　　%s : 最後　　　%s : 全板検索\n　%s : お気に入りに追加",
            sBtnV[index1], sBtnV[index2], sBtnV[index3], sBtnV[index4]);
}

/**********************
 カテゴリーと板一覧表示
**********************/
int psp2chIta(void)
{
    int ret;
    static int focus = 0, end = 0;
    static char* menuStr = "";
    int lineEnd, rMenu;

    if (s2ch.itaList == NULL)
    {
        ret = psp2chItaList();
        if (ret < 0)
        {
            s2ch.sel = 0;
            return ret;
        }
        end = s2ch.ita.count;
    }
    if (s2ch.tateFlag)
    {
        lineEnd = DRAW_LINE_V;
    }
    else
    {
        lineEnd = DRAW_LINE_H;
    }
    if(sceCtrlPeekBufferPositive(&s2ch.pad, 1))
    {

        if (focus)
        {
            if (s2ch.category.select < (s2ch.category.count - 1))
            {
                s2ch.ita.count = s2ch.categoryList[s2ch.category.select + 1].itaId;
            }
            else
            {
                s2ch.ita.count = end;
            }
            if (s2ch.tateFlag)
            {
                rMenu = psp2chCursorSet(&s2ch.ita, lineEnd, s2ch.itaV.shift);
            }
            else
            {
                rMenu = psp2chCursorSet(&s2ch.ita, lineEnd, s2ch.itaH.shift);
            }
            if (s2ch.ita.start < s2ch.categoryList[s2ch.category.select].itaId)
            {
                s2ch.ita.start = s2ch.categoryList[s2ch.category.select].itaId;
                s2ch.ita.select = s2ch.categoryList[s2ch.category.select].itaId;
            }
            psp2chDrawCategory(s2ch.category.start, s2ch.category.select, s2ch.cateOffColor);
            psp2chDrawIta(s2ch.ita.start, s2ch.ita.select, s2ch.cateOffColor);
            if (rMenu)
            {
                if (s2ch.tateFlag)
                {
                    menuStr = s2ch.menuItaV.sub;
                }
                else
                {
                    menuStr = s2ch.menuItaH.sub;
                }
            }
            else
            {
                if (s2ch.tateFlag)
                {
                    menuStr = s2ch.menuItaV.main;
                }
                else
                {
                    menuStr = s2ch.menuItaH.main;
                }
            }
        }
        else
        {
            if (s2ch.tateFlag)
            {
                rMenu = psp2chCursorSet(&s2ch.category, lineEnd, s2ch.itaV.shift);
            }
            else
            {
                rMenu = psp2chCursorSet(&s2ch.category, lineEnd, s2ch.itaH.shift);
            }
            s2ch.ita.start = s2ch.categoryList[s2ch.category.select].itaId;
            s2ch.ita.select = s2ch.categoryList[s2ch.category.select].itaId;
            psp2chDrawCategory(s2ch.category.start, s2ch.category.select, s2ch.cateOnColor);
            psp2chDrawIta(s2ch.ita.start, s2ch.ita.select, s2ch.cateOnColor);
            if (rMenu)
            {
                if (s2ch.tateFlag)
                {
                    menuStr = s2ch.menuCateV.sub;
                }
                else
                {
                    menuStr = s2ch.menuCateH.sub;
                }
            }
            else
            {
                if (s2ch.tateFlag)
                {
                    menuStr = s2ch.menuCateV.main;
                }
                else
                {
                    menuStr = s2ch.menuCateH.main;
                }
            }
        }
        if (s2ch.pad.Buttons != s2ch.oldPad.Buttons)
        {
            s2ch.oldPad = s2ch.pad;
            if (s2ch.pad.Buttons & PSP_CTRL_SELECT)
            {
                s2ch.tateFlag = (s2ch.tateFlag) ? 0 : 1;
            }
            // STARTボタン
            else if(s2ch.pad.Buttons & PSP_CTRL_START)
            {
                psp2chMenu(0, 0);
            }
            else if (rMenu)
            {
                if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.itaH.addFav) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.itaV.addFav))
                {
                    if (focus)
                    {
                        psp2chAddFavoriteIta(s2ch.categoryList[s2ch.category.select].name, s2ch.itaList[s2ch.ita.select].title);
                    }
                }
                else if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.itaH.search2ch) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.itaV.search2ch))
                {
                    if (psp2chThreadSearch() == 0 && keyWords[0])
                    {
                        if (s2ch.findList)
                        {
                            free(s2ch.findList);
                            s2ch.findList = NULL;
                        }
                        s2ch.sel = 7;
                    }
                }
            }
            else
            {
                // 決定
                if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.itaH.ok) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.itaV.ok))
                {
                    if (focus)
                    {
                        if (psp2chThreadList(s2ch.ita.select) < 0)
                        {
                            s2ch.sel = 0;
                            return -1;
                        }
                        s2ch.thread.start = 0;
                        s2ch.thread.select = 0;
                        pgFillvram(WHITE, 0, 0, SCR_WIDTH, BUF_HEIGHT);
                        s2ch.sel = 3;
                        return 0;
                    }
                    else
                    {
                        focus = 1;
                    }
                }
                // 戻る、終了
                else if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.itaH.esc) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.itaV.esc))
                {
                    if (focus)
                    {
                        focus = 0;
                    }
                    else
                    {
                        if (psp2chOwata())
                        {
                            return 0;
                        }
                    }
                }
                // 更新
                else if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.itaH.reload) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.itaV.reload))
                {
                    psp2chGetMenu();
                    psp2chItaList();
                    s2ch.category.start = 0;
                    s2ch.category.select = 0;
                    s2ch.ita.start = 0;
                    s2ch.ita.select = 0;
                    focus = 0;
                }
                // お気に入りに移動
                else if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.itaH.move) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.itaV.move))
                {
                    s2ch.sel = 1;
                }
            }
        }
        pgCopy(0, 0);
        pgMenuBar(menuStr);
        sceDisplayWaitVblankStart();
        framebuffer = sceGuSwapBuffers();
    }
    return 0;
}

/**********************
初回アクセス時等で2channel.brdがない場合psp2chGetMenu()で2channel.brdを作成
2channel.brdを読み込んで s2ch.categoryListとs2ch.itaList構造体のメモリ確保とデータ作成を行う
s2ch.category.countとs2ch.ita.countに総数
**********************/
int psp2chItaList(void)
{
    SceUID fd;
    SceIoStat st, st2;
    char file[256];
    char file2[256];
    char *buf, *p, *q, *r;
    int i, cateOn;

    sprintf(file, "%s/%s/%s", s2ch.cwDir, s2ch.logDir, boardFile);
    i = sceIoGetstat(file, &st);
    if (i < 0)
    {
        psp2chGetMenu();
        i = sceIoGetstat(file, &st);
        if (i< 0)
        {
            memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
            sprintf(s2ch.mh.message, "File stat error\n%s", file);
            pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
            sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
            return -1;
        }
    }
    sprintf(file2, "%s/%s/%s", s2ch.cwDir, s2ch.logDir, boardFile2);
    i = sceIoGetstat(file2, &st2);
    if (i < 0)
    {
        st2.st_size = 0;
    }
    buf = (char*)malloc(st.st_size + st2.st_size + 1);
    if (buf == NULL)
    {
        memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
        strcpy(s2ch.mh.message, "memorry error\npsp2chItaList() buf");
        pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
        sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
        return -1;
    }
    fd = sceIoOpen(file, PSP_O_RDONLY, 0777);
    if (fd < 0)
    {
        free(buf);
        memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
        sprintf(s2ch.mh.message, "File open error\n%s", file);
        pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
        sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
        return -1;
    }
    sceIoRead(fd, buf, st.st_size);
    sceIoClose(fd);
    if (st2.st_size)
    {
        fd = sceIoOpen(file2, PSP_O_RDONLY, 0777);
        if (fd < 0)
        {
            st2.st_size = 0;
        }
        else
        {
            sceIoRead(fd, buf + st.st_size, st2.st_size);
            sceIoClose(fd);
        }
    }
    buf[st.st_size + st2.st_size] = '\0';
    s2ch.category.count = 0;
    s2ch.ita.count = 0;
    p = buf;
    while (*p)
    {
        if (*p == '\t')
        {
            s2ch.ita.count++;
        }
        else
        {
            s2ch.category.count++;
        }
        while (*p++ != '\n')
        {
            ;
        }
    }
    s2ch.categoryList = (S_2CH_CATEGORY*)realloc(s2ch.categoryList, sizeof(S_2CH_CATEGORY) * s2ch.category.count);
    if (s2ch.categoryList == NULL)
    {
        free(buf);
        memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
        strcpy(s2ch.mh.message, "memorry error\ncategoryList");
        pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
        sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
        return -1;
    }
    s2ch.itaList = (S_2CH_ITA*)realloc(s2ch.itaList, sizeof(S_2CH_ITA) * s2ch.ita.count);
    if (s2ch.itaList == NULL)
    {
        free(buf);
        memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
        strcpy(s2ch.mh.message, "memorry error\nitaList");
        pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
        sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
        return -1;
    }
    s2ch.category.count = 0;
    s2ch.ita.count = 0;
    cateOn = 0;
    p = buf;
    q = buf;
    while(*q)
    {
        if (*q == '\t')
        {
            q++;
            p = q;
            q = strchr(p, '\t');
            *q = '\0';
            strcpy(s2ch.itaList[s2ch.ita.count].host, p);
            q++;
            p = q;
            q = strchr(p, '\t');
            *q = '\0';
            strcpy(s2ch.itaList[s2ch.ita.count].dir, p);
            q++;
            p = q;
            q = strchr(p, '\n');
            *q = '\0';
            r = strchr(p, '\r');
            if (r)
            {
                *r = '\0';
            }
            strcpy(s2ch.itaList[s2ch.ita.count].title, p);
            if (cateOn)
            {
                s2ch.categoryList[s2ch.category.count - 1].itaId = s2ch.ita.count;
                cateOn = 0;
            }
            s2ch.ita.count++;
            q++;
        }
        else
        {
            p = q;
            q = strchr(p, '\t');
            *q = '\0';
            strcpy(s2ch.categoryList[s2ch.category.count].name, p);
            s2ch.category.count++;
            cateOn = 1;
            q++;
            p = q;
            q = strchr(p, '\n');
            q++;
        }
    }
    free(buf);
    return 0;
}

/**********************
2channel.brdを作成
**********************/
int psp2chGetMenu(void)
{
    const char* menuHost = "menu.2ch.net";
    const char* path = "bbsmenu.html";
    const char* menustart = "<BR><BR><B>";
    const char* menuend = "</B>";
    const char* itastart = "<A HREF=";
    const char* hostend = "/";
    const char* dirend = "/>";
    const char* titleend = "</A>";
    const char* ita2chnet = "2ch.net";
    const char* itabbspink = "bbspink.com";
    S_NET net;
    SceUID fd;
    int ret, menuOn;
    char buf[256];
    char menu[32];
    char itahost[32];
    char itadir[32];
    char *p, *q, *line;

    ret = psp2chGet(menuHost, path, "", &net);
    if (ret < 0)
    {
        return ret;
    }
    switch(net.status)
    {
        case 200:
            break;
        default:
            memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
            sprintf(s2ch.mh.message, "HTTP error\nStatus code %d", ret);
            pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
            sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
            return -1;
    }
    sprintf(buf, "%s/%s", s2ch.cwDir, s2ch.logDir);
    if ((fd = sceIoDopen(buf)) < 0)
    {
        if (sceIoMkdir(buf, 0777) < 0)
        {
            free(net.body);
            memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
            sprintf(s2ch.mh.message, "Make dir error\n%s", buf);
            pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
            sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
            return -1;
        }
    }
    else
    {
        sceIoDclose(fd);
    }
    strcat(buf, "/");
    strcat(buf, boardFile);
    fd = sceIoOpen(buf, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
    if (fd < 0)
    {
        free(net.body);
        memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
        sprintf(s2ch.mh.message, "File open error\n%s", buf);
        pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
        sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
        return fd;
    }
    menuOn = 0;
    q = net.body;
    while(*q)
    {
        p = strchr(q, '\n');
        if (p == NULL)
        {
            break;
        }
        *p = '\0';
        strcpy(buf, q);
        q = p + 1;
        if (strstr(buf, menustart) == buf && (p = strstr(buf, menuend)) != NULL)
        {
            *p = '\0';
            sprintf(menu, "%s\t0\n", &buf[11]);
            menuOn = 1;
        }
        else if (menuOn)
        {
            if (strstr(buf, itastart) != buf)
            {
                menuOn = 0;
                continue;
            }
            if (strstr(buf, ita2chnet) == NULL && strstr(buf, itabbspink) == NULL)
            {
                continue;
            }
            line = buf + 15;
            if ((p = strstr(line, hostend)) == NULL)
            {
                continue;
            }
            *p = '\0';
            strcpy(itahost, line);
            line = p + 1;
            if ((p = strstr(line, dirend)) == NULL)
            {
                continue;
            }
            *p = '\0';
            strcpy(itadir, line);
            line = p + 2;
            if ((p = strstr(line, titleend)) == NULL)
            {
                continue;
            }
            *p = '\0';
            if (menuOn == 1)
            {
                sceIoWrite(fd, menu, strlen(menu));
                menuOn = 2;
            }
            sprintf(buf, "\t%s\t%s\t%s\n", itahost, itadir, line);
            sceIoWrite(fd, buf, strlen(buf));
        }
    }
    sceIoClose(fd);
    free(net.body);
    return 0;
}

/**********************
カテゴリー一覧を表示
start: 表示開始位置
select: 選択位置
**********************/
#define CATEGORY_W (100)
#define ITA_W (110)
void psp2chDrawCategory(int start, int select, S_2CH_ITA_COLOR c)
{
    int i;
    int lineEnd, scrW, scrH;

    if (s2ch.tateFlag)
    {
        lineEnd = DRAW_LINE_V;
        scrW = SCR_HEIGHT;
        scrH = SCR_WIDTH;
    }
    else
    {
        lineEnd = DRAW_LINE_H;
        scrW = SCR_WIDTH;
        scrH = SCR_HEIGHT;
    }
    if (start + lineEnd > s2ch.category.count)
    {
        start = s2ch.category.count - lineEnd;
    }
    pgFillvram(c.cate.bg, 0, 0, CATEGORY_W, scrH);
    s2ch.pgCursorY = 0;
    for (i = start; i < start + lineEnd; i++)
    {
        s2ch.pgCursorX = 10;
        if (i == select)
        {
            pgPrint(s2ch.categoryList[i].name, c.cate.s_text, c.cate.s_bg, scrW);
        }
        else
        {
            pgPrint(s2ch.categoryList[i].name, c.cate.text, c.cate.bg, scrW);
        }
        s2ch.pgCursorY += LINE_PITCH;
    }
}

/**********************
板一覧を表示
**********************/
void psp2chDrawIta(int start, int select, S_2CH_ITA_COLOR c)
{
    int i, end;
    int lineEnd, scrW, scrH;

    if (s2ch.tateFlag)
    {
        lineEnd = DRAW_LINE_V;
        scrW = SCR_HEIGHT;
        scrH = SCR_WIDTH;
    }
    else
    {
        lineEnd = DRAW_LINE_H;
        scrW = SCR_WIDTH;
        scrH = SCR_HEIGHT;
    }
    if (s2ch.category.select < (s2ch.category.count - 1))
    {
        end = s2ch.categoryList[s2ch.category.select + 1].itaId;
    }
    else
    {
        end = s2ch.ita.count;
    }
    if (start + lineEnd < end)
    {
        end = start + lineEnd;
    }
    pgFillvram(c.ita.bg, CATEGORY_W, 0, ITA_W, scrH);
    pgFillvram(c.base, CATEGORY_W + ITA_W, 0, scrW - CATEGORY_W - ITA_W, scrH);
    s2ch.pgCursorY = 0;
    for (i = start; i < end; i++)
    {
        s2ch.pgCursorX = CATEGORY_W + 10;
        if (i == select)
        {
            pgPrint(s2ch.itaList[i].title, c.ita.s_text, c.ita.s_bg, scrW);
        }
        else
        {
            pgPrint(s2ch.itaList[i].title, c.ita.text, c.ita.bg, scrW);
        }
        s2ch.pgCursorY += LINE_PITCH;
    }
}
