/*
* $Id$
*/

#include <stdio.h>
#include <malloc.h>
#include <time.h>
#include <pspdebug.h>
#include "pg.h"
#include "psp2ch.h"
#include "psp2chNet.h"
#include "psp2chIta.h"
#include "psp2chThread.h"
#include "psp2chRes.h"
#include "psp2chResWindow.h"
#include "psp2chFavorite.h"
#include "psp2chForm.h"
#include "psp2chMenu.h"
#include "utf8.h"

extern S_2CH s2ch; // psp2ch.c
extern int* favSort; // psp2chFavorite.c
extern int* threadSort; // psp2chThread.c
extern const char* ngNameFile; // psp2chMenu.c
extern const char* ngIDFile; // psp2chMenu.c
extern const char* ngWordFile; // psp2chMenu.c
extern const char* ngMailFile; // psp2chMenu.c

int preLine = -2;

static char* resBuffer = NULL;
static char jmpHost[32], jmpDir[32], jmpTitle[32];
static int jmpDat;
const char *sBtnH[] = {"Sel", "", "", "St", "↑", "→", "↓", "←", "L", "R", "", "", "△", "○", "×", "□", ""};
const char *sBtnV[] = {"Sel", "", "", "St", "←", "↑", "→", "↓", "L", "R", "", "", "△", "○", "×", "□", ""};

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

void psp2chResSetMenuString(void)
{
    int index1, index2, index3, index4, index5;
    int i, tmp;

    getIndex(s2ch.btnResH.form, index1);
    getIndex(s2ch.btnResH.back, index2);
    getIndex(s2ch.btnResH.reload, index3);
    getIndex(s2ch.btnResH.datDel, index4);
    getIndex(s2ch.btnResH.change, index5);
    sprintf(s2ch.menuResH.main, "　%s : 書き込み　　%s : 戻る　　%s : 更新　　%s : 削除　　%s : メニュー切替",
            sBtnH[index1], sBtnH[index2], sBtnH[index3], sBtnH[index4], sBtnH[index5]);

    getIndex(s2ch.btnResH.s.top, index1);
    getIndex(s2ch.btnResH.s.end, index2);
    getIndex(s2ch.btnResH.addFav, index3);
    getIndex(s2ch.btnResH.delFav, index4);
    sprintf(s2ch.menuResH.sub1, "　%s : 先頭　　　%s : 最後　　　%s : お気に入りに登録",
            sBtnH[index1], sBtnH[index2], sBtnH[index3]);
    sprintf(s2ch.menuResH.sub2, "　%s : 先頭　　　%s : 最後　　　%s : お気に入りから削除",
            sBtnH[index1], sBtnH[index2], sBtnH[index4]);

    getIndex(s2ch.btnResH.resForm, index1);
    getIndex(s2ch.btnResH.resFBack, index2);
    sprintf(s2ch.menuResH.aNum, "　%s : レスをする　　　%s : 戻る",
            sBtnH[index1], sBtnH[index2]);

    getIndex(s2ch.btnResH.idView, index1);
    getIndex(s2ch.btnResH.idNG, index2);
    getIndex(s2ch.btnResH.idBack, index3);
    sprintf(s2ch.menuResH.aId, "　%s : ID抽出　　　%s : NGID登録　　　%s : 戻る",
            sBtnH[index1], sBtnH[index2], sBtnH[index3]);

    getIndex(s2ch.btnResH.resView, index1);
    getIndex(s2ch.btnResH.resMove, index2);
    getIndex(s2ch.btnResH.resBack, index3);
    sprintf(s2ch.menuResH.aRes, "　%s : レス表\示　　　%s : レスに移動　　　%s : 戻る",
            sBtnH[index1], sBtnH[index2], sBtnH[index3]);

    getIndex(s2ch.btnResH.url, index1);
    getIndex(s2ch.btnResH.urlBack, index2);
    sprintf(s2ch.menuResH.aUrl, "　%s : リンク表\示　　　%s : 戻る",
            sBtnH[index1], sBtnH[index2]);

    getIndex(s2ch.btnResV.form, index1);
    getIndex(s2ch.btnResV.back, index2);
    getIndex(s2ch.btnResV.reload, index3);
    getIndex(s2ch.btnResV.datDel, index4);
    getIndex(s2ch.btnResV.change, index5);
    sprintf(s2ch.menuResV.main, "　%s : 書き込み　　　%s : 戻る　　　　%s : 更新\n　%s : 削除　　　%s : メニュー切替",
            sBtnV[index1], sBtnV[index2], sBtnV[index3], sBtnV[index4], sBtnV[index5]);

    getIndex(s2ch.btnResV.s.top, index1);
    getIndex(s2ch.btnResV.s.end, index2);
    getIndex(s2ch.btnResV.addFav, index3);
    getIndex(s2ch.btnResV.delFav, index4);
    sprintf(s2ch.menuResV.sub1, "　%s : 先頭　　　%s : 最後\n　%s : お気に入りに登録",
            sBtnV[index1], sBtnV[index2], sBtnV[index3]);
    sprintf(s2ch.menuResV.sub2, "　%s : 先頭　　　%s : 最後\n　%s : お気に入りから削除",
            sBtnV[index1], sBtnV[index2], sBtnV[index4]);

    getIndex(s2ch.btnResV.resForm, index1);
    getIndex(s2ch.btnResV.resFBack, index2);
    sprintf(s2ch.menuResV.aNum, "　%s : レスをする　　　%s : 戻る",
            sBtnV[index1], sBtnV[index2]);

    getIndex(s2ch.btnResV.idView, index1);
    getIndex(s2ch.btnResV.idNG, index2);
    getIndex(s2ch.btnResV.idBack, index3);
    sprintf(s2ch.menuResV.aId, "　%s : ID抽出　　　%s : NGID登録　　　%s : 戻る",
            sBtnV[index1], sBtnV[index2], sBtnV[index3]);

    getIndex(s2ch.btnResV.resView, index1);
    getIndex(s2ch.btnResV.resMove, index2);
    getIndex(s2ch.btnResV.resBack, index3);
    sprintf(s2ch.menuResV.aRes, "　%s : レス表\示　　　%s : レスに移動　　　%s : 戻る",
            sBtnV[index1], sBtnV[index2], sBtnV[index3]);

    getIndex(s2ch.btnResV.url, index1);
    getIndex(s2ch.btnResV.urlBack, index2);
    sprintf(s2ch.menuResV.aUrl, "　%s : リンク表\示　　　%s : 戻る",
            sBtnV[index1], sBtnV[index2]);
}

