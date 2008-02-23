
#include "pspdialogs.h"
#include <stdio.h>
#include <malloc.h>
#include <time.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include "psp2chIta.h"
#include "psp2chThread.h"
#include "psp2chFavorite.h"
#include "utf8.h"
#include "pg.h"

extern int running; //main.c
extern char cwDir[256]; //main.c
extern char* logDir; // psp2ch.c
extern int sel; // psp2ch.c
extern int tateFlag; // psp2ch.c
extern SceCtrlData pad; // psp2ch.c
extern SceCtrlData oldPad; // psp2ch.c
extern MESSAGE_HELPER mh; // psp2ch.c
extern S_2CH_THREAD* threadList; // psp2chThread.c
extern S_2CH_SCREEN thread; // psp2chThread.c
extern unsigned long pgCursorX, pgCursorY; // pg.c
extern void* framebuffer; // pg.c
extern S_2CH_ITA_COLOR cateOnColor; // psp2ch.c
extern S_2CH_ITA_COLOR cateOffColor; // psp2ch.c
extern S_2CH_FAVORITE* findList; // psp2chSearch.c
extern char keyWords[128]; // psp2chThread.c

static const char* boardFile = "2channel.brd";
S_2CH_ITA* itaList = NULL;
S_2CH_CATEGORY* categoryList = NULL;
S_2CH_SCREEN ita;
S_2CH_SCREEN category;

