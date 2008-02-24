/*
* $Id$
*/

#include "pspdialogs.h"
#include <stdio.h>
#include <malloc.h>
#include <time.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include "psp2ch.h"
#include "psp2chIta.h"
#include "psp2chThread.h"
#include "psp2chRes.h"
#include "psp2chFavorite.h"
#include "utf8.h"
#include "pg.h"

extern int running; //main.c
extern char cwDir[256]; //main.c
extern unsigned long pgCursorX, pgCursorY; // pg.c
extern void* framebuffer; // pg.c
extern char* logDir; // psp2ch.c
extern int sel; // psp2ch.c
extern int tateFlag; // psp2ch.c
extern SceCtrlData pad; // psp2ch.c
extern SceCtrlData oldPad; // psp2ch.c
extern MESSAGE_HELPER mh; // psp2ch.c
extern S_2CH_THREAD* threadList; // psp2chThread.c
extern S_2CH_SCREEN thread; // psp2chThread.c
extern S_2CH_RES* resList; // psp2chRes.c
extern int preLine; // psp2chRes.c
extern S_2CH_THREAD_COLOR threadColor; // psp2ch.c
extern S_2CH_FAVORITE* findList; // psp2chSearch.c
extern char keyWords[128]; // psp2chThread.c

S_2CH_FAVORITE* favList = NULL;
S_2CH_SCREEN fav;

/**********************
 Favorite
**********************/
int psp2chFavorite(void)
{
    static int scrollX = 0;
    static char* menuStr = "";
    int rMenu;

    if (favList == NULL)
    {
        if (psp2chLoadFavorite() < 0)
        {
            sel = 2;
            return 0;
        }
    }
    if(sceCtrlPeekBufferPositive(&pad, 1))
    {
        rMenu = psp2chCursorSet(&fav);
        if (rMenu)
        {
            menuStr = "　↑ : 先頭　　　↓ : 最後　　　　□ : 全板検索";
        }
        else
        {
            menuStr = "　○ : 決定　　　　　× : 終了　　　　　△ : 削除　　　　　□ : 板一覧　　　R : メニュー切替";
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
                    sel = 4;
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
                    if (psp2chOwata())
                    {
                        return 0;
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
                    psp2chDelFavorite(favList[fav.select].title, favList[fav.select].dat);
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
                    sel = 2;
                }
            }
        }
        scrollX = psp2chPadSet(scrollX);
        psp2chDrawFavorite(scrollX);
        pgCopy(scrollX, 0);
        pgMenuBar(menuStr);
        sceDisplayWaitVblankStart();
        framebuffer = sceGuSwapBuffers();
    }
    return 0;
}

/**********************
favorite.brdがあれば読み込んで
favListのメモリ再確保とデータ作成
**********************/
int psp2chLoadFavorite(void)
{
    SceUID fd;
    SceIoStat st;
    char path[256];
    char *buf, *p, *r;
    int i;

    sprintf(path, "%s/%s/favorite.brd", cwDir, logDir);
    i = sceIoGetstat(path, &st);
    if (i < 0)
    {
        return -1;
    }
    buf = (char*)malloc(st.st_size + 1);
    if (buf == NULL)
    {
        memset(&mh,0,sizeof(MESSAGE_HELPER));
        strcpy(mh.message, "memorry error");
        pspShowMessageDialog(&mh, DIALOG_LANGUAGE_AUTO);
        return -1;
    }
    fd = sceIoOpen(path, PSP_O_RDONLY, 0777);
    if (fd < 0)
    {
        free(buf);
        return -1;
    }
    sceIoRead(fd, buf, st.st_size);
    sceIoClose(fd);
    buf[st.st_size] = '\0';
    p = buf;
    fav.count = 0;
    while (*p)
    {
        if (*p++ == '\n')
        {
            fav.count++;
        }
    }
    if (fav.count <= 0)
    {
        free(buf);
        return -1;
    }
    favList = (S_2CH_FAVORITE*)realloc(favList, sizeof(S_2CH_FAVORITE) * fav.count);
    if (favList == NULL)
    {
        memset(&mh,0,sizeof(MESSAGE_HELPER));
        strcpy(mh.message, "memorry error");
        pspShowMessageDialog(&mh, DIALOG_LANGUAGE_AUTO);
        return -1;
    }
    r = buf;
    i = 0;
    while (*r)
    {
        p = strchr(r, '\t');
        *p= '\0';
        strcpy(favList[i].host, r);
        r = ++p;
        p = strchr(r, '\t');
        *p= '\0';
        strcpy(favList[i].dir, r);
        r = ++p;
        p = strchr(r, '\t');
        *p= '\0';
        strcpy(favList[i].title, r);
        r = ++p;
        p = strchr(r, '\t');
        *p= '\0';
        sscanf(r, "%d", &favList[i].dat);
        r = ++p;
        p = strchr(r, '\n');
        *p= '\0';
        strcpy(favList[i].subject, r);
        r = ++p;
        i++;
    }
    free(buf);
    return 0;
}