/**************
  レス表示
***************/
int psp2chFavoriteRes(int ret)
{
    return psp2chRes(s2ch.favList[favSort[s2ch.fav.select]].host, s2ch.favList[favSort[s2ch.fav.select]].dir, s2ch.favList[favSort[s2ch.fav.select]].title, s2ch.favList[favSort[s2ch.fav.select]].dat, ret);
}
int psp2chThreadRes(int ret)
{
    return psp2chRes(s2ch.itaList[s2ch.ita.select].host, s2ch.itaList[s2ch.ita.select].dir,s2ch.itaList[s2ch.ita.select].title,s2ch.threadList[threadSort[s2ch.thread.select]].dat,ret);
}
int psp2chJumpRes(int ret)
{
    return psp2chRes(jmpHost, jmpDir, jmpTitle, jmpDat, ret);
}
int psp2chSearchRes(int ret)
{
    return psp2chRes(s2ch.findList[s2ch.find.select].host, s2ch.findList[s2ch.find.select].dir, s2ch.findList[s2ch.find.select].title, s2ch.findList[s2ch.find.select].dat, ret);
}
int psp2chRes(char* host, char* dir, char* title, int dat, int ret)
{
    static int cursorX = 240, cursorY = 130;
    static int totalLine = 0;
    static S_SCROLLBAR bar;
    static char* message = NULL;
    char path[256];
    char *p, *q;
    static char* menuStr = NULL;
    int i, j, tmp;
    static int resMenu = -1, urlMenu = -1, idMenu = -1, numMenu = -1;
    int lineEnd, rMenu;

    if (s2ch.tateFlag)
    {
        lineEnd = DRAW_LINE_V;
    }
    else
    {
        lineEnd = DRAW_LINE_H;
    }
    if (s2ch.resList == NULL)
    {
        if (psp2chResList(host, dir, title, dat) < 0)
        {
            s2ch.sel = ret;
            return -1;
        }
        totalLine = psp2chResSetLine(&bar);
        if (s2ch.res.start > totalLine - lineEnd)
        {
            s2ch.res.start = totalLine - lineEnd;
        }
        if (s2ch.res.start < 0)
        {
            s2ch.res.start = 0;
        }
        s2ch.viewX = 0;
        s2ch.viewY = 0;
    }
    if(sceCtrlPeekBufferPositive(&s2ch.pad, 1))
    {
        rMenu = psp2chResCursorMove(&totalLine, &lineEnd, &cursorY, bar.view);
        if (s2ch.pad.Buttons != s2ch.oldPad.Buttons)
        {
            s2ch.oldPad = s2ch.pad;
            // SELECTボタン
            if (s2ch.pad.Buttons & PSP_CTRL_SELECT)
            {
                for (i = 0; s2ch.res.start > s2ch.resList[i].line; i++)
                {
                    if (s2ch.resList[i].ng == 0)
                    {
                        s2ch.res.start -= s2ch.resList[i].line;
                        s2ch.res.start--;
                    }
                }
                s2ch.tateFlag = (s2ch.tateFlag) ? 0 : 1;
                totalLine = psp2chResSetLine(&bar);
                for (j = 0; j < i; j++)
                {
                    if (s2ch.resList[j].ng == 0)
                    {
                        s2ch.res.start += s2ch.resList[j].line;
                        s2ch.res.start++;
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
                if (s2ch.res.start > totalLine - lineEnd)
                {
                    s2ch.res.start = totalLine - lineEnd;
                }
                if (s2ch.res.start < 0)
                {
                    s2ch.res.start = 0;
                }
                psp2chResResetAnchors();
                preLine = -2;
            }
            // STARTボタン
            else if(s2ch.pad.Buttons & PSP_CTRL_START)
            {
                psp2chMenu();
                totalLine = psp2chResSetLine(&bar);
                if (s2ch.res.start > totalLine - lineEnd)
                {
                    s2ch.res.start = totalLine - lineEnd;
                }
                if (s2ch.res.start < 0)
                {
                    s2ch.res.start = 0;
                }
                preLine = -2;
            }
            // レスアンカーメニュー
            else if (resMenu >= 0)
            {
                // レスウィンドウ表示
                if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.btnResH.resView) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.btnResV.resView))
                {
                    psp2chResAnchor(resMenu);
                }
                // アンカーレス番に移動
                else if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.btnResH.resMove) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.btnResV.resMove))
                {
                    if (s2ch.resList[s2ch.resAnchor[resMenu].res[0]].ng == 0)
                    {
                        for (i = 0, j = 0; i < s2ch.resAnchor[resMenu].res[0]; i++)
                        {
                            if (s2ch.resList[i].ng == 0)
                            {
                                j += s2ch.resList[i].line;
                                j++;
                            }
                        }
                        s2ch.res.start = j;
                        if (s2ch.res.start > totalLine - lineEnd)
                        {
                            s2ch.res.start = totalLine - lineEnd;
                        }
                        if (s2ch.res.start < 0)
                        {
                            s2ch.res.start = 0;
                        }
                    }
                }
                // 戻る
                else if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.btnResH.resBack) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.btnResV.resBack))
                {
                    psp2chSaveIdx(title, dat);
                    if (s2ch.threadList)
                    {
                        psp2chSort(-1); // 前回のソート順で再ソート
                    }
                    s2ch.sel = ret;
                    return ret;
                }
            }
            // URLアンカーメニュー
            else if (urlMenu >= 0)
            {
                if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.btnResH.url) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.btnResV.url))
                {
                    psp2chSaveIdx(title, dat);
                    /* 2ちゃんリンク移動 */
                    if ((p = strstr(s2ch.urlAnchor[urlMenu].host, ".2ch.net")) && p[8] == '\0')
                    {
                        if (s2ch.itaList == NULL)
                        {
                            if (psp2chItaList() < 0)
                            {
                                return ret;
                            }
                        }
                        strcpy(jmpHost, s2ch.urlAnchor[urlMenu].host);
                        memcpy(path, s2ch.urlAnchor[urlMenu].path, 256);
                        path[255] = '\0';
                        if ((p = strstr(path, "test/read.cgi/")))
                        {
                            p += 14;
                            q = strchr(p, '/');
                            if (!q)
                            {
                                return ret;
                            }
                            *q = '\0';
                            memcpy(jmpDir, p, 32);
                            jmpDir[31] = '\0';
                            q++;
                            sscanf(q, "%d", &jmpDat);
                            jmpTitle[0] = '\0';
                            for (i = 0; i < s2ch.ita.count; i++)
                            {
                                if (strcmp(s2ch.itaList[i].host, s2ch.urlAnchor[urlMenu].host) == 0 &&
                                        strcmp(s2ch.itaList[i].dir, jmpDir) == 0)
                                {
                                    strcpy(jmpTitle, s2ch.itaList[i].title);
                                    break;
                                }
                            }
                            if (jmpTitle[0] == '\0')
                            {
                                return ret;
                            }
                        }
                        else
                        {
                            return ret;
                        }
                        memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
                        s2ch.mh.options = PSP_UTILITY_MSGDIALOG_OPTION_TEXT |
                            PSP_UTILITY_MSGDIALOG_OPTION_YESNO_BUTTONS | PSP_UTILITY_MSGDIALOG_OPTION_DEFAULT_NO;
                        strcpy(s2ch.mh.message, TEXT_11);
                        pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
                        sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
                        if (s2ch.mh.buttonPressed == PSP_UTILITY_MSGDIALOG_RESULT_YES)
                        {
                            free(s2ch.resList);
                            s2ch.resList = NULL;
                            s2ch.res.start = 0;
                            s2ch.sel = 6;
                            return ret;
                        }
                    }
                    /* 外部リンク */
                    else
                    {
                        psp2chUrlAnchor(urlMenu, title, dat, s2ch.res.start*LINE_PITCH);
                    }
                }
                // 戻る
                else if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.btnResH.urlBack) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.btnResV.urlBack))
                {
                    psp2chSaveIdx(title, dat);
                    if (s2ch.threadList)
                    {
                        psp2chSort(-1); // 前回のソート順で再ソート
                    }
                    s2ch.sel = ret;
                    return ret;
                }
            }
            // IDメニュー
            else if (idMenu >= 0)
            {
                if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.btnResH.idView) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.btnResV.idView))
                {
                    psp2chIdAnchor(idMenu);
                }
                else if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.btnResH.idNG) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.btnResV.idNG))
                {
                    psp2chNGAdd(ngIDFile, s2ch.idAnchor[idMenu].id);
                    psp2chResCheckNG();
                    totalLine = psp2chResSetLine(&bar);
                    if (s2ch.res.start > totalLine - lineEnd)
                    {
                        s2ch.res.start = totalLine - lineEnd;
                    }
                    preLine = -2;
                }
                // 戻る
                else if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.btnResH.idBack) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.btnResV.idBack))
                {
                    psp2chSaveIdx(title, dat);
                    if (s2ch.threadList)
                    {
                        psp2chSort(-1); // 前回のソート順で再ソート
                    }
                    s2ch.sel = ret;
                    return ret;
                }
            }
            // レス番号メニュー
            else if (numMenu >= 0)
            {
                if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.btnResH.resForm) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.btnResV.resForm))
                {
                    tmp = s2ch.tateFlag;
                    s2ch.tateFlag = 0;
                    if (message == NULL)
                    {
                        message = (char*)calloc(1, 2048);
                    }
                    if (message == NULL)
                    {
                        psp2chErrorDialog("memorry error\nForm message");
                        return 0;
                    }
                    sprintf(path, ">>%d\n", s2ch.numAnchor[numMenu].num + 1);
                    strcat(message, path);
                    if (psp2chForm(host, dir, dat, s2ch.resList[0].title, message) > 0)
                    {
                        psp2chSaveIdx(title, dat);
                        psp2chGetDat(host, dir, title, dat);
                        psp2chResList(host, dir, title, dat);
                        s2ch.res.start++;
                        totalLine = psp2chResSetLine(&bar);
                    }
                    s2ch.tateFlag = tmp;
                    preLine = -2;
                }
                // 戻る
                else if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.btnResH.resFBack) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.btnResV.resFBack))
                {
                    psp2chSaveIdx(title, dat);
                    if (s2ch.threadList)
                    {
                        psp2chSort(-1); // 前回のソート順で再ソート
                    }
                    s2ch.sel = ret;
                    return ret;
                }
            }
            // ノーマルメニュー
            else
            {
                if (rMenu)
                {
                    // お気に入り追加
                    if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.btnResH.addFav) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.btnResV.addFav))
                    {
                        psp2chAddFavorite(host, dir, title, dat);
                    }
                    // お気に入り削除
                    else if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.btnResH.delFav) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.btnResV.delFav))
                    {
                        psp2chDelFavorite(title, dat);
                    }
                }
                else
                {
                    // 戻る
                    if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.btnResH.back) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.btnResV.back))
                    {
                        psp2chSaveIdx(title, dat);
                        if (s2ch.threadList)
                        {
                            psp2chSort(-1); // 前回のソート順で再ソート
                        }
                        s2ch.sel = ret;
                        return ret;
                    }
                    // 書き込み
                    else if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.btnResH.form) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.btnResV.form))
                    {
                        tmp = s2ch.tateFlag;
                        s2ch.tateFlag = 0;
                        if (message == NULL)
                        {
                            message = (char*)calloc(1, 2048);
                        }
                        if (message == NULL)
                        {
                            psp2chErrorDialog("memorry error\nForm message");
                            return 0;
                        }
                        // 書き込みがあったときのみ更新
                        if (psp2chForm(host, dir, dat, s2ch.resList[0].title, message) > 0)
                        {
                            psp2chSaveIdx(title, dat);
                            psp2chGetDat(host, dir, title, dat);
                            psp2chResList(host, dir, title, dat);
                            s2ch.res.start++;
                            totalLine = psp2chResSetLine(&bar);
                        }
                        s2ch.tateFlag = tmp;
                        preLine = -2;
                    }
                    // 更新
                    else if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.btnResH.reload) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.btnResV.reload))
                    {
                        psp2chSaveIdx(title, dat);
                        if (psp2chGetDat(host, dir, title, dat) == 0)
                        {
                            psp2chResList(host, dir, title, dat);
                            totalLine = psp2chResSetLine(&bar);
                            s2ch.res.start++;
                            if (s2ch.res.start > totalLine - lineEnd)
                            {
                                s2ch.res.start = totalLine - lineEnd;
                            }
                            if (s2ch.res.start < 0)
                            {
                                s2ch.res.start = 0;
                            }
                        }
                    }
                    // DAT削除
                    else if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.btnResH.datDel) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.btnResV.datDel))
                    {
                        memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
                        s2ch.mh.options = PSP_UTILITY_MSGDIALOG_OPTION_TEXT |
                            PSP_UTILITY_MSGDIALOG_OPTION_YESNO_BUTTONS | PSP_UTILITY_MSGDIALOG_OPTION_DEFAULT_NO;
                        strcpy(s2ch.mh.message, TEXT_5);
                        pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
                        sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
                        if (s2ch.mh.buttonPressed == PSP_UTILITY_MSGDIALOG_RESULT_YES)
                        {
                            sprintf(path, "%s/%s/%s/%d.dat", s2ch.cwDir, s2ch.logDir, title, dat);
                            sceIoRemove(path);
                            sprintf(path, "%s/%s/%s/%d.idx", s2ch.cwDir, s2ch.logDir, title, dat);
                            sceIoRemove(path);
                            if (s2ch.threadList)
                            {
                                s2ch.threadList[threadSort[s2ch.thread.select]].old = 0;
                                psp2chSort(-1);
                            }
                            s2ch.sel = ret;
                            return 0;
                        }
                    }
                }
            }
        }
        psp2chResPadMove(&cursorX, &cursorY, bar.x, bar.view);
        // 矢印カーソルにレスアンカーリンクがあるか
        for (i = 0; i < 50; i++)
        {
            if (cursorY/LINE_PITCH+s2ch.res.start == s2ch.resAnchor[i].line &&
                cursorX > s2ch.resAnchor[i].x1 && cursorX < s2ch.resAnchor[i].x2)
            {
                resMenu = i;
                break;
            }
            else
            {
                resMenu = -1;
            }
        }
        // URLリンクがあるか
        for (i = 0; i < 50; i++)
        {
            if (cursorY/LINE_PITCH+s2ch.res.start == s2ch.urlAnchor[i].line &&
                cursorX > s2ch.urlAnchor[i].x1 && cursorX < s2ch.urlAnchor[i].x2)
            {
                urlMenu = i;
                break;
            }
            else
            {
                urlMenu = -1;
            }
        }
        // IDの場所か
        for (i = 0; i < 40; i++)
        {
            if (cursorY/LINE_PITCH+s2ch.res.start == s2ch.idAnchor[i].line &&
                cursorX > s2ch.idAnchor[i].x1 && cursorX < s2ch.idAnchor[i].x2)
            {
                idMenu = i;
                break;
            }
            else
            {
                idMenu = -1;
            }
        }
        // 番号の場所か
        for (i = 0; i < 40; i++)
        {
            if (cursorY/LINE_PITCH+s2ch.res.start == s2ch.numAnchor[i].line &&
                cursorX > s2ch.numAnchor[i].x1 && cursorX < s2ch.numAnchor[i].x2)
            {
                numMenu = i;
                break;
            }
            else
            {
                numMenu = -1;
            }
        }
        if (resMenu >= 0)
        {
            if (s2ch.tateFlag)
            {
                menuStr = s2ch.menuResV.aRes;
            }
            else
            {
                menuStr = s2ch.menuResH.aRes;
            }
        }
        else if (urlMenu >= 0)
        {
            if (s2ch.tateFlag)
            {
                menuStr = s2ch.menuResV.aUrl;
            }
            else
            {
                menuStr = s2ch.menuResH.aUrl;
            }
        }
        else if (idMenu >= 0)
        {
            if (s2ch.tateFlag)
            {
                menuStr = s2ch.menuResV.aId;
            }
            else
            {
                menuStr = s2ch.menuResH.aId;
            }
        }
        else if (numMenu >= 0)
        {
            if (s2ch.tateFlag)
            {
                menuStr = s2ch.menuResV.aNum;
            }
            else
            {
                menuStr = s2ch.menuResH.aNum;
            }
        }
        else if (rMenu)
        {
            // お気に入りリストにあるかチェック
            j = 0;
            if (s2ch.fav.count)
            {
                for (i = 0; i < s2ch.fav.count; i++)
                {
                    if (s2ch.favList[i].dat == dat && strcmp(s2ch.favList[i].title, title) == 0)
                    {
                        j = 1;
                        break;
                    }
                }
            }
            // お気に入りにある
            if (j)
            {
                if (s2ch.tateFlag)
                {
                    menuStr = s2ch.menuResV.sub2;
                }
                else
                {
                    menuStr = s2ch.menuResH.sub2;
                }
            }
            // お気に入りにない
            else
            {
                if (s2ch.tateFlag)
                {
                    menuStr = s2ch.menuResV.sub1;
                }
                else
                {
                    menuStr = s2ch.menuResH.sub1;
                }
            }
        }
        else
        {
            if (s2ch.tateFlag)
            {
                menuStr = s2ch.menuResV.main;
            }
            else
            {
                menuStr = s2ch.menuResH.main;
            }
        }
        s2ch.viewY = s2ch.res.start * LINE_PITCH;
        psp2chDrawRes(s2ch.res.start);
        pgCopy(s2ch.viewX, s2ch.viewY);
        bar.start = s2ch.viewY;
        pgScrollbar(bar, s2ch.resBarColor);
        pgMenuBar(menuStr);
        if (rMenu)
        {
            pgTitleBar(title, s2ch.resList[0].title);
        }
        pgPadCursor(cursorX,cursorY);
        sceDisplayWaitVblankStart();
        framebuffer = sceGuSwapBuffers();
    }
    return ret;
}

