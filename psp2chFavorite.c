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
#include "psp2chRes.h"
#include "psp2chFavorite.h"
#include "psp2chMenu.h"
#include "utf8.h"
#include "pg.h"
#include "intraFont.h"

extern S_2CH s2ch; // psp2ch.c
extern unsigned int list[512*512]; // pg.c
extern intraFont* jpn0; // pg.c
extern int preLine; // psp2chRes.c
extern char keyWords[128]; // psp2chThread.c
extern const char *sBtnH[]; // psp2chRes.c
extern const char *sBtnV[]; // psp2chRes.c

int* favSort = NULL;

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

void psp2chFavSetMenuString(void)
{
    int index1, index2, index3, index4, index5;
    int i, tmp;

    getIndex(s2ch.favH.ok, index1);
    getIndex(s2ch.favH.move, index2);
    getIndex(s2ch.favH.change, index3);
    getIndex(s2ch.favH.del, index4);
    getIndex(s2ch.favH.shift, index5);
    sprintf(s2ch.menuFavH.main, "　%s : 決定　　%s : 板一覧　　%s : お気に板　　%s : 削除　　%s : メニュー切替",
            sBtnH[index1], sBtnH[index2], sBtnH[index3], sBtnH[index4], sBtnH[index5]);
    sprintf(s2ch.menuFavItaH.main, "　%s : 決定　　%s : 板一覧　　%s : お気にスレ　　%s : 削除　　%s : メニュー切替",
            sBtnH[index1], sBtnH[index2], sBtnH[index3], sBtnH[index4], sBtnH[index5]);

    getIndex(s2ch.listH.top, index1);
    getIndex(s2ch.listH.end, index2);
    getIndex(s2ch.favH.sort, index3);
    getIndex(s2ch.favH.search2ch, index4);
    getIndex(s2ch.favH.update, index5);
    sprintf(s2ch.menuFavH.sub, "　%s : 先頭　　%s : 最後　　%s : ソ\ート　　%s : 全板検索　　%s : 一括取得",
            sBtnH[index1], sBtnH[index2], sBtnH[index3], sBtnH[index4], sBtnH[index5]);
    sprintf(s2ch.menuFavItaH.sub, "　%s : 先頭　　　%s : 最後　　　%s : 全板検索",
            sBtnH[index1], sBtnH[index2], sBtnH[index4]);

    getIndex(s2ch.favV.ok, index1);
    getIndex(s2ch.favV.move, index2);
    getIndex(s2ch.favV.change, index3);
    getIndex(s2ch.favV.del, index4);
    getIndex(s2ch.favV.shift, index5);
    sprintf(s2ch.menuFavV.main, "　%s : 決定　　%s : 板一覧　　%s : お気に板\n　%s : 削除　　%s : メニュー切替",
            sBtnV[index1], sBtnV[index2], sBtnV[index3], sBtnV[index4], sBtnV[index5]);
    sprintf(s2ch.menuFavItaV.main,"　%s : 決定　　%s : 板一覧　　%s : お気にスレ\n　%s : 削除　　%s : メニュー切替",
            sBtnV[index1], sBtnV[index2], sBtnV[index3], sBtnV[index4], sBtnV[index5]);

    getIndex(s2ch.listV.top, index1);
    getIndex(s2ch.listV.end, index2);
    getIndex(s2ch.favV.sort, index3);
    getIndex(s2ch.favV.search2ch, index4);
    getIndex(s2ch.favV.update, index5);
    sprintf(s2ch.menuFavV.sub, "　%s : 先頭　　%s : 最後　　%s : ソ\ート\n　%s : 全板検索　　%s : 一括取得",
            sBtnV[index1], sBtnV[index2], sBtnV[index3], sBtnV[index4], sBtnH[index5]);
    sprintf(s2ch.menuFavItaV.sub, "　%s : 先頭　　　%s : 最後　　　%s : 全板検索",
            sBtnV[index1], sBtnV[index2], sBtnV[index4]);
}