/**********************
表示中のスレッドをfavorite.brdの最後に追加
psp2chLoadFavorite()でリストを作成しなおす
**********************/
int psp2chAddFavorite(char* host, char* dir, char* title, int dat)
{
    SceUID fd;
    char path[256];
    int i;

    if (fav.count == 0)
    {
        psp2chLoadFavorite();
    }
    for (i = 0; i < fav.count; i++)
    {
        if (favList[i].dat == dat && strcmp(favList[i].title, title) == 0)
        {
            memset(&mh,0,sizeof(MESSAGE_HELPER));
            strcpy(mh.message, TEXT_8);
            pspShowMessageDialog(&mh, DIALOG_LANGUAGE_AUTO);
            return -1;
        }
    }
    sprintf(path, "%s/%s/favorite.brd", cwDir, logDir);
    fd = sceIoOpen(path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_APPEND, 0777);
    if (fd < 0)
    {
        return -1;
    }
    sprintf(path, "%s\t%s\t%s\t%d\t%s\n", host, dir, title, dat, resList[0].title);
    sceIoWrite(fd, path, strlen(path));
    sceIoClose(fd);
    return psp2chLoadFavorite();
}

/**********************
favListからtitleとdatの一致する項目以外のリストのみをfavorite.brdに書き出す
psp2chLoadFavorite()でリストを作成しなおす
**********************/
int psp2chDelFavorite(char* title, int dat)
{
    SceUID fd;
    char path[256];
    int i;

    if (favList == NULL || fav.count <= 0)
    {
        return -1;
    }
    memset(&mh,0,sizeof(MESSAGE_HELPER));
    mh.options = PSP_UTILITY_MSGDIALOG_OPTION_TEXT | PSP_UTILITY_MSGDIALOG_OPTION_YESNO_BUTTONS;
    strcpy(mh.message, TEXT_9);
    pspShowMessageDialog(&mh, DIALOG_LANGUAGE_AUTO);
    sceCtrlPeekBufferPositive(&oldPad, 1);
    if (mh.buttonPressed == PSP_UTILITY_MSGDIALOG_RESULT_YES)
    {
        sprintf(path, "%s/%s/favorite.brd", cwDir, logDir);
        fd = sceIoOpen(path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
        if (fd < 0)
        {
            return -1;
        }
        for (i = 0; i < fav.count; i++)
        {
            if (favList[i].dat == dat && strcmp(favList[i].title, title) == 0)
            {
                continue;
            }
            sprintf(path, "%s\t%s\t%s\t%d\t%s\n", favList[i].host, favList[i].dir, favList[i].title, favList[i].dat, favList[i].subject);
            sceIoWrite(fd, path, strlen(path));
        }
        sceIoClose(fd);
        fav.start = 0;
        fav.select = 0;
        return psp2chLoadFavorite();
    }
    return 0;
}

/**********************
**********************/
void psp2chDrawFavorite(int scrollX)
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
    start = fav.start;
    if (start + lineEnd > fav.count)
    {
        start = fav.count - lineEnd;
    }
    if (start < 0)
    {
        start = 0;
    }
    pgFillvram(threadColor.bg, 0, 0, BUF_WIDTH, BUF_HEIGHT);
    pgCursorY = 0;
    for (i = start; i < start + lineEnd; i++)
    {
        if (i >= fav.count)
        {
            return;
        }
        pgCursorX = 0;
        sprintf(buf, "%4d", i + 1);
        if (i == fav.select)
        {
            pgFillvram(threadColor.s_bg, 0, pgCursorY, BUF_WIDTH, LINE_PITCH);
            pgPrintNumber(i + 1, threadColor.s_num, threadColor.s_bg);
        }
        else
        {
            pgPrintNumber(i + 1, threadColor.num, threadColor.bg);
        }
        pgCursorX = THREAD_ID;
        if (i == fav.select)
        {
            pgPrint(favList[i].title, threadColor.s_category, threadColor.s_bg, scrW);
            pgCursorX += 8;
            pgPrint(favList[i].subject, threadColor.s_text1, threadColor.s_bg, scrW);
        }
        else
        {
            pgPrint(favList[i].title, threadColor.category, threadColor.bg, scrW);
            pgCursorX += 8;
            pgPrint(favList[i].subject, threadColor.text1, threadColor.bg, scrW);
        }
        pgCursorY += LINE_PITCH;
    }
}