/*****************************
アンカー情報をリセット
*****************************/
void psp2chResResetAnchors(void)
{
    int i;

    for (i = 0; i < 50; i++)
    {
        s2ch.resAnchor[i].x1 = 0;
        s2ch.resAnchor[i].x2 = 0;
        s2ch.urlAnchor[i].x1 = 0;
        s2ch.urlAnchor[i].x2 = 0;
    }
    for (i = 0; i < 40; i++)
    {
        s2ch.idAnchor[i].x1 = 0;
        s2ch.idAnchor[i].x2 = 0;
        s2ch.numAnchor[i].x1 = 0;
        s2ch.numAnchor[i].x2 = 0;
    }
}

/*****************************
上下左右キーでの移動
アナログパッドの移動も追加
*****************************/
int psp2chResCursorMove(int* totalLine, int* lineEnd, int* cursorY, int limitY)
{
    static int keyStart = 0, keyRepeat = 0, rMenu = 0;
    static clock_t keyTime = 0;
    int padUp = 0, padDown = 0;

    if (s2ch.tateFlag)
    {
        if (*cursorY == 0 && s2ch.pad.Lx == 255)
        {
            padUp = 1;
        }
        else if (*cursorY == limitY && s2ch.pad.Lx == 0)
        {
            padDown = 1;
        }
    }
    else
    {
        if (*cursorY == 0 && s2ch.pad.Ly == 0)
        {
            padUp = 1;
        }
        else if (*cursorY == limitY && s2ch.pad.Ly == 255)
        {
            padDown = 1;
        }
    }
    if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.btnResH.change) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.btnResV.change))
    {
        rMenu = 1;
    }
    else
    {
        rMenu = 0;
    }
    if (s2ch.pad.Buttons != s2ch.oldPad.Buttons || keyRepeat)
    {
        if (s2ch.pad.Buttons != s2ch.oldPad.Buttons)
        {
            keyStart = 1;
        }
        else
        {
            keyStart = 0;
        }
        keyTime = clock();
        keyRepeat = 0;
        if((s2ch.pad.Buttons & s2ch.btnResH.s.up && !s2ch.tateFlag) || (s2ch.pad.Buttons & s2ch.btnResV.s.up && s2ch.tateFlag) || padUp)
        {
            s2ch.res.start--;
            if (s2ch.res.start < 0)
            {
                s2ch.res.start = 0;
            }
        }
        if((s2ch.pad.Buttons & s2ch.btnResH.s.down && !s2ch.tateFlag) || (s2ch.pad.Buttons & s2ch.btnResV.s.down && s2ch.tateFlag) || padDown)
        {
            s2ch.res.start++;
            if (s2ch.res.start > *totalLine - *lineEnd)
            {
                s2ch.res.start = *totalLine - *lineEnd;
            }
            if (s2ch.res.start < 0)
            {
                s2ch.res.start = 0;
            }
        }
        if((s2ch.pad.Buttons & s2ch.btnResH.s.pUp && !s2ch.tateFlag) || (s2ch.pad.Buttons & s2ch.btnResV.s.pUp && s2ch.tateFlag))
        {
            s2ch.res.start -= (*lineEnd - 2);
            if (s2ch.res.start < 0)
            {
                s2ch.res.start = 0;
            }
        }
        if((s2ch.pad.Buttons & s2ch.btnResH.s.pDown && !s2ch.tateFlag) || (s2ch.pad.Buttons & s2ch.btnResV.s.pDown && s2ch.tateFlag))
        {
            s2ch.res.start += (*lineEnd - 2);
            if (s2ch.res.start > *totalLine - *lineEnd)
            {
                s2ch.res.start = *totalLine - *lineEnd;
            }
            if (s2ch.res.start < 0)
            {
                s2ch.res.start = 0;
            }
        }
        if((s2ch.pad.Buttons & s2ch.btnResH.s.top && !s2ch.tateFlag) || (s2ch.pad.Buttons & s2ch.btnResV.s.top && s2ch.tateFlag))
        {
            if (rMenu && !padUp)
            {
                s2ch.res.start = 0;
            }
        }
        if((s2ch.pad.Buttons & s2ch.btnResH.s.end && !s2ch.tateFlag) || (s2ch.pad.Buttons & s2ch.btnResV.s.end && s2ch.tateFlag))
        {
            if (rMenu && !padDown)
            {
                s2ch.res.start = *totalLine - *lineEnd;
            }
        }
    }
    else
    {
        if (keyStart)
        {
            if (clock() - keyTime > 400000)
            {
                keyRepeat = 1;
            }
        }
        else
        {
            if (clock() - keyTime > 10000)
            {
                keyRepeat = 1;
            }
        }
    }
    return rMenu;
}