/**********************
 カテゴリーと板一覧表示
**********************/
int psp2chIta(void)
{
    int ret;
    static int focus = 0, end = 0;
    static char* menuStr = "";
    int rMenu;

    if (itaList == NULL)
    {
        ret = psp2chItaList();
        if (ret < 0)
        {
            sel = 0;
            return ret;
        }
        end = ita.count;
    }
    if(sceCtrlPeekBufferPositive(&pad, 1))
    {

        if (focus)
        {
            if (category.select < (category.count - 1))
            {
                ita.count = categoryList[category.select + 1].itaId;
            }
            else
            {
                ita.count = end;
            }
            rMenu = psp2chCursorSet(&ita);
            if (ita.start < categoryList[category.select].itaId)
            {
                ita.start = categoryList[category.select].itaId;
                ita.select = categoryList[category.select].itaId;
            }
            psp2chDrawCategory(category.start, category.select, cateOffColor);
            psp2chDrawIta(ita.start, ita.select, cateOffColor);
            if (rMenu)
            {
                menuStr = "　↑ : 先頭　　　↓ : 最後　　　□ : 全板検索";
            }
            else
            {
                menuStr = "　○ : 決定　　　　× : 戻る　　　　□ : お気に入り　　　△ : 更新　　　　　R : メニュー切替";
            }
        }
        else
        {
            rMenu = psp2chCursorSet(&category);
            ita.start = categoryList[category.select].itaId;
            ita.select = categoryList[category.select].itaId;
            psp2chDrawCategory(category.start, category.select, cateOnColor);
            psp2chDrawIta(ita.start, ita.select, cateOnColor);
            if (rMenu)
            {
                menuStr = "　↑ : 先頭　　　↓ : 最後　　　□ : 全板検索";
            }
            else
            {
                menuStr = "　○ : 決定　　　　× : 終了　　　　□ : お気に入り　　　△ : 更新　　　　　R : メニュー切替";
            }
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
                    if (focus)
                    {
                        if (threadList)
                        {
                            free(threadList);
                            threadList = NULL;
                        }
                        thread.start = 0;
                        thread.select = 0;
                        pgFillvram(WHITE, 0, 0, SCR_WIDTH, BUF_HEIGHT);
                        sel = 3;
                        return 0;
                    }
                    else
                    {
                        focus = 1;
                    }
                }
            }
            else if(pad.Buttons & PSP_CTRL_CROSS)
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
            else if(pad.Buttons & PSP_CTRL_TRIANGLE)
            {
                if (rMenu)
                {
                }
                else
                {
                    psp2chGetMenu();
                    psp2chItaList();
                    category.start = 0;
                    category.select = 0;
                    ita.start = 0;
                    ita.select = 0;
                    focus = 0;
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
                        sel = 7;
                    }
                }
                else
                {
                    sel = 1;
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
2channel.brdを読み込んで categoryListとitaList構造体のメモリ確保とデータ作成を行う
category.countとita.countに総数
**********************/
int psp2chItaList(void)
{
    SceUID fd;
    SceIoStat st;
    char file[256];
    char *buf, *p, *q;
    int i, cateOn;

    sprintf(file, "%s/%s/%s", cwDir, logDir, boardFile);
    i = sceIoGetstat(file, &st);
    if (i < 0)
    {
        psp2chGetMenu();
        i = sceIoGetstat(file, &st);
        if (i< 0)
        {
            memset(&mh,0,sizeof(MESSAGE_HELPER));
            sprintf(mh.message, "File stat error\n%s", file);
            pspShowMessageDialog(&mh, DIALOG_LANGUAGE_AUTO);
            sceCtrlPeekBufferPositive(&oldPad, 1);
            return -1;
        }
    }
    buf = (char*)malloc(st.st_size + 1);
    if (buf == NULL)
    {
        memset(&mh,0,sizeof(MESSAGE_HELPER));
        strcpy(mh.message, "memorry error\npsp2chItaList() buf");
        pspShowMessageDialog(&mh, DIALOG_LANGUAGE_AUTO);
        sceCtrlPeekBufferPositive(&oldPad, 1);
        return -1;
    }
    fd = sceIoOpen(file, PSP_O_RDONLY, 0777);
    if (fd < 0)
    {
        free(buf);
        memset(&mh,0,sizeof(MESSAGE_HELPER));
        sprintf(mh.message, "File open error\n%s", file);
        pspShowMessageDialog(&mh, DIALOG_LANGUAGE_AUTO);
        sceCtrlPeekBufferPositive(&oldPad, 1);
        return -1;
    }
    sceIoRead(fd, buf, st.st_size);
    sceIoClose(fd);
    buf[st.st_size] = '\0';
    category.count = 0;
    ita.count = 0;
    p = buf;
    while (*p)
    {
        if (*p == '\t')
        {
            ita.count++;
        }
        else
        {
            category.count++;
        }
        while (*p++ != '\n')
        {
            ;
        }
    }
    categoryList = (S_2CH_CATEGORY*)realloc(categoryList, sizeof(S_2CH_CATEGORY) * category.count);
    if (categoryList == NULL)
    {
        free(buf);
        memset(&mh,0,sizeof(MESSAGE_HELPER));
        strcpy(mh.message, "memorry error\ncategoryList");
        pspShowMessageDialog(&mh, DIALOG_LANGUAGE_AUTO);
        sceCtrlPeekBufferPositive(&oldPad, 1);
        return -1;
    }
    itaList = (S_2CH_ITA*)realloc(itaList, sizeof(S_2CH_ITA) * ita.count);
    if (itaList == NULL)
    {
        free(buf);
        memset(&mh,0,sizeof(MESSAGE_HELPER));
        strcpy(mh.message, "memorry error\nitaList");
        pspShowMessageDialog(&mh, DIALOG_LANGUAGE_AUTO);
        sceCtrlPeekBufferPositive(&oldPad, 1);
        return -1;
    }
    category.count = 0;
    ita.count = 0;
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
            strcpy(itaList[ita.count].host, p);
            q++;
            p = q;
            q = strchr(p, '\t');
            *q = '\0';
            strcpy(itaList[ita.count].dir, p);
            q++;
            p = q;
            q = strchr(p, '\n');
            *q = '\0';
            strcpy(itaList[ita.count].title, p);
            if (cateOn)
            {
                categoryList[category.count - 1].itaId = ita.count;
                cateOn = 0;
            }
            ita.count++;
            q++;
        }
        else
        {
            p = q;
            q = strchr(p, '\t');
            *q = '\0';
            strcpy(categoryList[category.count].name, p);
            category.count++;
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
            memset(&mh,0,sizeof(MESSAGE_HELPER));
            sprintf(mh.message, "HTTP error\nStatus code %d", ret);
            pspShowMessageDialog(&mh, DIALOG_LANGUAGE_AUTO);
            sceCtrlPeekBufferPositive(&oldPad, 1);
            psp2chCloseSocket(mySocket);
            return -1;
    }
    contentLength = psp2chGetHttpHeaders(mySocket, &resHeader);
    if (contentLength <= 0)
    {
        psp2chCloseSocket(mySocket);
        return -1;
    }
    sprintf(buf, "%s/%s", cwDir, logDir);
    if ((fd = sceIoDopen(buf)) < 0)
    {
        if (sceIoMkdir(buf, 0777) < 0)
        {
            memset(&mh,0,sizeof(MESSAGE_HELPER));
            sprintf(mh.message, "Make dir error\n%s", buf);
            pspShowMessageDialog(&mh, DIALOG_LANGUAGE_AUTO);
            sceCtrlPeekBufferPositive(&oldPad, 1);
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
        memset(&mh,0,sizeof(MESSAGE_HELPER));
        sprintf(mh.message, "File open error\n%s", buf);
        pspShowMessageDialog(&mh, DIALOG_LANGUAGE_AUTO);
        sceCtrlPeekBufferPositive(&oldPad, 1);
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

    if (tateFlag)
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
    if (start + lineEnd > category.count)
    {
        start = category.count - lineEnd;
    }
    pgFillvram(c.cate.bg, 0, 0, 90, scrH);
    pgCursorY = 0;
    for (i = start; i < start + lineEnd; i++)
    {
        pgCursorX = 10;
        if (i == select)
        {
            pgPrint(categoryList[i].name, c.cate.s_text, c.cate.s_bg, scrW);
        }
        else
        {
            pgPrint(categoryList[i].name, c.cate.text, c.cate.bg, scrW);
        }
        pgCursorY += LINE_PITCH;
    }
}

/**********************
板一覧を表示
**********************/
void psp2chDrawIta(int start, int select, S_2CH_ITA_COLOR c)
{
    int i, end;
    int lineEnd, scrW, scrH;

    if (tateFlag)
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
    if (category.select < (category.count - 1))
    {
        end = categoryList[category.select + 1].itaId;
    }
    else
    {
        end = ita.count;
    }
    if (start + lineEnd < end)
    {
        end = start + lineEnd;
    }
    pgFillvram(c.ita.bg, 90, 0, 100, scrH);
    pgFillvram(c.base, 190, 0, scrW-190, scrH);
    pgCursorY = 0;
    for (i = start; i < end; i++)
    {
        pgCursorX = 100;
        if (i == select)
        {
            pgPrint(itaList[i].title, c.ita.s_text, c.ita.s_bg, scrW);
        }
        else
        {
            pgPrint(itaList[i].title, c.ita.text, c.ita.bg, scrW);
        }
        pgCursorY += LINE_PITCH;
    }
}