/**********************
 Favorite
**********************/
int psp2chFavorite(void)
{
    static char* menuStr = "";
    static int focus = -1, update = -1;
    int lineEnd, rMenu;
    int i, res, change = 0;

    if (s2ch.favList == NULL)
    {
        psp2chLoadFavorite();
    }
    if (s2ch.favItaList == NULL)
    {
        psp2chLoadFavoriteIta();
    }
    if (s2ch.favList == NULL && s2ch.favItaList == NULL)
    {
        s2ch.sel = 2;
        return -1;
    }
    if (focus < 0)
    {
        if (s2ch.favList && s2ch.cfg.favSelect == 0)
        {
            focus = s2ch.cfg.favSelect;
        }
        else if (s2ch.favItaList && s2ch.cfg.favSelect == 1)
        {
            focus = s2ch.cfg.favSelect;
        }
        else
        {
            focus = 1;
        }
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
        if (s2ch.tateFlag)
        {
            if (focus)
            {
                rMenu = psp2chCursorSet(&s2ch.favIta, lineEnd, s2ch.favV.shift, &change);
            }
            else
            {
                rMenu = psp2chCursorSet(&s2ch.fav, lineEnd, s2ch.favV.shift, &change);
            }
        }
        else
        {
            if (focus)
            {
                rMenu = psp2chCursorSet(&s2ch.favIta, lineEnd, s2ch.favH.shift, &change);
            }
            else
            {
                rMenu = psp2chCursorSet(&s2ch.fav, lineEnd, s2ch.favH.shift, &change);
            }
        }
        if (rMenu)
        {
            if (s2ch.tateFlag)
            {
                if (focus)
                {
                    menuStr = s2ch.menuFavItaV.sub;
                }
                else
                {
                    menuStr = s2ch.menuFavV.sub;
                }
            }
            else
            {
                if (focus)
                {
                    menuStr = s2ch.menuFavItaH.sub;
                }
                else
                {
                    menuStr = s2ch.menuFavH.sub;
                }
            }
        }
        else
        {
            if (s2ch.tateFlag)
            {
                if (focus)
                {
                    menuStr = s2ch.menuFavItaV.main;
                }
                else
                {
                    menuStr = s2ch.menuFavV.main;
                }
            }
            else
            {
                if (focus)
                {
                    menuStr = s2ch.menuFavItaH.main;
                }
                else
                {
                    menuStr = s2ch.menuFavH.main;
                }
            }
        }
        // 一括取得処理(1スレごとに描画)
        if (update >= 0)
        {
            if (update < s2ch.fav.count)
            {
                s2ch.fav.select = update;
				psp2chDrawFavorite();
				pgCopy(s2ch.viewX, 0);
				pgCopyMenuBar();
				sceDisplayWaitVblankStart();
				framebuffer = sceGuSwapBuffers();
				pgCopy(s2ch.viewX, 0);
				pgCopyMenuBar();
				sceDisplayWaitVblankStart();
				framebuffer = sceGuSwapBuffers();
                if (s2ch.fav.select >= s2ch.fav.start + lineEnd)
                {
                    s2ch.fav.start = s2ch.fav.select - lineEnd + 1;
                }
                res = psp2chGetDat(s2ch.favList[favSort[update]].host, s2ch.favList[favSort[update]].dir, s2ch.favList[favSort[update]].title, s2ch.favList[favSort[update]].dat);
                if (res == 0)
                {
                    psp2chResList(s2ch.favList[favSort[update]].host, s2ch.favList[favSort[update]].dir, s2ch.favList[favSort[update]].title, s2ch.favList[favSort[update]].dat);
                    s2ch.favList[favSort[update]].res = psp2chGetResCount(s2ch.favList[favSort[update]].title, s2ch.favList[favSort[update]].dat);
                    s2ch.favList[favSort[update]].update = 1;
                    update++;
                }
                else if (res == 1)
                {
                    s2ch.favList[favSort[update]].update = 0;
                    update++;
                }
                else
                {
                    update = -1;
                }
            }
            else
            {
                update = -1;
                sceNetApctlDisconnect();
				pgPrintMenuBar(menuStr);
				change = 1;
            }
        }
        else if (s2ch.pad.Buttons != s2ch.oldPad.Buttons)
        {
            s2ch.oldPad = s2ch.pad;
            if (s2ch.pad.Buttons & PSP_CTRL_SELECT)
            {
                s2ch.tateFlag = (s2ch.tateFlag) ? 0 : 1;
            }
            // STARTボタン
            else if(s2ch.pad.Buttons & PSP_CTRL_START)
            {
                psp2chMenu();
            }
            else if (rMenu)
            {
                // 全板検索
                if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.favH.search2ch) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.favV.search2ch))
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
                if (!focus)
                {
                    // ソート
                    if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.favH.sort) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.favV.sort))
                    {
                        psp2chFavSortDialog();
                    }
                    // 一括取得開始
                    if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.favH.update) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.favV.update))
                    {
                        update = 0;
                    }
                }
            }
            else
            {
                // 決定
                if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.favH.ok) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.favV.ok))
                {
                    if (focus)
                    {
                        if (s2ch.itaList == NULL)
                        {
                            if (psp2chItaList() < 0)
                            {
                                return 0;
                            }
                        }
                        for (i = 0; i < s2ch.ita.count; i++)
                        {
                            if (strcmp(s2ch.itaList[i].title, s2ch.favItaList[s2ch.favIta.select].title) == 0)
                            {
                                if (psp2chThreadList(i) < 0)
                                {
                                    return 0;
                                }
                                s2ch.ita.select = i;
                                s2ch.thread.start = 0;
                                s2ch.thread.select = 0;
                                s2ch.sel = 3;
                                return 0;
                            }
                        }
                    }
                    else
                    {
                        free(s2ch.resList);
                        s2ch.resList = NULL;
                        preLine = -2;
                        pgFillvram(WHITE, 0, 0, SCR_WIDTH, BUF_HEIGHT, 2);
                        s2ch.sel = 4;
                        return 0;
                    }
                }
                // 板一覧に移動
                else if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.favH.move) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.favV.move))
                {
                    s2ch.sel = 2;
                }
                // お気に入り切り替え
                else if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.favH.change) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.favV.change))
                {
                    focus = focus ? 0 : 1;
                    if (focus && s2ch.favItaList == NULL)
                    {
                        focus = 0;
                    }
                    else if (focus == 0 && s2ch.favList == NULL)
                    {
                        focus = 1;
                    }
                }
                // 削除
                else if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.favH.del) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.favV.del))
                {
                    if (focus)
                    {
                        psp2chDelFavoriteIta(s2ch.favIta.select);
                    }
                    else
                    {
                        psp2chDelFavorite(s2ch.favList[favSort[s2ch.fav.select]].title, s2ch.favList[favSort[s2ch.fav.select]].dat);
                    }
                }
            }
			pgPrintMenuBar(menuStr);
			change = 1;
        }
		res = psp2chPadSet(s2ch.viewX);
		if (res != s2ch.viewX || change)
		{
			s2ch.viewX = res;
			if (focus)
			{
				psp2chDrawFavoriteIta();
			}
			else
			{
				psp2chDrawFavorite();
			}
		}
        pgCopy(s2ch.viewX, 0);
        pgCopyMenuBar();
        sceDisplayWaitVblankStart();
        framebuffer = sceGuSwapBuffers();
    }
    return 0;
}