/*****************************
レスの行数をセット
スクロールバーの構造体をセット
総行数を返す
*****************************/
int psp2chResSetLine(S_SCROLLBAR* bar)
{
    int i, j;

    if (s2ch.tateFlag)
    {
        bar->view = SCR_WIDTH - s2ch.font.height - s2ch.font.pitch;
        bar->x = RES_SCR_WIDTH_V;
        bar->y = 0;
        bar->w = RES_BAR_WIDTH_V;
        bar->h = bar->view;
        for (i = 0, j = 0; i < s2ch.res.count; i++)
        {
            s2ch.resList[i].line = psp2chCountRes(i, RES_SCR_WIDTH_V);
            if (s2ch.resList[i].ng == 0)
            {
                j += s2ch.resList[i].line;
                j++;
            }
        }
    }
    else
    {
        bar->view = SCR_HEIGHT - s2ch.font.height;
        bar->x = RES_SCR_WIDTH;
        bar->y = 0;
        bar->w = RES_BAR_WIDTH;
        bar->h = bar->view;
        for (i = 0, j = 0; i < s2ch.res.count; i++)
        {
            s2ch.resList[i].line = psp2chCountRes(i, RES_SCR_WIDTH);
            if (s2ch.resList[i].ng == 0)
            {
                j += s2ch.resList[i].line;
                j++;
            }
        }
    }
    bar->total = j * LINE_PITCH;
    return j;
}

