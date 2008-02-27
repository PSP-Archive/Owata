/*
* $Id$
*/

#include <stdio.h>
#include <malloc.h>
#include <time.h>
#include <pspdebug.h>
#include "pg.h"
#include "psp2ch.h"
#include "psp2chIta.h"
#include "psp2chThread.h"
#include "psp2chRes.h"
#include "psp2chResWindow.h"
#include "psp2chFavorite.h"
#include "psp2chForm.h"
#include "psp2chMenu.h"
#include "utf8.h"

extern S_2CH s2ch; // psp2ch.c
extern int* threadSort; // psp2chThread.c
extern const char* ngNameFile;
extern const char* ngIDFile;

int preLine = -2;
char* resBuffer = NULL;

static char jmpHost[32], jmpDir[32], jmpTitle[32];
static int jmpDat;

/**************
  レス表示
***************/
int psp2chFavoriteRes(int ret)
{
    return psp2chRes(s2ch.favList[s2ch.fav.select].host, s2ch.favList[s2ch.fav.select].dir, s2ch.favList[s2ch.fav.select].title, s2ch.favList[s2ch.fav.select].dat, ret);
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
    static char* menuStr = "";
    int i, j, tmp;
    static int resMenu = -1, urlMenu = -1, idMenu = -1, numMenu = -1;
    int lineEnd, rMenu;

    if (s2ch.tateFlag)
    {
        lineEnd = 35;
    }
    else
    {
        lineEnd = 20;
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
                s2ch.tateFlag = (s2ch.tateFlag) ? 0 : 1;
                totalLine = psp2chResSetLine(&bar);
                if (s2ch.res.start > totalLine - lineEnd)
                {
                    s2ch.res.start = totalLine - lineEnd;
                }
                if (s2ch.res.start < 0)
                {
                    s2ch.res.start = 0;
                }
                psp2chResResetAnchors();
            }
            // STARTボタン
            else if(s2ch.pad.Buttons & PSP_CTRL_START)
            {
                psp2chMenu(0, s2ch.res.start*LINE_PITCH);
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
            // ○ボタン
            else if((!s2ch.tateFlag && s2ch.pad.Buttons & PSP_CTRL_CIRCLE) || (s2ch.tateFlag && s2ch.pad.Buttons & PSP_CTRL_LTRIGGER))
            {
                /* レスアンカー表示 */
                if (resMenu >= 0)
                {
                    psp2chResAnchor(resMenu);
                }
                /* URLアンカー処理 */
                else if (urlMenu >= 0)
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
                /* ID抽出表示 */
                else if (idMenu >= 0)
                {
                    psp2chIdAnchor(idMenu);
                }
                /* お気に入りに追加 */
                else if (rMenu)
                {
                    psp2chAddFavorite(host, dir, title, dat);
                }
                /* 投稿フォーム */
                else
                {
                    tmp = s2ch.tateFlag;
                    s2ch.tateFlag = 0;
                    if (message == NULL)
                    {
                        message = (char*)calloc(1, 2048);
                    }
                    if (message == NULL)
                    {
                        memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
                        sprintf(s2ch.mh.message, "memorry error\nForm message");
                        pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
                        sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
                        return 0;
                    }
                    if (numMenu >= 0 && message[0] == '\0')
                    {
                        sprintf(message, ">>%d\n", s2ch.numAnchor[numMenu].num + 1);
                    }
                    if (psp2chForm(host, dir, dat, s2ch.resList[0].title, message) == 1)
                    {
                        free(message);
                        message = NULL;
                        psp2chSaveIdx(title, dat);
                        psp2chGetDat(host, dir, title, dat);
                        psp2chResList(host, dir, title, dat);
                        s2ch.res.start++;
                        totalLine = psp2chResSetLine(&bar);
                    }
                    s2ch.tateFlag = tmp;
                }
                preLine = -2;
            }
            // ×ボタン
            else if(s2ch.pad.Buttons & PSP_CTRL_CROSS)
            {
                if (rMenu)
                {
                }
                else
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
            // △ボタン
            else if(s2ch.pad.Buttons & PSP_CTRL_TRIANGLE)
            {
                if (resMenu >= 0)
                {
                    if (s2ch.resList[s2ch.resAnchor[resMenu].res[0]].ng == 0)
                    {
                        for (i = 0, j = 0; i < s2ch.resAnchor[resMenu].res[0]; i++)
                        {
                            j += s2ch.resList[i].line;
                            j++;
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
                else
                {
                    psp2chSaveIdx(title, dat);
                    psp2chGetDat(host, dir, title, dat);
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
            // □ボタン
            else if(s2ch.pad.Buttons & PSP_CTRL_SQUARE)
            {
                if (rMenu)
                {
                    psp2chDelFavorite(title, dat);
                }
                else
                {
                    if (idMenu >= 0)
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
                    else
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
                menuStr = "　L : レス表\示　　　△ : レスに移動";
            }
            else
            {
                menuStr = "　○ : レス表\示　　　△ : レスに移動";
            }
        }
        else if (urlMenu >= 0)
        {
            if (s2ch.tateFlag)
            {
                menuStr = "　L : リンク表\示　　　";
            }
            else
            {
                menuStr = "　○ : リンク表\示　　　";
            }
        }
        else if (idMenu >= 0)
        {
            if (s2ch.tateFlag)
            {
                menuStr = "　L : ID抽出　　　□ : NGID登録";
            }
            else
            {
                menuStr = "　○ : ID抽出　　　□ : NGID登録";
            }
        }
        else if (numMenu >= 0)
        {
            if (s2ch.tateFlag)
            {
                menuStr = "　L : レスをする";
            }
            else
            {
                menuStr = "　○ : レスをする";
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
                menuStr = "　↑ : 先頭　　　↓ : 最後　　　□ : お気に入りから削除";
            }
            // お気に入りにない
            else
            {
                menuStr = "　↑ : 先頭　　　↓ : 最後　　　○ : お気に入りに登録";
            }
        }
        else
        {
            if (s2ch.tateFlag)
            {
                menuStr = "　L : 書き込み　　　× : 戻る　　　　　△ : 更新　　　　　□ : 削除　　　R : メニュー切替";
            }
            else
            {
                menuStr = "　○ : 書き込み　　　× : 戻る　　　　　△ : 更新　　　　　□ : 削除　　　R : メニュー切替";
            }
        }
        psp2chDrawRes(s2ch.res.start);
        pgCopy(0, s2ch.res.start*LINE_PITCH);
        bar.start = s2ch.res.start * LINE_PITCH;
        pgScrollbar(bar, s2ch.resBarColor);
        pgMenuBar(menuStr);
        pgPadCursor(cursorX,cursorY);
        sceDisplayWaitVblankStart();
        framebuffer = sceGuSwapBuffers();
    }
    return ret;
}

/*****************************
上下左右キーでの移動
アナログパッドの移動も追加
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
    if(s2ch.pad.Buttons & PSP_CTRL_RTRIGGER)
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
        if((s2ch.pad.Buttons & PSP_CTRL_UP && !s2ch.tateFlag) || (s2ch.pad.Buttons & PSP_CTRL_RIGHT && s2ch.tateFlag) || padUp)
        {
            if (rMenu && !padUp)
            {
                s2ch.res.start = 0;
            }
            else
            {
                s2ch.res.start--;
                if (s2ch.res.start < 0)
                {
                    s2ch.res.start = 0;
                }
            }
        }
        if((s2ch.pad.Buttons & PSP_CTRL_DOWN && !s2ch.tateFlag) || (s2ch.pad.Buttons & PSP_CTRL_LEFT && s2ch.tateFlag) || padDown)
        {
            if (rMenu && !padDown)
            {
                s2ch.res.start = *totalLine - *lineEnd;
            }
            else
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
        }
        if((s2ch.pad.Buttons & PSP_CTRL_LEFT && !s2ch.tateFlag) || (s2ch.pad.Buttons & PSP_CTRL_UP && s2ch.tateFlag))
        {
            s2ch.res.start -= (*lineEnd - 2);
            if (s2ch.res.start < 0)
            {
                s2ch.res.start = 0;
            }
        }
        if((s2ch.pad.Buttons & PSP_CTRL_RIGHT && !s2ch.tateFlag) || (s2ch.pad.Buttons & PSP_CTRL_DOWN && s2ch.tateFlag))
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
        bar->view = 455;
        bar->x = RES_SCR_WIDTH_V;
        bar->y = 0;
        bar->w = RES_BAR_WIDTH_V;
        bar->h = 455;
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
        bar->view = 259;
        bar->x = RES_SCR_WIDTH;
        bar->y = 0;
        bar->w = RES_BAR_WIDTH;
        bar->h = 259;
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
    if(s2ch.pad.Buttons & PSP_CTRL_RTRIGGER)
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
        if (padX < -PAD_CUTOFF)
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
        else if (padX > PAD_CUTOFF)
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
        if (padY < -PAD_CUTOFF)
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
        else if (padY > PAD_CUTOFF)
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
        if (padX < -PAD_CUTOFF)
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
        else if (padX > PAD_CUTOFF)
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
        if (padY < -PAD_CUTOFF)
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
        else if (padY > PAD_CUTOFF)
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
                if (strcmp(s2ch.resList[i].id, p) == 0)
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
            memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
            sprintf(s2ch.mh.message, "File stat error\n%s", path);
            pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
            sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
            return -1;
        }
    }
    free(resBuffer);
    resBuffer = (char*)malloc(st.st_size + 1);
    if (resBuffer == NULL)
    {
        memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
        strcpy(s2ch.mh.message, "memorry error\nresBuffer");
        pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
        sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
        return -1;
    }
    fd = sceIoOpen(path, PSP_O_RDONLY, 0777);
    if (fd < 0)
    {
        memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
        sprintf(s2ch.mh.message, "File open error\n%s", path);
        pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
        sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
        return -1;
    }
    sceIoRead(fd, resBuffer, st.st_size);
    sceIoClose(fd);
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
        memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
        strcpy(s2ch.mh.message, "memorry error\nresList");
        pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
        sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
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
            memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
            sprintf(s2ch.mh.message, "DAT log error1\n%d/%d", ret, s2ch.res.count);
            pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
            sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
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
            memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
            sprintf(s2ch.mh.message, "DAT log error2\n%d", ret);
            pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
            sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
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
            memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
            sprintf(s2ch.mh.message, "DAT log error3\n%d", ret);
            pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
            sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
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
            memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
            sprintf(s2ch.mh.message, "DAT log error4\n%d", ret);
            pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
            sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
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
            memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
            sprintf(s2ch.mh.message, "DAT log error5\n%d", ret);
            pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
            sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
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
*****************************/
int psp2chGetDat(char* host, char* dir, char* title, int dat)
{
    int ret, mySocket, contentLength, range;
    HTTP_HEADERS resHeader;
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
    mySocket = psp2chRequest(host, path, buf);
    if (mySocket < 0)
    {
        return mySocket;
    }
    ret = psp2chGetStatusLine(mySocket);
    switch(ret)
    {
        case 200: // OK
            break;
        case 206: // Partial content
            break;
        case 302: // Found
            /*
            memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
            strcpy(s2ch.mh.message, TEXT_10);
            pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
            sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
            */
            psp2chCloseSocket(mySocket);
            pgWaitVn(40);
            pgMenuBar("このスレはDAT落ちしたようです");
            sceDisplayWaitVblankStart();
            framebuffer = sceGuSwapBuffers();
            pgWaitVn(60);
            return -1;
            break;
        case 304: // Not modified
            psp2chCloseSocket(mySocket);
            return 0;
        default:
            memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
            sprintf(s2ch.mh.message, "HTTP error\nhost %s path %s\nStatus code %d", host, path, ret);
            pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
            psp2chCloseSocket(mySocket);
            sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
            return -1;
    }
    // Receive and Save dat
    contentLength = psp2chGetHttpHeaders(mySocket, &resHeader);
    if (contentLength <= 0)
    {
        psp2chCloseSocket(mySocket);
        return -1;
    }
    // save dat.dat
    sprintf(path, "%s/%s/%s/%d.dat", s2ch.cwDir, s2ch.logDir, title, dat);
    fd = sceIoOpen(path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_APPEND, 0777);
    if (fd < 0)
    {
        psp2chCloseSocket(mySocket);
        memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
        sprintf(s2ch.mh.message, "File open error\n%s", path);
        pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
        sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
        return fd;
    }
    sprintf(buf, "http://%s/%s/dat/%d.dat からデータを転送しています...", host, dir, dat);
    pgMenuBar(buf);
    sceDisplayWaitVblankStart();
    framebuffer = sceGuSwapBuffers();
    if (range && (recv(mySocket, buf, 1, 0) <= 0 || buf[0] !='\n'))
    {
        psp2chCloseSocket(mySocket);
        memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
        strcpy(s2ch.mh.message, TEXT_4);
        pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
        sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
        return -1;
    }
    while((ret = recv(mySocket, buf, sizeof(buf), 0)) > 0)
    {
        sceIoWrite(fd, buf, ret);
        range += ret;
    }
    psp2chCloseSocket(mySocket);
    sceIoClose(fd);
    // save dat.idx
    sprintf(path, "%s/%s/%s/%d.idx", s2ch.cwDir, s2ch.logDir, title, dat);
    fd = sceIoOpen(path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
    if (fd < 0)
    {
        psp2chCloseSocket(mySocket);
        memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
        sprintf(s2ch.mh.message, "File open error\n%s", path);
        pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
        sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
        return fd;
    }
    sceIoWrite(fd, resHeader.Last_Modified, strlen(resHeader.Last_Modified));
    sceIoWrite(fd, resHeader.ETag, strlen(resHeader.ETag));
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
        lineEnd = 35;
    }
    else
    {
        endX = RES_SCR_WIDTH;
        lineEnd = 20;
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
            return;
        }
        line = psp2chDrawResText(re, &skip, lineEnd, lineEnd, 0, endX, s2ch.resColor, &drawLine);
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