/**********************
idxファイルからレス数を読み込む
idxファイルがないときはdatの改行を数えidxファイルを作成
**********************/
int psp2chGetResCount(char* title, int dat)
{
    SceUID fd;
    char path[256];
    char buf[1024];
    char *p;
    int res, ret, i, range;

    sprintf(path, "%s/%s/%s/%d.idx", s2ch.cwDir, s2ch.logDir, title, dat);
    fd = sceIoOpen(path, PSP_O_RDONLY, 0777);
    if (fd < 0)
    {
        res = 0;
        sprintf(path, "%s/%s/%s/%d.dat", s2ch.cwDir, s2ch.logDir, title, dat);
        fd = sceIoOpen(path, PSP_O_RDONLY, 0777);
        if (fd >= 0)
        {
            range = 0;
            while((ret = sceIoRead(fd, buf, sizeof(buf))))
            {
                for (i = 0; i < ret; i++)
                {
                    if (buf[i] == '\n')
                    {
                        res++;
                    }
                }
                range += ret;
            }
            sceIoClose(fd);
            // idxファイル作成
            sprintf(path, "%s/%s/%s/%d.idx", s2ch.cwDir, s2ch.logDir, title, dat);
            fd = sceIoOpen(path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
            if (fd >= 0)
            {
                sprintf(buf, "Thu, 1 Jan 1970 00:00:00 GMT\n\"\"\n%d\n0\n0\n%d\n", range, res);
                sceIoWrite(fd, buf, strlen(buf));
                sceIoClose(fd);
            }
        }
    }
    else
    {
        sceIoRead(fd, path, 128);
        sceIoClose(fd);
        p = strchr(path, '\n'); // Last-Modified
        p++;
        p =  strchr(p, '\n'); // ETag
        p++;
        p =  strchr(p, '\n'); // Range
        p++;
        p =  strchr(p, '\n'); // res.start
        p++;
        p =  strchr(p, '\n'); // res.select
        p++;
        sscanf(p, "%d", &res );
    }
    return res;
}

/**********************
favorite.brdがあれば読み込んで
s2ch.favListのメモリ再確保とデータ作成
**********************/
int psp2chLoadFavorite(void)
{
    SceUID fd;
    SceIoStat st;
    char path[256];
    char *buf, *p, *r;
    int i;

    sprintf(path, "%s/%s/favorite.brd", s2ch.cwDir, s2ch.logDir);
    i = sceIoGetstat(path, &st);
    if (i < 0)
    {
        return -1;
    }
    buf = (char*)malloc(st.st_size + 1);
    if (buf == NULL)
    {
        psp2chErrorDialog("memorry error");
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
    s2ch.fav.count = 0;
    while (*p)
    {
        if (*p++ == '\n')
        {
            s2ch.fav.count++;
        }
    }
    if (s2ch.fav.count <= 0)
    {
        free(buf);
        return -1;
    }
    s2ch.favList = (S_2CH_FAVORITE*)realloc(s2ch.favList, sizeof(S_2CH_FAVORITE) * s2ch.fav.count);
    if (s2ch.favList == NULL)
    {
        psp2chErrorDialog("memorry error\nfavList");
        return -1;
    }
    favSort = (int*)realloc(favSort, sizeof(int) * s2ch.fav.count);
    if (favSort == NULL)
    {
        psp2chErrorDialog("memorry error\nfavSort");
        return -1;
    }
    r = buf;
    i = 0;
    while (*r)
    {
        p = strchr(r, '\t');
        *p= '\0';
        strcpy(s2ch.favList[i].host, r);
        r = ++p;
        p = strchr(r, '\t');
        *p= '\0';
        strcpy(s2ch.favList[i].dir, r);
        r = ++p;
        p = strchr(r, '\t');
        *p= '\0';
        strcpy(s2ch.favList[i].title, r);
        r = ++p;
        p = strchr(r, '\t');
        *p= '\0';
        sscanf(r, "%d", &s2ch.favList[i].dat);
        r = ++p;
        p = strchr(r, '\n');
        *p= '\0';
        strcpy(s2ch.favList[i].subject, r);
        r = ++p;
        s2ch.favList[i].res = psp2chGetResCount(s2ch.favList[i].title, s2ch.favList[i].dat);
        s2ch.favList[i].update = 0;
        i++;
    }
    free(buf);
    psp2chFavSort(0);
    return 0;
}

/**********************
favoriteita.brdがあれば読み込んで
s2ch.favItaListのメモリ再確保とデータ作成
**********************/
int psp2chLoadFavoriteIta(void)
{
    SceUID fd;
    SceIoStat st;
    char path[256];
    char *buf, *p, *r;
    int i;

    sprintf(path, "%s/%s/favoriteita.brd", s2ch.cwDir, s2ch.logDir);
    i = sceIoGetstat(path, &st);
    if (i < 0)
    {
        return -1;
    }
    buf = (char*)malloc(st.st_size + 1);
    if (buf == NULL)
    {
        psp2chErrorDialog("memorry error");
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
    s2ch.favIta.count = 0;
    while (*p)
    {
        if (*p++ == '\n')
        {
            s2ch.favIta.count++;
        }
    }
    if (s2ch.favIta.count <= 0)
    {
        free(buf);
        return -1;
    }
    s2ch.favItaList = (S_2CH_FAV_ITA*)realloc(s2ch.favItaList, sizeof(S_2CH_FAV_ITA) * s2ch.favIta.count);
    if (s2ch.favItaList == NULL)
    {
        psp2chErrorDialog("memorry error");
        return -1;
    }
    r = buf;
    i = 0;
    while (*r)
    {
        p = strchr(r, '\t');
        *p= '\0';
        strcpy(s2ch.favItaList[i].cate, r);
        r = ++p;
        p = strchr(r, '\n');
        *p= '\0';
        strcpy(s2ch.favItaList[i].title, r);
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

    if (s2ch.fav.count == 0)
    {
        psp2chLoadFavorite();
    }
    for (i = 0; i < s2ch.fav.count; i++)
    {
        if (s2ch.favList[i].dat == dat && strcmp(s2ch.favList[i].title, title) == 0)
        {
            psp2chErrorDialog(TEXT_8);
            return -1;
        }
    }
    sprintf(path, "%s/%s/favorite.brd", s2ch.cwDir, s2ch.logDir);
    fd = sceIoOpen(path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_APPEND, 0777);
    if (fd < 0)
    {
        return -1;
    }
    sprintf(path, "%s\t%s\t%s\t%d\t%s\n", host, dir, title, dat, s2ch.resList[0].title);
    sceIoWrite(fd, path, strlen(path));
    sceIoClose(fd);
    return psp2chLoadFavorite();
}

/**********************
favoriteita.brdの最後に追加
psp2chLoadFavoriteIta()でリストを作成しなおす
**********************/
int psp2chAddFavoriteIta(char* cate, char* title)
{
    SceUID fd;
    char path[256];
    int i;

    if (s2ch.favIta.count == 0)
    {
        psp2chLoadFavoriteIta();
    }
    for (i = 0; i < s2ch.favIta.count; i++)
    {
        if (strcmp(s2ch.favItaList[i].cate, cate) == 0 && strcmp(s2ch.favItaList[i].title, title) == 0)
        {
            psp2chErrorDialog(TEXT_8);
            return -1;
        }
    }
    sprintf(path, "%s/%s/favoriteita.brd", s2ch.cwDir, s2ch.logDir);
    fd = sceIoOpen(path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_APPEND, 0777);
    if (fd < 0)
    {
        return -1;
    }
    sprintf(path, "%s\t%s\n", cate, title);
    sceIoWrite(fd, path, strlen(path));
    sceIoClose(fd);
    return psp2chLoadFavoriteIta();
}

/**********************
s2ch.favListからtitleとdatの一致する項目以外のリストのみをfavorite.brdに書き出す
リストを作成しなおす
**********************/
int psp2chDelFavorite(char* title, int dat)
{
    SceUID fd;
    char path[256];
    int i, del;

    if (s2ch.favList == NULL || s2ch.fav.count <= 0)
    {
        return -1;
    }
    memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
    s2ch.mh.options = PSP_UTILITY_MSGDIALOG_OPTION_TEXT | PSP_UTILITY_MSGDIALOG_OPTION_YESNO_BUTTONS;
    strcpy(s2ch.mh.message, TEXT_9);
    pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
    sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
    if (s2ch.mh.buttonPressed == PSP_UTILITY_MSGDIALOG_RESULT_YES)
    {
        sprintf(path, "%s/%s/favorite.brd", s2ch.cwDir, s2ch.logDir);
        fd = sceIoOpen(path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
        if (fd < 0)
        {
            return -1;
        }
        for (i = 0, del = -1; i < s2ch.fav.count; i++)
        {
            if (s2ch.favList[i].dat == dat && strcmp(s2ch.favList[i].title, title) == 0)
            {
                del = i;
                continue;
            }
            sprintf(path, "%s\t%s\t%s\t%d\t%s\n", s2ch.favList[i].host, s2ch.favList[i].dir, s2ch.favList[i].title, s2ch.favList[i].dat, s2ch.favList[i].subject);
            sceIoWrite(fd, path, strlen(path));
        }
        sceIoClose(fd);
        s2ch.fav.start = 0;
        s2ch.fav.select = 0;
        if (del >= 0)
        {
            s2ch.fav.count--;
            for (i = del; i < s2ch.fav.count; i++)
            {
                s2ch.favList[i] = s2ch.favList[i+1];
            }
        }
    }
    psp2chFavSort(-1);
    return 0;
}

/**********************
s2ch.favItaListからindex以外のリストのみをfavoriteita.brdに書き出す
psp2chLoadFavoriteIta()でリストを作成しなおす
**********************/
int psp2chDelFavoriteIta(int index)
{
    SceUID fd;
    char path[256];
    int i;

    if (s2ch.favItaList == NULL || s2ch.favIta.count <= 0)
    {
        return -1;
    }
    memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
    s2ch.mh.options = PSP_UTILITY_MSGDIALOG_OPTION_TEXT | PSP_UTILITY_MSGDIALOG_OPTION_YESNO_BUTTONS;
    strcpy(s2ch.mh.message, TEXT_12);
    pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
    sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
    if (s2ch.mh.buttonPressed == PSP_UTILITY_MSGDIALOG_RESULT_YES)
    {
        sprintf(path, "%s/%s/favoriteIta.brd", s2ch.cwDir, s2ch.logDir);
        fd = sceIoOpen(path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
        if (fd < 0)
        {
            return -1;
        }
        for (i = 0; i < s2ch.favIta.count; i++)
        {
            if (i == index)
            {
                continue;
            }
            sprintf(path, "%s\t%s\n", s2ch.favItaList[i].cate, s2ch.favItaList[i].title);
            sceIoWrite(fd, path, strlen(path));
        }
        sceIoClose(fd);
        s2ch.favIta.start = 0;
        s2ch.favIta.select = 0;
        return psp2chLoadFavoriteIta();
    }
    return 0;
}

/****************
お気に入りスレッドをソートする
favSort配列にソートデータが入る
sort:
0=板名
1=スレタイ
2=作成日(降順)
3=作成日(昇順)
*****************/
void psp2chFavSort(int sort)
{
    static int s = 0;
    int i, j, tmp;

    if (sort >= 0)
    {
        s = sort;
    }
    switch (s)
    {
    case 0:
        for (i = 0; i < s2ch.fav.count; i++)
        {
            favSort[i] = i;
        }
        for (i = 0; i < s2ch.fav.count-1; i++)
        {
            for (j = i; j < s2ch.fav.count; j++)
            {
                if (strcmp(s2ch.favList[favSort[j]].title, s2ch.favList[favSort[i]].title) < 0)
                {
                    tmp = favSort[j];
                    favSort[j] = favSort[i];
                    favSort[i] = tmp;
                }
                else if (strcmp(s2ch.favList[favSort[j]].title, s2ch.favList[favSort[i]].title) == 0)
                {
                    if (strcmp(s2ch.favList[favSort[j]].subject, s2ch.favList[favSort[i]].subject) < 0)
                    {
                        tmp = favSort[j];
                        favSort[j] = favSort[i];
                        favSort[i] = tmp;
                    }
                }
            }
        }
        break;
    case 1:
        for (i = 0; i < s2ch.fav.count; i++)
        {
            favSort[i] = i;
        }
        for (i = 0; i < s2ch.fav.count-1; i++)
        {
            for (j = i; j < s2ch.fav.count; j++)
            {
                if (strcmp(s2ch.favList[favSort[j]].subject, s2ch.favList[favSort[i]].subject) < 0)
                {
                    tmp = favSort[j];
                    favSort[j] = favSort[i];
                    favSort[i] = tmp;
                }
            }
        }
        break;
    case 2:
        for (i = 0; i < s2ch.fav.count; i++)
        {
            favSort[i] = i;
        }
        for (i = 0; i < s2ch.fav.count-1; i++)
        {
            for (j = i; j < s2ch.fav.count; j++)
            {
                if (s2ch.favList[favSort[j]].dat > s2ch.favList[favSort[i]].dat)
                {
                    tmp = favSort[j];
                    favSort[j] = favSort[i];
                    favSort[i] = tmp;
                }
            }
        }
        break;
    case 3:
        for (i = 0; i < s2ch.fav.count; i++)
        {
            favSort[i] = i;
        }
        for (i = 0; i < s2ch.fav.count-1; i++)
        {
            for (j = i; j < s2ch.fav.count; j++)
            {
                if (s2ch.favList[favSort[j]].dat < s2ch.favList[favSort[i]].dat)
                {
                    tmp = favSort[j];
                    favSort[j] = favSort[i];
                    favSort[i] = tmp;
                }
            }
        }
        break;
    }
}

/****************
ソート用ダイアログ表示
*****************/
#define MAX_SORT_COUNT (4)
void psp2chFavSortDialog(void)
{
    const unsigned short title[] = {0x3069,0x306E,0x9805,0x76EE,0x3067,0x30BD,0x30FC,0x30C8,0x3057,0x307E,0x3059,0x304B,0};
    const unsigned short text1[] = {0x677F,0x540D,0}; // 板名
    const unsigned short text2[] = {0x30B9,0x30EC,0x30BF,0x30A4,0}; // スレタイ
    const unsigned short text3[] = {0x4F5C,0x6210,0x65E5,0x0028,0x964D,0x9806,0x0029,0}; // 作成日(降順)
    const unsigned short text4[] = {0x4F5C,0x6210,0x65E5,0x0028,0x6607,0x9806,0x0029,0}; // 作成日(昇順)
    const unsigned short* text[MAX_SORT_COUNT] = {text1, text2, text3, text4};
    int i, select = 0;

    while (s2ch.running)
    {
        if(sceCtrlPeekBufferPositive(&s2ch.pad, 1))
        {
            if (s2ch.pad.Buttons != s2ch.oldPad.Buttons)
            {
                s2ch.oldPad = s2ch.pad;
                if(s2ch.pad.Buttons & PSP_CTRL_UP)
                {
                    if (select)
                    {
                        select--;
                    }
                }
                if(s2ch.pad.Buttons & PSP_CTRL_DOWN)
                {
                    if (select < MAX_SORT_COUNT - 1)
                    {
                        select++;
                    }
                }
                if(s2ch.pad.Buttons & PSP_CTRL_CIRCLE)
                {
                    break;
                }
                if(s2ch.pad.Buttons & PSP_CTRL_CROSS)
                {
                    return;
                }
            }
            sceGuStart(GU_DIRECT, list);
			sceGuScissor(0, 0, SCR_WIDTH, SCR_HEIGHT);
            sceGuClearColor(0xFFFF0000);
            sceGuClear(GU_COLOR_BUFFER_BIT);
            s2ch.pgCursorX = 240;
            s2ch.pgCursorY =  77;
            intraFontSetStyle(jpn0, 1.0f, 0xFF00FFFF, 0xFFFF0000, INTRAFONT_ALIGN_CENTER);
            intraFontPrintUCS2(jpn0, s2ch.pgCursorX, s2ch.pgCursorY, title);
            s2ch.pgCursorX = 240;
            s2ch.pgCursorY += 25;
            for (i = 0; i < MAX_SORT_COUNT; i++)
            {
                if (select == i)
                {
                    intraFontSetStyle(jpn0, 0.9f, 0xFFFFFFFF, 0xFF000000, INTRAFONT_ALIGN_CENTER);
                    intraFontPrintUCS2(jpn0, s2ch.pgCursorX, s2ch.pgCursorY, text[i]);
                }
                else
                {
                    intraFontSetStyle(jpn0, 0.9f, 0xFFCCCCCC, 0, INTRAFONT_ALIGN_CENTER);
                    intraFontPrintUCS2(jpn0, s2ch.pgCursorX, s2ch.pgCursorY, text[i]);
                }
                s2ch.pgCursorX = 240;
                s2ch.pgCursorY += 20;
            }
            sceGuFinish();
            sceGuSync(0,0);
            sceDisplayWaitVblankStart();
            framebuffer = sceGuSwapBuffers();
        }
    }
    return psp2chFavSort(select);
}

/**********************
お気に入りスレの描画
**********************/
void psp2chDrawFavorite(void)
{
    int start;
    int i;
    int lineEnd, scrW, scrH;

    if (s2ch.tateFlag)
    {
        lineEnd = DRAW_LINE_V;
        scrW = SCR_HEIGHT + s2ch.viewX;
        scrH = SCR_WIDTH;
    }
    else
    {
        lineEnd = DRAW_LINE_H;
        scrW = SCR_WIDTH + s2ch.viewX;
        scrH = SCR_HEIGHT;
    }
    start = s2ch.fav.start;
    if (start + lineEnd > s2ch.fav.count)
    {
        start = s2ch.fav.count - lineEnd;
    }
    if (start < 0)
    {
        start = 0;
    }
    pgFillvram(s2ch.threadColor.bg, s2ch.viewX, 0, scrW, BUF_HEIGHT, 2);
    s2ch.pgCursorY = 0;
    for (i = start; i < start + lineEnd; i++)
    {
        if (i >= s2ch.fav.count)
        {
            return;
        }
        s2ch.pgCursorX = 0;
        if (i == s2ch.fav.select)
        {
            pgFillvram(s2ch.threadColor.s_bg, s2ch.viewX, s2ch.pgCursorY, scrW, LINE_PITCH, 2);
            pgPrintNumber(i + 1, s2ch.threadColor.s_num, s2ch.threadColor.s_bg);
        }
        else
        {
            pgPrintNumber(i + 1, s2ch.threadColor.num, s2ch.threadColor.bg);
        }
        s2ch.pgCursorX = FONT_HEIGHT * 2;
        if (i == s2ch.fav.select)
        {
            pgPrint(s2ch.favList[favSort[i]].title, s2ch.threadColor.s_category, s2ch.threadColor.s_bg, scrW);
            s2ch.pgCursorX += 8;
            pgPrint(s2ch.favList[favSort[i]].subject, s2ch.threadColor.s_text1, s2ch.threadColor.s_bg, scrW);
            s2ch.pgCursorX = scrW - FONT_HEIGHT * 2;
            if (s2ch.favList[favSort[i]].update)
            {
                pgPrintNumber(s2ch.favList[favSort[i]].res, s2ch.threadColor.s_count2, s2ch.threadColor.s_bg);
            }
            else
            {
                pgPrintNumber(s2ch.favList[favSort[i]].res, s2ch.threadColor.s_count1, s2ch.threadColor.s_bg);
            }
        }
        else
        {
            pgPrint(s2ch.favList[favSort[i]].title, s2ch.threadColor.category, s2ch.threadColor.bg, scrW);
            s2ch.pgCursorX += 8;
            pgPrint(s2ch.favList[favSort[i]].subject, s2ch.threadColor.text1, s2ch.threadColor.bg, scrW);
            s2ch.pgCursorX = scrW - FONT_HEIGHT * 2;
            if (s2ch.favList[favSort[i]].update)
            {
                pgPrintNumber(s2ch.favList[favSort[i]].res, s2ch.threadColor.count2, s2ch.threadColor.bg);
            }
            else
            {
                pgPrintNumber(s2ch.favList[favSort[i]].res, s2ch.threadColor.count1, s2ch.threadColor.bg);
            }
        }
        s2ch.pgCursorY += LINE_PITCH;
    }
}

/**********************
お気に入り板の描画
**********************/
void psp2chDrawFavoriteIta(void)
{
    int start;
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
    start = s2ch.favIta.start;
    if (start + lineEnd > s2ch.favIta.count)
    {
        start = s2ch.favIta.count - lineEnd;
    }
    if (start < 0)
    {
        start = 0;
    }
    pgFillvram(s2ch.threadColor.bg, 0, 0, BUF_WIDTH * 2, BUF_HEIGHT, 2);
    s2ch.pgCursorY = 0;
    for (i = start; i < start + lineEnd; i++)
    {
        if (i >= s2ch.favIta.count)
        {
            return;
        }
        s2ch.pgCursorX = 0;
        if (i == s2ch.favIta.select)
        {
            pgFillvram(s2ch.threadColor.s_bg, 0, s2ch.pgCursorY, BUF_WIDTH * 2, LINE_PITCH, 2);
            pgPrintNumber(i + 1, s2ch.threadColor.s_num, s2ch.threadColor.s_bg);
        }
        else
        {
            pgPrintNumber(i + 1, s2ch.threadColor.num, s2ch.threadColor.bg);
        }
        s2ch.pgCursorX = 30;
        if (i == s2ch.favIta.select)
        {
            pgPrint(s2ch.favItaList[i].cate, s2ch.threadColor.s_category, s2ch.threadColor.s_bg, scrW);
            s2ch.pgCursorX = 100;
            pgPrint(s2ch.favItaList[i].title, s2ch.threadColor.s_text1, s2ch.threadColor.s_bg, scrW);
        }
        else
        {
            pgPrint(s2ch.favItaList[i].cate, s2ch.threadColor.category, s2ch.threadColor.bg, scrW);
            s2ch.pgCursorX = 100;
            pgPrint(s2ch.favItaList[i].title, s2ch.threadColor.text1, s2ch.threadColor.bg, scrW);
        }
        s2ch.pgCursorY += LINE_PITCH;
    }
}