/*****************************
アナログパッドを読み取ってカーソル座標を更新
*****************************/
void psp2chResPadMove(int* cursorX, int* cursorY, int limitX, int limitY)
{
    int padX, padY;
    int dL, dS;

    padX = s2ch.pad.Lx - 127;
    padY = s2ch.pad.Ly - 127;
    if (s2ch.cfg.padAccel)
    {
        if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.btnResH.change) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.btnResV.change))
        {
            dL = 5;
        }
        else
        {
            dL = 3;
        }
        if (s2ch.tateFlag)
        {
            if (padX < -s2ch.cfg.padCutoff)
            {
                padX = (1 - padX - s2ch.cfg.padCutoff) * 128 / (128 - s2ch.cfg.padCutoff);
                *cursorY += padX >> dL;
            }
            else if (padX > s2ch.cfg.padCutoff)
            {
                padX = (padX - s2ch.cfg.padCutoff) * 128 / (128 - s2ch.cfg.padCutoff);
                *cursorY -= padX >> dL;
            }
            if (padY < -s2ch.cfg.padCutoff)
            {
                padY = (1 - padY - s2ch.cfg.padCutoff) * 128 / (128 - s2ch.cfg.padCutoff);
                *cursorX -= padY >> dL;
            }
            else if (padY > s2ch.cfg.padCutoff)
            {
                padY = (padY - s2ch.cfg.padCutoff) * 128 / (128 - s2ch.cfg.padCutoff);
                *cursorX += padY >> dL;
            }
        }
        else
        {
            dL++;
            if (padX < -s2ch.cfg.padCutoff)
            {
                padX = (1 - padX - s2ch.cfg.padCutoff) * 128 / (128 - s2ch.cfg.padCutoff);
                *cursorX -= padX >> dL;
            }
            else if (padX > s2ch.cfg.padCutoff)
            {
                padX = (padX - s2ch.cfg.padCutoff) * 128 / (128 - s2ch.cfg.padCutoff);
                *cursorX += padX >> dL;
            }
            if (padY < -s2ch.cfg.padCutoff)
            {
                padY = (1 - padY - s2ch.cfg.padCutoff) * 128 / (128 - s2ch.cfg.padCutoff);
                *cursorY -= padY >> dL;
            }
            else if (padY > s2ch.cfg.padCutoff)
            {
                padY = (padY - s2ch.cfg.padCutoff) * 128 / (128 - s2ch.cfg.padCutoff);
                *cursorY += padY >> dL;
            }
        }
    }
    else
    {
        if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.btnResH.change) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.btnResV.change))
        {
            dL = 4;
            dS = 2;
        }
        else
        {
            dL = 16;
            dS = 2;
        }
        if (s2ch.tateFlag)
        {
            if (padX < -s2ch.cfg.padCutoff)
            {
                if (padX == -127)
                {
                    *cursorY += dL;
                }
                else
                {
                    *cursorY += dS;
                }
            }
            else if (padX > s2ch.cfg.padCutoff)
            {
                if (padX == 128)
                {
                    *cursorY -= dL;
                }
                else
                {
                    *cursorY -= dS;
                }
            }
            if (padY < -s2ch.cfg.padCutoff)
            {
                if (padY == -127)
                {
                    *cursorX -= dL;
                }
                else
                {
                    *cursorX -= dS;
                }
            }
            else if (padY > s2ch.cfg.padCutoff)
            {
                if (padY == 128)
                {
                    *cursorX += dL;
                }
                else
                {
                    *cursorX += dS;
                }
            }
        }
        else
        {
            dL >>= 1;
            dS >>= 1;
            if (padX < -s2ch.cfg.padCutoff)
            {
                if (padX == -127)
                {
                    *cursorX -= dL;
                }
                else
                {
                    *cursorX -= dS;
                }
            }
            else if (padX > s2ch.cfg.padCutoff)
            {
                if (padX == 128)
                {
                    *cursorX += dL;
                }
                else
                {
                    *cursorX += dS;
                }
            }
            if (padY < -s2ch.cfg.padCutoff)
            {
                if (padY == -127)
                {
                    *cursorY -= dL;
                }
                else
                {
                    *cursorY -= dS;
                }
            }
            else if (padY > s2ch.cfg.padCutoff)
            {
                if (padY == 128)
                {
                    *cursorY += dL;
                }
                else
                {
                    *cursorY += dS;
                }
            }
        }
    }
    if (*cursorX < 0)
    {
        *cursorX = 0;
    }
    else if (*cursorX > limitX)
    {
        *cursorX = limitX;
    }
    if (*cursorY < 0)
    {
        *cursorY = 0;
    }
    else if (*cursorY > limitY)
    {
        *cursorY = limitY;
    }
}

/*****************************
NGチェック
*****************************/
void psp2chResCheckNG(void)
{
    char* buf, *p, *q;
    int i;

    if (s2ch.resList == NULL)
    {
        return;
    }
    for (i = 0; i < s2ch.res.count; i++)
    {
        s2ch.resList[i].ng = 0;
    }
    buf = NULL;
    buf = psp2chGetNGBuf(ngNameFile, buf);
    if (buf)
    {
        p= buf;
        while (*p)
        {
            q = strchr(p, '\n');
            if (q == NULL)
            {
                break;
            }
            *q = '\0';
            for (i = 0; i < s2ch.res.count; i++)
            {
                if (strstr(s2ch.resList[i].name, p))
                {
                    s2ch.resList[i].ng = 1;
                }
            }
            p = q + 1;
        }
        free(buf);
    }
    buf = NULL;
    buf = psp2chGetNGBuf(ngIDFile, buf);
    if (buf)
    {
        p= buf;
        while (*p)
        {
            q = strchr(p, '\n');
            if (q == NULL)
            {
                break;
            }
            *q = '\0';
            for (i = 0; i < s2ch.res.count; i++)
            {
                if (s2ch.resList[i].id == NULL)
                {
                    continue;
                }
                if (strcmp(s2ch.resList[i].id, p) == 0)
                {
                    s2ch.resList[i].ng = 1;
                }
            }
            p = q + 1;
        }
        free(buf);
    }
    buf = NULL;
    buf = psp2chGetNGBuf(ngWordFile, buf);
    if (buf)
    {
        p= buf;
        while (*p)
        {
            q = strchr(p, '\n');
            if (q == NULL)
            {
                break;
            }
            *q = '\0';
            for (i = 0; i < s2ch.res.count; i++)
            {
                if (strstr(s2ch.resList[i].text, p))
                {
                    s2ch.resList[i].ng = 1;
                }
            }
            p = q + 1;
        }
        free(buf);
    }
    buf = NULL;
    buf = psp2chGetNGBuf(ngMailFile, buf);
    if (buf)
    {
        p= buf;
        while (*p)
        {
            q = strchr(p, '\n');
            if (q == NULL)
            {
                break;
            }
            *q = '\0';
            for (i = 0; i < s2ch.res.count; i++)
            {
                if (strstr(s2ch.resList[i].mail, p))
                {
                    s2ch.resList[i].ng = 1;
                }
            }
            p = q + 1;
        }
        free(buf);
    }
}

/*****************************
datファイルをメモリに読み込みデータの区切りを'\0'（文字列終端）に書き換える
s2ch.resList構造体のポインタに各データのアドレスを代入
*****************************/
int psp2chResList(char* host, char* dir, char* title, int dat)
{
    const char* delimiter = "<>";
    SceUID fd;
    SceIoStat st;
    char path[256];
    char *p, *q;
    char *buf;
    int ret;

    sprintf(path, "%s/%s/%s/%d.idx", s2ch.cwDir, s2ch.logDir, title, dat);
    fd = sceIoOpen(path, PSP_O_RDONLY, 0777);
    if (fd < 0)
    {
        s2ch.res.start = 0;
        s2ch.res.select = 0;
    }
    else
    {
        sceIoRead(fd, path, 128);
        sceIoClose(fd);
        p = strchr(path, '\n');
        p++;
        p =  strchr(p, '\n');
        p++;
        p =  strchr(p, '\n');
        p++;
        sscanf(p, "%d %d", &s2ch.res.start, &s2ch.res.select);
    }
    sprintf(path, "%s/%s/%s/%d.dat", s2ch.cwDir, s2ch.logDir, title, dat);
    ret = sceIoGetstat(path, &st);
    if (ret < 0)
    {
        s2ch.res.start = 0;
        s2ch.res.select = 0;
        ret = psp2chGetDat(host, dir, title, dat);
        if (ret < 0)
        {
            return ret;
        }
        ret = sceIoGetstat(path, &st);
        if (ret < 0)
        {
            psp2chErrorDialog("File stat error\n%s", path);
            return -1;
        }
    }
    free(resBuffer);
    resBuffer = (char*)malloc(st.st_size + 1);
    if (resBuffer == NULL)
    {
        psp2chErrorDialog("memorry error\nresBuffer");
        return -1;
    }
    fd = sceIoOpen(path, PSP_O_RDONLY, 0777);
    if (fd < 0)
    {
        psp2chErrorDialog("File open error\n%s", path);
        return -1;
    }
    ret = sceIoRead(fd, resBuffer, st.st_size);
    sceIoClose(fd);
    if (ret != st.st_size)
    {
        free(resBuffer);
        resBuffer = NULL;
        pgMenuBar("DATの読み込みに失敗しました");
        sceDisplayWaitVblankStart();
        framebuffer = sceGuSwapBuffers();
    }
    resBuffer[st.st_size] = '\0';
    s2ch.res.count = 0;
    p = resBuffer;
    while (*p)
    {
        if (*p++ == '\n')
        {
            s2ch.res.count++;
        }
    }
    free(s2ch.resList);
    s2ch.resList = (S_2CH_RES*)malloc(sizeof(S_2CH_RES) * s2ch.res.count);
    if (s2ch.resList == NULL)
    {
        free(resBuffer);
        resBuffer = NULL;
        psp2chErrorDialog("memorry error\nresList");
        return -1;
    }
    p = resBuffer;
    buf = resBuffer;
    for (ret = 0; ret < s2ch.res.count; ret++)
    {
        p =  strstr(buf, delimiter);
        if (p == NULL)
        {
            free(s2ch.resList);
            s2ch.resList = NULL;
            free(resBuffer);
            resBuffer = NULL;
            psp2chErrorDialog("DAT log error1\n%d/%d", ret, s2ch.res.count);
            return -1;
        }
        *p = '\0';
        s2ch.resList[ret].name = buf;
        p += 2;
        buf = p;
        p =  strstr(buf, delimiter);
        if (p == NULL)
        {
            free(s2ch.resList);
            s2ch.resList = NULL;
            free(resBuffer);
            resBuffer = NULL;
            psp2chErrorDialog("DAT log error2\n%d", ret);
            return -1;
        }
        *p = '\0';
        s2ch.resList[ret].mail = buf;
        p += 2;
        buf = p;
        p =  strstr(buf, delimiter);
        if (p == NULL)
        {
            free(s2ch.resList);
            s2ch.resList = NULL;
            free(resBuffer);
            resBuffer = NULL;
            psp2chErrorDialog("DAT log error3\n%d", ret);
            return -1;
        }
        *p = '\0';
        s2ch.resList[ret].date = buf;
        q =  strstr(buf, " BE:");
        if (q)
        {
            *q = '\0';
            q += 4;
            s2ch.resList[ret].be = q;
        }
        else
        {
            s2ch.resList[ret].be = NULL;
        }
        q =  strstr(buf, " ID:");
        if (q)
        {
            *q = '\0';
            q += 4;
            s2ch.resList[ret].id = q;
        }
        else
        {
            s2ch.resList[ret].id = NULL;
        }
        p += 2;
        buf = p;
        p =  strstr(buf, delimiter);
        if (p == NULL)
        {
            free(s2ch.resList);
            s2ch.resList = NULL;
            free(resBuffer);
            resBuffer = NULL;
            psp2chErrorDialog("DAT log error4\n%d", ret);
            return -1;
        }
        *p = '\0';
        s2ch.resList[ret].text = buf;
        p += 2;
        buf = p;
        p =  strstr(buf, "\n");
        if (p == NULL)
        {
            free(s2ch.resList);
            s2ch.resList = NULL;
            free(resBuffer);
            resBuffer = NULL;
            psp2chErrorDialog("DAT log error5\n%d", ret);
            return -1;
        }
        *p = '\0';
        s2ch.resList[ret].title = buf;
        p++;
        buf = p;
        s2ch.resList[ret].num = ret;
    }
    psp2chResCheckNG();
    psp2chSaveIdx(title, dat);
    return 0;
}

