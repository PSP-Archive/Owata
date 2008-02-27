/*
* $Id$
*/

#include <stdio.h>
#include <malloc.h>
#include <time.h>
#include <pspdebug.h>
#include "psp2ch.h"
#include "psp2chIta.h"
#include "psp2chThread.h"
#include "psp2chFavorite.h"
#include "psp2chMenu.h"
#include "utf8.h"
#include "pg.h"

extern S_2CH s2ch; // psp2ch.c
extern char keyWords[128]; // psp2chThread.c

static const char* boardFile = "2channel.brd";

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
        lineEnd = 35;
    }
    else
    {
        lineEnd = 20;
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
            rMenu = psp2chCursorSet(&s2ch.ita, lineEnd);
            if (s2ch.ita.start < s2ch.categoryList[s2ch.category.select].itaId)
            {
                s2ch.ita.start = s2ch.categoryList[s2ch.category.select].itaId;
                s2ch.ita.select = s2ch.categoryList[s2ch.category.select].itaId;
            }
            psp2chDrawCategory(s2ch.category.start, s2ch.category.select, s2ch.cateOffColor);
            psp2chDrawIta(s2ch.ita.start, s2ch.ita.select, s2ch.cateOffColor);
            if (rMenu)
            {
                menuStr = "　↑ : 先頭　　　　↓ : 最後　　　　□ : 全板検索　　　　　△ : お気に入りに追加";
            }
            else
            {
                if (s2ch.tateFlag)
                {
                    menuStr = "　L : 決定　　　　　× : 戻る　　　　□ : お気に入り　　　　△ : 更新　　　　　R : メニュー切替";
                }
                else
                {
                    menuStr = "　○ : 決定　　　　× : 戻る　　　　□ : お気に入り　　　△ : 更新　　　　　R : メニュー切替";
                }
            }
        }
        else
        {
            rMenu = psp2chCursorSet(&s2ch.category, lineEnd);
            s2ch.ita.start = s2ch.categoryList[s2ch.category.select].itaId;
            s2ch.ita.select = s2ch.categoryList[s2ch.category.select].itaId;
            psp2chDrawCategory(s2ch.category.start, s2ch.category.select, s2ch.cateOnColor);
            psp2chDrawIta(s2ch.ita.start, s2ch.ita.select, s2ch.cateOnColor);
            if (rMenu)
            {
                menuStr = "　↑ : 先頭　　　　↓ : 最後　　　　□ : 全板検索";
            }
            else
            {
                if (s2ch.tateFlag)
                {
                    menuStr = "　L : 決定　　　　　× : 終了　　　　□ : お気に入り　　　　△ : 更新　　　　　R : メニュー切替";
                }
                else
                {
                    menuStr = "　○ : 決定　　　　× : 終了　　　　□ : お気に入り　　　△ : 更新　　　　　R : メニュー切替";
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
            else if((!s2ch.tateFlag && s2ch.pad.Buttons & PSP_CTRL_CIRCLE) || (s2ch.tateFlag && s2ch.pad.Buttons & PSP_CTRL_LTRIGGER))
            {
                if (rMenu)
                {
                }
                else
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
            }
            else if(s2ch.pad.Buttons & PSP_CTRL_CROSS)
            {
                if (rMenu)
                {
                }
                else
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
            }
            else if(s2ch.pad.Buttons & PSP_CTRL_TRIANGLE)
            {
                if (rMenu)
                {
                    psp2chAddFavoriteIta(s2ch.categoryList[s2ch.category.select].name, s2ch.itaList[s2ch.ita.select].title);
                }
                else
                {
                    psp2chGetMenu();
                    psp2chItaList();
                    s2ch.category.start = 0;
                    s2ch.category.select = 0;
                    s2ch.ita.start = 0;
                    s2ch.ita.select = 0;
                    focus = 0;
                }
            }
            else if(s2ch.pad.Buttons & PSP_CTRL_SQUARE)
            {
                if (rMenu)
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
                else
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
    SceIoStat st;
    char file[256];
    char *buf, *p, *q;
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
    buf = (char*)malloc(st.st_size + 1);
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
    buf[st.st_size] = '\0';
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
    HTTP_HEADERS resHeader;
    SceUID fd;
    int contentLength, ret, mySocket, len, menuOn;
    char buf[256];
    char menu[32];
    char itahost[32];
    char itadir[32];
    char *p, *line;

    mySocket = psp2chRequest(menuHost, path, "");
    if (mySocket < 0)
    {
        return mySocket;
    }
    ret = psp2chGetStatusLine(mySocket);
    switch(ret)
    {
        case 200:
            break;
        default:
            memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
            sprintf(s2ch.mh.message, "HTTP error\nStatus code %d", ret);
            pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
            sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
            psp2chCloseSocket(mySocket);
            return -1;
    }
    contentLength = psp2chGetHttpHeaders(mySocket, &resHeader);
    if (contentLength <= 0)
    {
        psp2chCloseSocket(mySocket);
        return -1;
    }
    sprintf(buf, "%s/%s", s2ch.cwDir, s2ch.logDir);
    if ((fd = sceIoDopen(buf)) < 0)
    {
        if (sceIoMkdir(buf, 0777) < 0)
        {
            memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
            sprintf(s2ch.mh.message, "Make dir error\n%s", buf);
            pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
            sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
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
        psp2chCloseSocket(mySocket);
        memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
        sprintf(s2ch.mh.message, "File open error\n%s", buf);
        pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
        sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
        return fd;
    }
    menuOn = 0;
    sprintf(buf, "http://%s/%s からデータを転送しています...", menuHost, path);
    pgMenuBar(buf);
    sceDisplayWaitVblankStart();
    framebuffer = sceGuSwapBuffers();
    while((recv(mySocket, buf, 1, 0)) > 0)
    {
        len = 0;
        while (buf[len] != '\n')
        {
            len++;
            if (recv(mySocket, &buf[len], 1, 0) == 0)
            {
                break;
            }
        }
        buf[len] = '\0';
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
    psp2chCloseSocket(mySocket);
    return 0;
}

/**********************
カテゴリー一覧を表示
start: 表示開始位置
select: 選択位置
**********************/
void psp2chDrawCategory(int start, int select, S_2CH_ITA_COLOR c)
{
    int i;
    int lineEnd, scrW, scrH;

    if (s2ch.tateFlag)
    {
        lineEnd = 35;
        scrW = SCR_HEIGHT;
        scrH = SCR_WIDTH;
    }
    else
    {
        lineEnd = 20;
        scrW = SCR_WIDTH;
        scrH = SCR_HEIGHT;
    }
    if (start + lineEnd > s2ch.category.count)
    {
        start = s2ch.category.count - lineEnd;
    }
    pgFillvram(c.cate.bg, 0, 0, 90, scrH);
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
        lineEnd = 35;
        scrW = SCR_HEIGHT;
        scrH = SCR_WIDTH;
    }
    else
    {
        lineEnd = 20;
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
    pgFillvram(c.ita.bg, 90, 0, 100, scrH);
    pgFillvram(c.base, 190, 0, scrW-190, scrH);
    s2ch.pgCursorY = 0;
    for (i = start; i < end; i++)
    {
        s2ch.pgCursorX = 100;
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