/*****************************
datファイルにアクセスして保存
戻り値 0:データ取得, 1:更新なし, <0:error
*****************************/
int psp2chGetDat(char* host, char* dir, char* title, int dat)
{
    int ret, range, len;
    S_NET net;
    SceUID fd;
    char path[256];
    char buf[256];
    char lastModified[32];
    char eTag[32];
    char *p, *q;

    // Make ita directory
    sprintf(path, "%s/%s/%s", s2ch.cwDir, s2ch.logDir, title);
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
    // check ita idx
    sprintf(path, "%s/%s/%s/%d.idx", s2ch.cwDir, s2ch.logDir, title, dat);
    fd = sceIoOpen(path, PSP_O_RDONLY, 0777);
    if (fd < 0)
    {
        buf[0] = '\0';
        range = 0;
        s2ch.res.start = 0;
        s2ch.res.select = 0;
    }
    else
    {
        sceIoRead(fd, buf, 128);
        sceIoClose(fd);
        p = strchr(buf, '\n');
        *p = '\0';
        strcpy(lastModified, buf);
        p++;
        q =  strchr(p, '\n');
        *q = 0;
        strcpy(eTag, p);
        q++;
        sscanf(q, "%d %d %d", &range, &s2ch.res.start, &s2ch.res.select);
        sprintf(buf, "If-Modified-Since: %s\r\nIf-None-Match: %s\r\nRange: bytes=%d-\r\n", lastModified, eTag, range - 1);
    }
    sprintf(path, "%s/dat/%d.dat", dir, dat);
    ret = psp2chGet(host, path, buf, &net);
    if (ret < 0)
    {
        return ret;
    }
    switch(net.status)
    {
        case 200: // OK
            p = net.body;
            len = net.length;
            break;
        case 206: // Partial content
            p = net.body + 1;
            len = net.length - 1;
            break;
        case 302: // Found
            /*
            psp2chErrorDialog(TEXT_10);
            */
            free(net.body);
            pgWaitVn(40);
            pgMenuBar("このスレはDAT落ちしたようです");
            sceDisplayWaitVblankStart();
            framebuffer = sceGuSwapBuffers();
            pgWaitVn(60);
            return 1;
            break;
        case 304: // Not modified
            free(net.body);
            return 1;
        default:
            free(net.body);
            psp2chErrorDialog("HTTP error\nhost %s path %s\nStatus code %d", host, path, ret);
            return -1;
    }
    // abone check
    if (range && (net.body[0] !='\n'))
    {
        free(net.body);
        psp2chErrorDialog(TEXT_4);
        return -1;
    }
    // save dat.dat
    sprintf(path, "%s/%s/%s/%d.dat", s2ch.cwDir, s2ch.logDir, title, dat);
    fd = sceIoOpen(path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_APPEND, 0777);
    if (fd < 0)
    {
        free(net.body);
        psp2chErrorDialog("File open error\n%s", path);
        return fd;
    }
    sceIoWrite(fd, p, len);
    range += len;
    free(net.body);
    sceIoClose(fd);
    // save dat.idx
    sprintf(path, "%s/%s/%s/%d.idx", s2ch.cwDir, s2ch.logDir, title, dat);
    fd = sceIoOpen(path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
    if (fd < 0)
    {
        psp2chErrorDialog("File open error\n%s", path);
        return fd;
    }
    sceIoWrite(fd, net.head.Last_Modified, strlen(net.head.Last_Modified));
    sceIoWrite(fd, net.head.ETag, strlen(net.head.ETag));
    sprintf(buf, "%d\n%d\n%d\n", range,s2ch.res.start, s2ch.res.select);
    sceIoWrite(fd, buf, strlen(buf));
    sceIoClose(fd);
    return 0;
}

/*****************************
スレッドの情報ファイル保存
ファイル名：dat番号.idx
Last-Modified\n
ETag\n
range（現在取得したバイト数）\n
スレッド表示行\n
未使用\n（最初は表示レスの行に使っていた）
取得レス数\n
*****************************/
void psp2chSaveIdx(char* title, int dat)
{
    SceUID fd;
    char path[256];
    char buf[256];
    char lastModified[32];
    char eTag[32];
    char *p, *q;
    int range, r, l;

    sprintf(path, "%s/%s/%s/%d.idx", s2ch.cwDir, s2ch.logDir, title, dat);
    fd = sceIoOpen(path, PSP_O_RDONLY, 0777);
    if (fd < 0)
    {
        return;
    }
    else
    {
        sceIoRead(fd, buf, 128);
        sceIoClose(fd);
        p = strchr(buf, '\n');
        *p = '\0';
        strcpy(lastModified, buf);
        p++;
        q =  strchr(p, '\n');
        *q = 0;
        strcpy(eTag, p);
        q++;
        sscanf(q, "%d %d %d", &range, &r, &l);
    }
    fd = sceIoOpen(path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
    if (fd < 0)
    {
        return;
    }
    else
    {
        sprintf(buf, "%s\n%s\n%d\n%d\n%d\n%d\n", lastModified, eTag, range, s2ch.res.start, s2ch.res.select, s2ch.res.count);
        sceIoWrite(fd, buf, strlen(buf));
        sceIoClose(fd);
    }
    if (s2ch.threadList)
    {
        s2ch.threadList[threadSort[s2ch.thread.select]].old = s2ch.res.count;
    }
}

int psp2chDrawResStr(char* str, S_2CH_RES_COLOR c, int line, int lineEnd, int startX, int endX, int *drawLine)
{
    while ((str = pgPrintHtml(str, c, startX, endX, *drawLine)))
    {
        s2ch.pgCursorX = startX;
        s2ch.pgCursorY += LINE_PITCH;
        (*drawLine)++;
        if (++line > lineEnd)
        {
            break;
        }
        pgFillvram(c.bg, startX, s2ch.pgCursorY, endX-startX, LINE_PITCH);
    }
    return line;
}

/*****************************
レスのヘッダ部分の表示（名前、日付、ID等）
*****************************/
// ID 出現回数で色を変える
#define ID_COUNT 5
int psp2chDrawResHeader(int re, int* skip, int line, int lineEnd, int startX, int endX,S_2CH_RES_COLOR c,S_2CH_HEADER_COLOR hc, int* drawLine)
{
    char* str;
    char buf[128];
    int i, j;

    sprintf(buf, " %d ", s2ch.resList[re].num + 1);
    str = buf;
    c.text = hc.num;
    if (*skip)
    {
        while ((str = pgCountHtml(str, endX, 1)))
        {
            s2ch.pgCursorX = startX;
            if (--(*skip) == 0)
            {
                s2ch.numAnchor[s2ch.numAnchorCount].x1 = s2ch.pgCursorX;
                s2ch.numAnchor[s2ch.numAnchorCount].line = *drawLine;
                s2ch.numAnchor[s2ch.numAnchorCount].num = s2ch.resList[re].num;
                pgFillvram(c.bg, startX, s2ch.pgCursorY, endX-startX, LINE_PITCH);
                line = psp2chDrawResStr(str, c, line, lineEnd, startX, endX, drawLine);
                break;
            }
        }
    }
    else
    {
        s2ch.numAnchor[s2ch.numAnchorCount].x1 = s2ch.pgCursorX;
        s2ch.numAnchor[s2ch.numAnchorCount].line = *drawLine;
        s2ch.numAnchor[s2ch.numAnchorCount].num = s2ch.resList[re].num;
        pgFillvram(c.bg, startX, s2ch.pgCursorY, endX-startX, LINE_PITCH);
        line = psp2chDrawResStr(str, c, line, lineEnd, startX, endX, drawLine);
    }
    s2ch.numAnchor[s2ch.numAnchorCount].x2 = s2ch.pgCursorX;
    s2ch.numAnchorCount++;
    if (s2ch.numAnchorCount >= 40)
    {
        s2ch.numAnchorCount = 0;
    }
    sprintf(buf, "名前:");
    str = buf;
    c.text = hc.name1;
    if (*skip)
    {
        while ((str = pgCountHtml(str, endX, 1)))
        {
            s2ch.pgCursorX = startX;
            if (--(*skip) == 0)
            {
                pgFillvram(c.bg, startX, s2ch.pgCursorY, endX-startX, LINE_PITCH);
                line = psp2chDrawResStr(str, c, line, lineEnd, startX, endX, drawLine);
                break;
            }
        }
    }
    else
    {
        line = psp2chDrawResStr(str, c, line, lineEnd, startX, endX, drawLine);
    }
    if (line > lineEnd)
    {
        return line;
    }
    str = s2ch.resList[re].name;
    c.text = hc.name2;
    if (*skip)
    {
        while ((str = pgCountHtml(str, endX, 1)))
        {
            s2ch.pgCursorX = startX;
            if (--(*skip) == 0)
            {
                pgFillvram(c.bg, startX, s2ch.pgCursorY, endX-startX, LINE_PITCH);
                line = psp2chDrawResStr(str, c, line, lineEnd, startX, endX, drawLine);
                break;
            }
        }
    }
    else
    {
        line = psp2chDrawResStr(str, c, line, lineEnd, startX, endX, drawLine);
    }
    if (line > lineEnd)
    {
        return line;
    }
    sprintf(buf, "[%s] ", s2ch.resList[re].mail);
    str = buf;
    c.text = hc.mail;
    if (*skip)
    {
        while ((str = pgCountHtml(str, endX, 1)))
        {
            s2ch.pgCursorX = startX;
            if (--(*skip) == 0)
            {
                pgFillvram(c.bg, startX, s2ch.pgCursorY, endX-startX, LINE_PITCH);
                line = psp2chDrawResStr(str, c, line, lineEnd, startX, endX, drawLine);
                break;
            }
        }
    }
    else
    {
        line = psp2chDrawResStr(str, c, line, lineEnd, startX, endX, drawLine);
    }
    if (line > lineEnd)
    {
        return line;
    }
    str = s2ch.resList[re].date;
    c.text = hc.date;
    if (*skip)
    {
        while ((str = pgCountHtml(str, endX, 1)))
        {
            s2ch.pgCursorX = startX;
            if (--(*skip) == 0)
            {
                pgFillvram(c.bg, startX, s2ch.pgCursorY, endX-startX, LINE_PITCH);
                line = psp2chDrawResStr(str, c, line, lineEnd, startX, endX, drawLine);
                break;
            }
        }
    }
    else
    {
        line = psp2chDrawResStr(str, c, line, lineEnd, startX, endX, drawLine);
    }
    if (line > lineEnd)
    {
        return line;
    }
    if (s2ch.resList[re].id)
    {
        sprintf(buf, " ID:");
        str = buf;
        c.text = hc.id1;
        for (i = 0, j = 0; i < s2ch.res.count; i++)
        {
            if (s2ch.resList[i].id && s2ch.resList[i].id[0] != '?' && (strcmp(s2ch.resList[i].id, s2ch.resList[re].id) == 0))
            {
                if (++j == ID_COUNT)
                {
                    c.text = hc.id2;
                    break;
                }
            }
        }
        if (*skip)
        {
            while ((str = pgCountHtml(str, endX, 1)))
            {
                s2ch.pgCursorX = startX;
                if (--(*skip) == 0)
                {
                    s2ch.idAnchor[s2ch.idAnchorCount].x1 = s2ch.pgCursorX;
                    s2ch.idAnchor[s2ch.idAnchorCount].line = *drawLine;
                    strcpy(s2ch.idAnchor[s2ch.idAnchorCount].id, s2ch.resList[re].id);
                    pgFillvram(c.bg, startX, s2ch.pgCursorY, endX-startX, LINE_PITCH);
                    line = psp2chDrawResStr(str, c, line, lineEnd, startX, endX, drawLine);
                    break;
                }
            }
        }
        else
        {
            s2ch.idAnchor[s2ch.idAnchorCount].x1 = s2ch.pgCursorX;
            s2ch.idAnchor[s2ch.idAnchorCount].line = *drawLine;
            strcpy(s2ch.idAnchor[s2ch.idAnchorCount].id, s2ch.resList[re].id);
            line = psp2chDrawResStr(str, c, line, lineEnd, startX, endX, drawLine);
        }
        if (line > lineEnd)
        {
            s2ch.idAnchor[s2ch.idAnchorCount].x2 = endX;
            return line;
        }
        str = s2ch.resList[re].id;
        c.text = hc.id3;
        if (*skip)
        {
            while ((str = pgCountHtml(str, endX, 1)))
            {
                s2ch.pgCursorX = startX;
                if (--(*skip) == 0)
                {
                    pgFillvram(c.bg, startX, s2ch.pgCursorY, endX-startX, LINE_PITCH);
                    line = psp2chDrawResStr(str, c, line, lineEnd, startX, endX, drawLine);
                    line++;
                    (*drawLine)++;
                    s2ch.pgCursorX = startX;
                    s2ch.pgCursorY += LINE_PITCH;
                    return line;
                }
            }
            s2ch.pgCursorX = startX;
            (*skip)--;
        }
        else
        {
            line = psp2chDrawResStr(str, c, line, lineEnd, startX, endX, drawLine);
            if (*drawLine == s2ch.idAnchor[s2ch.idAnchorCount].line)
            {
                s2ch.idAnchor[s2ch.idAnchorCount].x2 = s2ch.pgCursorX;
            }
            else
            {
                s2ch.idAnchor[s2ch.idAnchorCount].x2 = endX;
                s2ch.idAnchorCount++;
                if (s2ch.idAnchorCount >= 40)
                {
                    s2ch.idAnchorCount = 0;
                }
                s2ch.idAnchor[s2ch.idAnchorCount].x1 = startX;
                s2ch.idAnchor[s2ch.idAnchorCount].line = *drawLine;
                strcpy(s2ch.idAnchor[s2ch.idAnchorCount].id, s2ch.resList[re].id);
                s2ch.idAnchor[s2ch.idAnchorCount].x2 = s2ch.pgCursorX;
            }
            s2ch.idAnchorCount++;
            if (s2ch.idAnchorCount >= 40)
            {
                s2ch.idAnchorCount = 0;
            }
            line++;
            (*drawLine)++;
            s2ch.pgCursorX = startX;
            s2ch.pgCursorY += LINE_PITCH;
        }
    }
    else
    {
        if (*skip)
        {
            s2ch.pgCursorX = startX;
            (*skip)--;
        }
        else
        {
            line++;
            (*drawLine)++;
            s2ch.pgCursorX = startX;
            s2ch.pgCursorY += LINE_PITCH;
        }
    }
    return line;
}

/*****************************
レスの本文部分の表示
*****************************/
int psp2chDrawResText(int res, int* skip, int line, int lineEnd, int startX, int endX, S_2CH_RES_COLOR c, int* drawLine)
{
    char* str;

    str = s2ch.resList[res].text;
    if (*skip)
    {
        while (str)
        {
            str = pgCountHtml(str, endX, 1);
            s2ch.pgCursorX = startX;
            if (--(*skip) == 0)
            {
                while (str)
                {
                    pgFillvram(c.bg, startX, s2ch.pgCursorY, endX-startX, LINE_PITCH);
                    str = pgPrintHtml(str, c, startX, endX, *drawLine);
                    (*drawLine)++;
                    if (++line > lineEnd)
                    {
                        return line;
                    }
                    s2ch.pgCursorX = startX;
                    s2ch.pgCursorY += LINE_PITCH;
                }
            }
        }
        pgFillvram(c.bg, startX, s2ch.pgCursorY, endX-startX, LINE_PITCH);
    }
    else
    {
        while (str)
        {
            pgFillvram(c.bg, startX, s2ch.pgCursorY, endX-startX, LINE_PITCH);
            str = pgPrintHtml(str, c, startX, endX, *drawLine);
            (*drawLine)++;
            if (++line > lineEnd)
            {
                return line;
            }
            s2ch.pgCursorX = startX;
            s2ch.pgCursorY += LINE_PITCH;
        }
        pgFillvram(c.bg, startX, s2ch.pgCursorY, endX-startX, LINE_PITCH);
    }
    s2ch.pgCursorX = startX;
    s2ch.pgCursorY += LINE_PITCH ;
    line++;
    (*drawLine)++;
    return line;
}

/*****************************
レスの表示
drawLine:画面一番上に表示する行
*****************************/
void psp2chDrawRes(int drawLine)
{
    int re;
    int skip;
    int line = 0;
    int i;
    int endX, lineEnd;

    if (s2ch.tateFlag)
    {
        endX = RES_SCR_WIDTH_V;
        lineEnd = DRAW_LINE_V;
    }
    else
    {
        endX = RES_SCR_WIDTH;
        lineEnd = DRAW_LINE_H;
    }
    // 表示行に変化なし
    if (drawLine == preLine)
    {
        return;
    }
    // 1行下に移動
    else if (drawLine == preLine+1)
    {
        preLine = drawLine;
        i = 0;
        drawLine += lineEnd-1;
        skip = drawLine;
        while (skip >= 0)
        {
            if (s2ch.resList[i].ng == 0)
            {
                skip -= s2ch.resList[i].line;
                skip--;
            }
            i++;
            if (i >= s2ch.res.count)
            {
                break;
            }
        }
        re = --i;
        skip++;
        skip += s2ch.resList[i].line;
        s2ch.pgCursorX = 0;
        s2ch.pgCursorY = (LINE_PITCH*drawLine)&0x01FF;
        //pspDebugScreenInit();
        //pspDebugScreenPrintf("Y=%d\n", (int)pgCursorY);
        //pgWaitVn(200);
        line = psp2chDrawResHeader(re, &skip, lineEnd, lineEnd, 0, endX, s2ch.resColor, s2ch.resHeaderColor, &drawLine);
        if (line > lineEnd)
        {
            pgFillvram(s2ch.resColor.bg, 0, s2ch.pgCursorY, endX, LINE_PITCH);
            return;
        }
        line = psp2chDrawResText(re, &skip, lineEnd, lineEnd, 0, endX, s2ch.resColor, &drawLine);
        pgFillvram(s2ch.resColor.bg, 0, s2ch.pgCursorY+LINE_PITCH, endX, LINE_PITCH);
    }
    // 1行上に移動
    else if (drawLine == preLine-1)
    {
        preLine = drawLine;
        skip = drawLine;
        i = 0;
        while (skip >= 0)
        {
            if (s2ch.resList[i].ng == 0)
            {
                skip -= s2ch.resList[i].line;
                skip--;
            }
            i++;
            if (i >= s2ch.res.count)
            {
                break;
            }
        }
        re = --i;
        skip++;
        skip += s2ch.resList[i].line;
        s2ch.pgCursorX = 0;
        s2ch.pgCursorY = (LINE_PITCH*drawLine)&0x01FF;
        line = psp2chDrawResHeader(re, &skip, lineEnd, lineEnd, 0, endX, s2ch.resColor, s2ch.resHeaderColor, &drawLine);
        if (line > lineEnd)
        {
            return;
        }
        line = psp2chDrawResText(re, &skip, lineEnd, lineEnd, 0, endX, s2ch.resColor, &drawLine);
    }
    // 全画面書き直し
    else
    {
        preLine = drawLine;
        skip = drawLine;
        i = 0;
        while (skip >= 0)
        {
            if (s2ch.resList[i].ng == 0)
            {
                skip -= s2ch.resList[i].line;
                skip--;
            }
            i++;
            if (i >= s2ch.res.count)
            {
                break;
            }
        }
        re = --i;
        skip++;
        skip += s2ch.resList[i].line;
        s2ch.pgCursorX = 0;
        s2ch.pgCursorY = (LINE_PITCH*drawLine)&0x01FF;
        s2ch.resAnchorCount = 0;
        s2ch.resAnchor[0].x1 = 0;
        psp2chResResetAnchors();
        line = 0;
        while (line <= lineEnd)
        {
            line = psp2chDrawResHeader(re, &skip, line, lineEnd, 0, endX, s2ch.resColor, s2ch.resHeaderColor, &drawLine);
            if (line > lineEnd)
            {
                break;
            }
            line = psp2chDrawResText(re, &skip, line, lineEnd, 0, endX, s2ch.resColor, &drawLine);
            while (++re < s2ch.res.count && s2ch.resList[re].ng)
            {
                // NG レスをスキップ
            }
            if (re >= s2ch.res.count)
            {
                pgFillvram(s2ch.resColor.bg, 0, s2ch.pgCursorY, endX, (lineEnd + 1 - line)*LINE_PITCH);
                break;
            }
        }
    }
}

/*****************************
res:レス番号
width:レス表示画面の幅
戻り値:レスを画面幅で表示したときの行数
*****************************/
int psp2chCountRes(int res, int width)
{
    char* str;
    char buf[128];
    int count = 0;

    // NG 対象スレは除外
    if (s2ch.resList[res].ng)
    {
        return 0;
    }
    s2ch.pgCursorX = 0;
    s2ch.pgCursorY = 0;
    sprintf(buf, " %d 名前:", res + 1);
    str = buf;
    while ((str = pgCountHtml(str, width, 1)))
    {
        s2ch.pgCursorX = 0;
        count++;
    }
    str = s2ch.resList[res].name;
    while ((str = pgCountHtml(str, width, 1)))
    {
        s2ch.pgCursorX = 0;
        count++;
    }
    sprintf(buf, "[%s] ", s2ch.resList[res].mail);
    str = buf;
    while ((str = pgCountHtml(str, width, 1)))
    {
        s2ch.pgCursorX = 0;
        count++;
    }
    str = s2ch.resList[res].date;
    while ((str = pgCountHtml(str, width, 1)))
    {
        s2ch.pgCursorX = 0;
        count++;
    }
    if (s2ch.resList[res].id)
    {
        sprintf(buf, " ID:");
        str = buf;
        while ((str = pgCountHtml(str, width, 1)))
        {
            s2ch.pgCursorX = 0;
            count++;
        }
        str = s2ch.resList[res].id;
        while ((str = pgCountHtml(str, width, 1)))
        {
            s2ch.pgCursorX = 0;
            count++;
        }
    }
    s2ch.pgCursorX = 0;
    count++;
    str = s2ch.resList[res].text;
    while ((str = pgCountHtml(str, width, 1)))
    {
        s2ch.pgCursorX = 0;
        count++;
    }
    s2ch.pgCursorX = 0;
    count++;
    return count;
}
