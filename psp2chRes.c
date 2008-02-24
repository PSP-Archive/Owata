
#include "pspdialogs.h"
#include <stdio.h>
#include <malloc.h>
#include <time.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include "pg.h"
#include "psp2chIta.h"
#include "psp2chThread.h"
#include "psp2chRes.h"
#include "psp2chFavorite.h"
#include "psp2chImageView.h"
#include "psp2chForm.h"
#include "utf8.h"

extern int running; //main.c
extern char cwDir[256]; //main.c
extern unsigned long pgCursorX, pgCursorY; // pg.c
extern unsigned int pixels[BUF_WIDTH*BUF_HEIGHT]; // pg.c
extern unsigned int winPixels[BUF_WIDTH*BUF_HEIGHT]; // pg.c
extern unsigned int* printBuf; // pg.c
extern void* framebuffer; // pg.c
extern char* logDir; // psp2ch.c
extern int sel; // psp2ch.c
extern int tateFlag; // psp2ch.c
extern SceCtrlData pad; // psp2ch.c
extern SceCtrlData oldPad; // psp2ch.c
extern MESSAGE_HELPER mh; // psp2ch.c
extern S_2CH_FAVORITE* favList; // psp2chFavorite.c
extern S_2CH_SCREEN fav; // psp2chFavorite.c
extern S_2CH_ITA* itaList; // psp2chIta.c
extern S_2CH_SCREEN ita; // psp2chIta.c
extern S_2CH_THREAD* threadList; // psp2chThread.c
extern S_2CH_SCREEN thread; // psp2chThread.c
extern S_2CH_FAVORITE* findList; // psp2chSearch.c
extern S_2CH_SCREEN find; // psp2chSearch.c
extern int* threadSort; // psp2chThread.c
extern S_2CH_HEADER_COLOR resHeaderColor; // psp2ch.c
extern S_2CH_RES_COLOR resColor; // psp2ch.c
extern S_2CH_BAR_COLOR resBarColor; // psp2ch.c
extern S_2CH_HEADER_COLOR resAHeaderColor; // psp2ch.c
extern S_2CH_RES_COLOR resAColor; // psp2ch.c
extern S_2CH_BAR_COLOR resABarColor; // psp2ch.c

int preLine = -2;
char* resBuffer = NULL;
S_2CH_RES* resList = NULL;
S_2CH_SCREEN res;
S_2CH_URL_ANCHOR urlAnchor[50];
S_2CH_RES_ANCHOR resAnchor[50];
int urlAnchorCount = 0;
int resAnchorCount = 0;

static const char* cacheDir = "CACHE";
static S_2CH_ID_ANCHOR idAnchor[40];
static int idAnchorCount = 0;
static char jmpHost[32], jmpDir[32], jmpTitle[32];
static int jmpDat;

/**************
  レス表示
***************/
int psp2chFavoriteRes(int ret)
{
    return psp2chRes(favList[fav.select].host, favList[fav.select].dir, favList[fav.select].title, favList[fav.select].dat, ret);
}
int psp2chThreadRes(int ret)
{
    return psp2chRes(itaList[ita.select].host, itaList[ita.select].dir,itaList[ita.select].title,threadList[threadSort[thread.select]].dat,ret);
}
int psp2chJumpRes(int ret)
{
    return psp2chRes(jmpHost, jmpDir, jmpTitle, jmpDat, ret);
}
int psp2chSearchRes(int ret)
{
    return psp2chRes(findList[find.select].host, findList[find.select].dir, findList[find.select].title, findList[find.select].dat, ret);
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
    static int resMenu = -1, urlMenu = -1, idMenu = -1;
    int lineEnd, rMenu;

    if (tateFlag)
    {
        lineEnd = 35;
    }
    else
    {
        lineEnd = 20;
    }
    if (resList == NULL)
    {
        if (psp2chResList(host, dir, title, dat) < 0)
        {
            sel = ret;
            return -1;
        }
        totalLine = psp2chResSetLine(&bar);
        if (res.start > totalLine - lineEnd)
        {
            res.start = totalLine - lineEnd;
        }
    }
    if(sceCtrlPeekBufferPositive(&pad, 1))
    {
        rMenu = psp2chResCursorMove(&totalLine, &lineEnd, &cursorY, bar.view);
        if (pad.Buttons != oldPad.Buttons)
        {
            oldPad = pad;
            // SELECTボタン
            if (pad.Buttons & PSP_CTRL_SELECT)
            {
                tateFlag = (tateFlag) ? 0 : 1;
                totalLine = psp2chResSetLine(&bar);
                if (res.start > totalLine - lineEnd)
                {
                    res.start = totalLine - lineEnd;
                }
                for (i = 0; i < 50; i++)
                {
                    resAnchor[i].x2 = 0;
                    urlAnchor[i].x2 = 0;
                }
                preLine = -2;
            }
            // ○ボタン
            else if(pad.Buttons & PSP_CTRL_CIRCLE)
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
                    if ((p = strstr(urlAnchor[urlMenu].host, ".2ch.net")) && p[8] == '\0')
                    {
                        if (itaList == NULL)
                        {
                            if (psp2chItaList() < 0)
                            {
                                return ret;
                            }
                        }
                        strcpy(jmpHost, urlAnchor[urlMenu].host);
                        memcpy(path, urlAnchor[urlMenu].path, 256);
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
                            for (i = 0; i < ita.count; i++)
                            {
                                if (strcmp(itaList[i].host, urlAnchor[urlMenu].host) == 0 &&
                                        strcmp(itaList[i].dir, jmpDir) == 0)
                                {
                                    strcpy(jmpTitle, itaList[i].title);
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
                        memset(&mh,0,sizeof(MESSAGE_HELPER));
                        mh.options = PSP_UTILITY_MSGDIALOG_OPTION_TEXT |
                            PSP_UTILITY_MSGDIALOG_OPTION_YESNO_BUTTONS | PSP_UTILITY_MSGDIALOG_OPTION_DEFAULT_NO;
                        strcpy(mh.message, TEXT_11);
                        pspShowMessageDialog(&mh, DIALOG_LANGUAGE_AUTO);
                        sceCtrlPeekBufferPositive(&oldPad, 1);
                        if (mh.buttonPressed == PSP_UTILITY_MSGDIALOG_RESULT_YES)
                        {
                            free(resList);
                            resList = NULL;
                            res.start = 0;
                            sel = 6;
                            return ret;
                        }
                    }
                    /* 外部リンク */
                    else
                    {
                        psp2chUrlAnchor(urlMenu, title, dat, res.start*LINE_PITCH);
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
                    tmp = tateFlag;
                    tateFlag = 0;
                    if (message == NULL)
                    {
                        message = (char*)calloc(1, 2048);
                    }
                    if (message == NULL)
                    {
                        memset(&mh,0,sizeof(MESSAGE_HELPER));
                        sprintf(mh.message, "memorry error\n");
                        pspShowMessageDialog(&mh, DIALOG_LANGUAGE_AUTO);
                        sceCtrlPeekBufferPositive(&oldPad, 1);
                        return 0;
                    }
                    if (psp2chForm(host, dir, dat, resList[0].title, message) == 1)
                    {
                        free(message);
                        message = NULL;
                        psp2chSaveIdx(title, dat);
                        psp2chGetDat(host, dir, title, dat);
                        psp2chResList(host, dir, title, dat);
                        res.start++;
                        for (i = 0, j=0; i < res.count; i++)
                        {
                            j += resList[i].line;
                            j++;
                        }
                        totalLine = j;
                    }
                    tateFlag = tmp;
                }
                preLine = -2;
            }
            // ×ボタン
            else if(pad.Buttons & PSP_CTRL_CROSS)
            {
                if (rMenu)
                {
                }
                else
                {
                    psp2chSaveIdx(title, dat);
                    if (threadList)
                    {
                        psp2chSort(-1); // 前回のソート順で再ソート
                    }
                    sel = ret;
                    return ret;
                }
            }
            // △ボタン
            else if(pad.Buttons & PSP_CTRL_TRIANGLE)
            {
                if (resMenu >= 0)
                {
                    for (i = 0, j = 0; i < resAnchor[resMenu].res[0]; i++)
                    {
                        j += resList[i].line;
                        j++;
                    }
                    res.start = j;
                    if (res.start > totalLine - lineEnd)
                    {
                        res.start = totalLine - lineEnd;
                    }
                }
                else
                {
                    psp2chSaveIdx(title, dat);
                    psp2chGetDat(host, dir, title, dat);
                    psp2chResList(host, dir, title, dat);
                    totalLine = psp2chResSetLine(&bar);
                    res.start++;
                    if (res.start > totalLine - lineEnd)
                    {
                        res.start = totalLine - lineEnd;
                    }
                }
            }
            // □ボタン
            else if(pad.Buttons & PSP_CTRL_SQUARE)
            {
                if (rMenu)
                {
                    psp2chDelFavorite(title, dat);
                }
                else
                {
                    memset(&mh,0,sizeof(MESSAGE_HELPER));
                    mh.options = PSP_UTILITY_MSGDIALOG_OPTION_TEXT |
                        PSP_UTILITY_MSGDIALOG_OPTION_YESNO_BUTTONS | PSP_UTILITY_MSGDIALOG_OPTION_DEFAULT_NO;
                    strcpy(mh.message, TEXT_5);
                    pspShowMessageDialog(&mh, DIALOG_LANGUAGE_AUTO);
                    sceCtrlPeekBufferPositive(&oldPad, 1);
                    if (mh.buttonPressed == PSP_UTILITY_MSGDIALOG_RESULT_YES)
                    {
                        sprintf(path, "%s/%s/%s/%d.dat", cwDir, logDir, title, dat);
                        sceIoRemove(path);
                        sprintf(path, "%s/%s/%s/%d.idx", cwDir, logDir, title, dat);
                        sceIoRemove(path);
                        if (threadList)
                        {
                            threadList[threadSort[thread.select]].old = 0;
                            psp2chSort(-1);
                        }
                        sel = ret;
                        return 0;
                    }
                }
            }
        }
        psp2chResPadMove(&cursorX, &cursorY, bar.x, bar.view);
        // 矢印カーソルにレスアンカーリンクがあるか
        for (i = 0; i < 50; i++)
        {
            if (cursorY/LINE_PITCH+res.start == resAnchor[i].line &&
                cursorX > resAnchor[i].x1 && cursorX < resAnchor[i].x2)
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
            if (cursorY/LINE_PITCH+res.start == urlAnchor[i].line &&
                cursorX > urlAnchor[i].x1 && cursorX < urlAnchor[i].x2)
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
            if (cursorY/LINE_PITCH+res.start == idAnchor[i].line &&
                cursorX > idAnchor[i].x1 && cursorX < idAnchor[i].x2)
            {
                idMenu = i;
                break;
            }
            else
            {
                idMenu = -1;
            }
        }
        if (resMenu >= 0)
        {
            menuStr = "　○ : レス表\示　　　△ : レスに移動";
        }
        else if (urlMenu >= 0)
        {
            menuStr = "　○ : リンク表\示　　　";
        }
        else if (idMenu >= 0)
        {
            menuStr = "　○ : ID抽出　　　";
        }
        else if (rMenu)
        {
            // お気に入りリストにあるかチェック
            j = 0;
            if (fav.count)
            {
                for (i = 0; i < fav.count; i++)
                {
                    if (favList[i].dat == dat && strcmp(favList[i].title, title) == 0)
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
            menuStr = "　○ : 書き込み　　　× : 戻る　　　　　△ : 更新　　　　　□ : 削除　　　R : メニュー切替";
        }
        psp2chDrawRes(res.start);
        pgCopy(0, res.start*LINE_PITCH);
        bar.start = res.start * LINE_PITCH;
        pgScrollbar(bar, resBarColor);
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
int psp2chResCursorMove(int* totalLine, int* lineEnd, int* cursorY, int limitY)
{
    static int keyStart = 0, keyRepeat = 0, rMenu = 0;
    static clock_t keyTime = 0;
    int padUp = 0, padDown = 0;

    if (tateFlag)
    {
        if (*cursorY == 0 && pad.Lx == 255)
        {
            padUp = 1;
        }
        else if (*cursorY == limitY && pad.Lx == 0)
        {
            padDown = 1;
        }
    }
    else
    {
        if (*cursorY == 0 && pad.Ly == 0)
        {
            padUp = 1;
        }
        else if (*cursorY == limitY && pad.Ly == 255)
        {
            padDown = 1;
        }
    }
    if(pad.Buttons & PSP_CTRL_RTRIGGER)
    {
        rMenu = 1;
    }
    else
    {
        rMenu = 0;
    }
    if (pad.Buttons != oldPad.Buttons || keyRepeat)
    {
        if (pad.Buttons != oldPad.Buttons)
        {
            keyStart = 1;
        }
        else
        {
            keyStart = 0;
        }
        keyTime = clock();
        keyRepeat = 0;
        if((pad.Buttons & PSP_CTRL_UP && !tateFlag) || (pad.Buttons & PSP_CTRL_RIGHT && tateFlag) || padUp)
        {
            if (rMenu && !padUp)
            {
                res.start = 0;
            }
            else
            {
                res.start--;
                if (res.start < 0)
                {
                    res.start = 0;
                }
            }
        }
        if((pad.Buttons & PSP_CTRL_DOWN && !tateFlag) || (pad.Buttons & PSP_CTRL_LEFT && tateFlag) || padDown)
        {
            if (rMenu && !padDown)
            {
                res.start = *totalLine - *lineEnd;
            }
            else
            {
                res.start++;
                if (res.start > *totalLine - *lineEnd)
                {
                    res.start = *totalLine - *lineEnd;
                }
            }
        }
        if((pad.Buttons & PSP_CTRL_LEFT && !tateFlag) || (pad.Buttons & PSP_CTRL_UP && tateFlag))
        {
            res.start -= (*lineEnd - 2);
            if (res.start < 0)
            {
                res.start = 0;
            }
        }
        if((pad.Buttons & PSP_CTRL_RIGHT && !tateFlag) || (pad.Buttons & PSP_CTRL_DOWN && tateFlag))
        {
            res.start += (*lineEnd - 2);
            if (res.start > *totalLine - *lineEnd)
            {
                res.start = *totalLine - *lineEnd;
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

    if (tateFlag)
    {
        bar->view = 455;
        bar->x = RES_SCR_WIDTH_V;
        bar->y = 0;
        bar->w = RES_BAR_WIDTH_V;
        bar->h = 455;
        for (i = 0, j = 0; i < res.count; i++)
        {
            resList[i].line = psp2chCountRes(i, RES_SCR_WIDTH_V);
            j += resList[i].line;
            j++;
        }
    }
    else
    {
        bar->view = 259;
        bar->x = RES_SCR_WIDTH;
        bar->y = 0;
        bar->w = RES_BAR_WIDTH;
        bar->h = 259;
        for (i = 0, j = 0; i < res.count; i++)
        {
            resList[i].line = psp2chCountRes(i, RES_SCR_WIDTH);
            j += resList[i].line;
            j++;
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

    padX = pad.Lx - 127;
    padY = pad.Ly - 127;
    if(pad.Buttons & PSP_CTRL_RTRIGGER)
    {
        dL = 4;
        dS = 2;
    }
    else
    {
        dL = 16;
        dS = 8;
    }
    if (tateFlag)
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
datファイルをメモリに読み込みデータの区切りを'\0'（文字列終端）に書き換える
resList構造体のポインタに各データのアドレスを代入
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

    sprintf(path, "%s/%s/%s/%d.idx", cwDir, logDir, title, dat);
    fd = sceIoOpen(path, PSP_O_RDONLY, 0777);
    if (fd < 0)
    {
        res.start = 0;
        res.select = 0;
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
        sscanf(p, "%d %d", &res.start, &res.select);
    }
    sprintf(path, "%s/%s/%s/%d.dat", cwDir, logDir, title, dat);
    ret = sceIoGetstat(path, &st);
    if (ret < 0)
    {
        res.start = 0;
        res.select = 0;
        ret = psp2chGetDat(host, dir, title, dat);
        if (ret < 0)
        {
            return ret;
        }
        ret = sceIoGetstat(path, &st);
        if (ret < 0)
        {
            memset(&mh,0,sizeof(MESSAGE_HELPER));
            sprintf(mh.message, "File stat error\n%s", path);
            pspShowMessageDialog(&mh, DIALOG_LANGUAGE_AUTO);
            sceCtrlPeekBufferPositive(&oldPad, 1);
            return -1;
        }
    }
    free(resBuffer);
    resBuffer = (char*)malloc(st.st_size + 1);
    if (resBuffer == NULL)
    {
        memset(&mh,0,sizeof(MESSAGE_HELPER));
        strcpy(mh.message, "memorry error\nresBuffer");
        pspShowMessageDialog(&mh, DIALOG_LANGUAGE_AUTO);
        sceCtrlPeekBufferPositive(&oldPad, 1);
        return -1;
    }
    fd = sceIoOpen(path, PSP_O_RDONLY, 0777);
    if (fd < 0)
    {
        memset(&mh,0,sizeof(MESSAGE_HELPER));
        sprintf(mh.message, "File open error\n%s", path);
        pspShowMessageDialog(&mh, DIALOG_LANGUAGE_AUTO);
        sceCtrlPeekBufferPositive(&oldPad, 1);
        return -1;
    }
    sceIoRead(fd, resBuffer, st.st_size);
    sceIoClose(fd);
    resBuffer[st.st_size] = '\0';
    res.count = 0;
    p = resBuffer;
    while (*p)
    {
        if (*p++ == '\n')
        {
            res.count++;
        }
    }
    free(resList);
    resList = (S_2CH_RES*)malloc(sizeof(S_2CH_RES) * res.count);
    if (resList == NULL)
    {
        free(resBuffer);
        resBuffer = NULL;
        memset(&mh,0,sizeof(MESSAGE_HELPER));
        strcpy(mh.message, "memorry error\nresList");
        pspShowMessageDialog(&mh, DIALOG_LANGUAGE_AUTO);
        sceCtrlPeekBufferPositive(&oldPad, 1);
        return -1;
    }
    p = resBuffer;
    buf = resBuffer;
    for (ret = 0; ret < res.count; ret++)
    {
        p =  strstr(buf, delimiter);
        if (p == NULL)
        {
            free(resList);
            resList = NULL;
            free(resBuffer);
            resBuffer = NULL;
            memset(&mh,0,sizeof(MESSAGE_HELPER));
            sprintf(mh.message, "DAT log error1\n%d/%d", ret, res.count);
            pspShowMessageDialog(&mh, DIALOG_LANGUAGE_AUTO);
            sceCtrlPeekBufferPositive(&oldPad, 1);
            return -1;
        }
        *p = '\0';
        resList[ret].name = buf;
        p += 2;
        buf = p;
        p =  strstr(buf, delimiter);
        if (p == NULL)
        {
            free(resList);
            resList = NULL;
            free(resBuffer);
            resBuffer = NULL;
            memset(&mh,0,sizeof(MESSAGE_HELPER));
            sprintf(mh.message, "DAT log error2\n%d", ret);
            pspShowMessageDialog(&mh, DIALOG_LANGUAGE_AUTO);
            sceCtrlPeekBufferPositive(&oldPad, 1);
            return -1;
        }
        *p = '\0';
        resList[ret].mail = buf;
        p += 2;
        buf = p;
        p =  strstr(buf, delimiter);
        if (p == NULL)
        {
            free(resList);
            resList = NULL;
            free(resBuffer);
            resBuffer = NULL;
            memset(&mh,0,sizeof(MESSAGE_HELPER));
            sprintf(mh.message, "DAT log error3\n%d", ret);
            pspShowMessageDialog(&mh, DIALOG_LANGUAGE_AUTO);
            sceCtrlPeekBufferPositive(&oldPad, 1);
            return -1;
        }
        *p = '\0';
        resList[ret].date = buf;
        q =  strstr(buf, " BE:");
        if (q)
        {
            *q = '\0';
            q += 4;
            resList[ret].be = q;
        }
        else
        {
            resList[ret].be = NULL;
        }
        q =  strstr(buf, " ID:");
        if (q)
        {
            *q = '\0';
            q += 4;
            resList[ret].id = q;
        }
        else
        {
            resList[ret].id = NULL;
        }
        p += 2;
        buf = p;
        p =  strstr(buf, delimiter);
        if (p == NULL)
        {
            free(resList);
            resList = NULL;
            free(resBuffer);
            resBuffer = NULL;
            memset(&mh,0,sizeof(MESSAGE_HELPER));
            sprintf(mh.message, "DAT log error4\n%d", ret);
            pspShowMessageDialog(&mh, DIALOG_LANGUAGE_AUTO);
            sceCtrlPeekBufferPositive(&oldPad, 1);
            return -1;
        }
        *p = '\0';
        resList[ret].text = buf;
        p += 2;
        buf = p;
        p =  strstr(buf, "\n");
        if (p == NULL)
        {
            free(resList);
            resList = NULL;
            free(resBuffer);
            resBuffer = NULL;
            memset(&mh,0,sizeof(MESSAGE_HELPER));
            sprintf(mh.message, "DAT log error5\n%d", ret);
            pspShowMessageDialog(&mh, DIALOG_LANGUAGE_AUTO);
            sceCtrlPeekBufferPositive(&oldPad, 1);
            return -1;
        }
        *p = '\0';
        resList[ret].title = buf;
        p++;
        buf = p;
    }
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
    sprintf(path, "%s/%s/%s", cwDir, logDir, title);
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
    // check ita idx
    sprintf(path, "%s/%s/%s/%d.idx", cwDir, logDir, title, dat);
    fd = sceIoOpen(path, PSP_O_RDONLY, 0777);
    if (fd < 0)
    {
        buf[0] = '\0';
        range = 0;
        res.start = 0;
        res.select = 0;
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
        sscanf(q, "%d %d %d", &range, &res.start, &res.select);
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
            memset(&mh,0,sizeof(MESSAGE_HELPER));
            strcpy(mh.message, TEXT_10);
            pspShowMessageDialog(&mh, DIALOG_LANGUAGE_AUTO);
            psp2chCloseSocket(mySocket);
            sceCtrlPeekBufferPositive(&oldPad, 1);
            return -1;
            break;
        case 304: // Not modified
            psp2chCloseSocket(mySocket);
            return 0;
        default:
            memset(&mh,0,sizeof(MESSAGE_HELPER));
            sprintf(mh.message, "HTTP error\nhost %s path %s\nStatus code %d", host, path, ret);
            pspShowMessageDialog(&mh, DIALOG_LANGUAGE_AUTO);
            psp2chCloseSocket(mySocket);
            sceCtrlPeekBufferPositive(&oldPad, 1);
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
    sprintf(path, "%s/%s/%s/%d.dat", cwDir, logDir, title, dat);
    fd = sceIoOpen(path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_APPEND, 0777);
    if (fd < 0)
    {
        psp2chCloseSocket(mySocket);
        memset(&mh,0,sizeof(MESSAGE_HELPER));
        sprintf(mh.message, "File open error\n%s", path);
        pspShowMessageDialog(&mh, DIALOG_LANGUAGE_AUTO);
        sceCtrlPeekBufferPositive(&oldPad, 1);
        return fd;
    }
    sprintf(buf, "http://%s/%s/dat/%d.dat からデータを転送しています...", host, dir, dat);
    pgMenuBar(buf);
    sceDisplayWaitVblankStart();
    framebuffer = sceGuSwapBuffers();
    if (range && (recv(mySocket, buf, 1, 0) <= 0 || buf[0] !='\n'))
    {
        psp2chCloseSocket(mySocket);
        memset(&mh,0,sizeof(MESSAGE_HELPER));
        strcpy(mh.message, TEXT_4);
        pspShowMessageDialog(&mh, DIALOG_LANGUAGE_AUTO);
        sceCtrlPeekBufferPositive(&oldPad, 1);
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
    sprintf(path, "%s/%s/%s/%d.idx", cwDir, logDir, title, dat);
    fd = sceIoOpen(path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
    if (fd < 0)
    {
        psp2chCloseSocket(mySocket);
        memset(&mh,0,sizeof(MESSAGE_HELPER));
        sprintf(mh.message, "File open error\n%s", path);
        pspShowMessageDialog(&mh, DIALOG_LANGUAGE_AUTO);
        sceCtrlPeekBufferPositive(&oldPad, 1);
        return fd;
    }
    sceIoWrite(fd, resHeader.Last_Modified, strlen(resHeader.Last_Modified));
    sceIoWrite(fd, resHeader.ETag, strlen(resHeader.ETag));
    sprintf(buf, "%d\n%d\n%d\n", range,res.start, res.select);
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

    sprintf(path, "%s/%s/%s/%d.idx", cwDir, logDir, title, dat);
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
        sprintf(buf, "%s\n%s\n%d\n%d\n%d\n%d\n", lastModified, eTag, range, res.start, res.select, res.count);
        sceIoWrite(fd, buf, strlen(buf));
        sceIoClose(fd);
    }
    if (threadList)
    {
        threadList[threadSort[thread.select]].old = res.count;
    }
}

int psp2chDrawResStr(char* str, S_2CH_RES_COLOR c, int line, int lineEnd, int startX, int endX, int *drawLine)
{
    while ((str = pgPrintHtml(str, c, startX, endX, *drawLine)))
    {
        pgCursorX = startX;
        pgCursorY += LINE_PITCH;
        (*drawLine)++;
        if (++line > lineEnd)
        {
            break;
        }
        pgFillvram(c.bg, startX, pgCursorY, endX-startX, LINE_PITCH);
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

    sprintf(buf, " %d ", re + 1);
    str = buf;
    c.text = hc.num;
    if (*skip)
    {
        while ((str = pgCountHtml(str, endX, 1)))
        {
            pgCursorX = startX;
            if (--(*skip) == 0)
            {
                pgFillvram(c.bg, startX, pgCursorY, endX-startX, LINE_PITCH);
                line = psp2chDrawResStr(str, c, line, lineEnd, startX, endX, drawLine);
                break;
            }
        }
    }
    else
    {
        pgFillvram(c.bg, startX, pgCursorY, endX-startX, LINE_PITCH);
        line = psp2chDrawResStr(str, c, line, lineEnd, startX, endX, drawLine);
    }
    sprintf(buf, "名前:");
    str = buf;
    c.text = hc.name1;
    if (*skip)
    {
        while ((str = pgCountHtml(str, endX, 1)))
        {
            pgCursorX = startX;
            if (--(*skip) == 0)
            {
                pgFillvram(c.bg, startX, pgCursorY, endX-startX, LINE_PITCH);
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
    str = resList[re].name;
    c.text = hc.name2;
    if (*skip)
    {
        while ((str = pgCountHtml(str, endX, 1)))
        {
            pgCursorX = startX;
            if (--(*skip) == 0)
            {
                pgFillvram(c.bg, startX, pgCursorY, endX-startX, LINE_PITCH);
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
    sprintf(buf, "[%s] ", resList[re].mail);
    str = buf;
    c.text = hc.mail;
    if (*skip)
    {
        while ((str = pgCountHtml(str, endX, 1)))
        {
            pgCursorX = startX;
            if (--(*skip) == 0)
            {
                pgFillvram(c.bg, startX, pgCursorY, endX-startX, LINE_PITCH);
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
    str = resList[re].date;
    c.text = hc.date;
    if (*skip)
    {
        while ((str = pgCountHtml(str, endX, 1)))
        {
            pgCursorX = startX;
            if (--(*skip) == 0)
            {
                pgFillvram(c.bg, startX, pgCursorY, endX-startX, LINE_PITCH);
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
    if (resList[re].id)
    {
        sprintf(buf, " ID:");
        str = buf;
        c.text = hc.id1;
        for (i = 0, j = 0; i < res.count; i++)
        {
            if (resList[i].id && (strcmp(resList[i].id, resList[re].id) == 0))
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
                pgCursorX = startX;
                if (--(*skip) == 0)
                {
                    idAnchor[idAnchorCount].x1 = pgCursorX;
                    idAnchor[idAnchorCount].line = *drawLine;
                    strcpy(idAnchor[idAnchorCount].id, resList[re].id);
                    pgFillvram(c.bg, startX, pgCursorY, endX-startX, LINE_PITCH);
                    line = psp2chDrawResStr(str, c, line, lineEnd, startX, endX, drawLine);
                    break;
                }
            }
        }
        else
        {
            idAnchor[idAnchorCount].x1 = pgCursorX;
            idAnchor[idAnchorCount].line = *drawLine;
            strcpy(idAnchor[idAnchorCount].id, resList[re].id);
            line = psp2chDrawResStr(str, c, line, lineEnd, startX, endX, drawLine);
        }
        if (line > lineEnd)
        {
            idAnchor[idAnchorCount].x2 = endX;
            return line;
        }
        str = resList[re].id;
        c.text = hc.id3;
        if (*skip)
        {
            while ((str = pgCountHtml(str, endX, 1)))
            {
                pgCursorX = startX;
                if (--(*skip) == 0)
                {
                    pgFillvram(c.bg, startX, pgCursorY, endX-startX, LINE_PITCH);
                    line = psp2chDrawResStr(str, c, line, lineEnd, startX, endX, drawLine);
                    line++;
                    (*drawLine)++;
                    pgCursorX = startX;
                    pgCursorY += LINE_PITCH;
                    return line;
                }
            }
            pgCursorX = startX;
            (*skip)--;
        }
        else
        {
            line = psp2chDrawResStr(str, c, line, lineEnd, startX, endX, drawLine);
            if (*drawLine == idAnchor[idAnchorCount].line)
            {
                idAnchor[idAnchorCount].x2 = pgCursorX;
            }
            else
            {
                idAnchor[idAnchorCount].x2 = endX;
                idAnchorCount++;
                if (idAnchorCount >= 40)
                {
                    idAnchorCount = 0;
                }
                idAnchor[idAnchorCount].x1 = startX;
                idAnchor[idAnchorCount].line = *drawLine;
                strcpy(idAnchor[idAnchorCount].id, resList[re].id);
                idAnchor[idAnchorCount].x2 = pgCursorX;
            }
            idAnchorCount++;
            if (idAnchorCount >= 40)
            {
                idAnchorCount = 0;
            }
            line++;
            (*drawLine)++;
            pgCursorX = startX;
            pgCursorY += LINE_PITCH;
        }
    }
    else
    {
        if (*skip)
        {
            pgCursorX = startX;
            (*skip)--;
        }
        else
        {
            line++;
            (*drawLine)++;
            pgCursorX = startX;
            pgCursorY += LINE_PITCH;
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

    str = resList[res].text;
    if (*skip)
    {
        while (str)
        {
            str = pgCountHtml(str, endX, 1);
            pgCursorX = startX;
            if (--(*skip) == 0)
            {
                while (str)
                {
                    pgFillvram(c.bg, startX, pgCursorY, endX-startX, LINE_PITCH);
                    str = pgPrintHtml(str, c, startX, endX, *drawLine);
                    (*drawLine)++;
                    if (++line > lineEnd)
                    {
                        return line;
                    }
                    pgCursorX = startX;
                    pgCursorY += LINE_PITCH;
                }
            }
        }
        pgFillvram(c.bg, startX, pgCursorY, endX-startX, LINE_PITCH);
    }
    else
    {
        while (str)
        {
            pgFillvram(c.bg, startX, pgCursorY, endX-startX, LINE_PITCH);
            str = pgPrintHtml(str, c, startX, endX, *drawLine);
            (*drawLine)++;
            if (++line > lineEnd)
            {
                return line;
            }
            pgCursorX = startX;
            pgCursorY += LINE_PITCH;
        }
        pgFillvram(c.bg, startX, pgCursorY, endX-startX, LINE_PITCH);
    }
    pgCursorX = startX;
    pgCursorY += LINE_PITCH ;
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

    if (tateFlag)
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
            skip -= resList[i].line;
            skip--;
            i++;
            if (i >= res.count)
            {
                break;
            }
        }
        re = --i;
        skip++;
        skip += resList[i].line;
        pgCursorX = 0;
        pgCursorY = (LINE_PITCH*drawLine)&0x01FF;
        //pspDebugScreenInit();
        //pspDebugScreenPrintf("Y=%d\n", (int)pgCursorY);
        //pgWaitVn(200);
        line = psp2chDrawResHeader(re, &skip, lineEnd, lineEnd, 0, endX, resColor, resHeaderColor, &drawLine);
        if (line > lineEnd)
        {
            return;
        }
        line = psp2chDrawResText(re, &skip, lineEnd, lineEnd, 0, endX, resColor, &drawLine);
    }
    // 1行上に移動
    else if (drawLine == preLine-1)
    {
        preLine = drawLine;
        skip = drawLine;
        i = 0;
        while (skip >= 0)
        {
            skip -= resList[i].line;
            skip--;
            i++;
            if (i >= res.count)
            {
                break;
            }
        }
        re = --i;
        skip++;
        skip += resList[i].line;
        pgCursorX = 0;
        pgCursorY = (LINE_PITCH*drawLine)&0x01FF;
        line = psp2chDrawResHeader(re, &skip, lineEnd, lineEnd, 0, endX, resColor, resHeaderColor, &drawLine);
        if (line > lineEnd)
        {
            return;
        }
        line = psp2chDrawResText(re, &skip, lineEnd, lineEnd, 0, endX, resColor, &drawLine);
    }
    // 全画面書き直し
    else
    {
        preLine = drawLine;
        skip = drawLine;
        i = 0;
        while (skip >= 0)
        {
            skip -= resList[i].line;
            skip--;
            i++;
            if (i >= res.count)
            {
                break;
            }
        }
        re = --i;
        skip++;
        skip += resList[i].line;
        pgCursorX = 0;
        pgCursorY = (LINE_PITCH*drawLine)&0x01FF;
        resAnchorCount = 0;
        resAnchor[0].x1 = 0;
        line = 0;
        while (line <= lineEnd)
        {
            line = psp2chDrawResHeader(re, &skip, line, lineEnd, 0, endX, resColor, resHeaderColor, &drawLine);
            if (line > lineEnd)
            {
                break;
            }
            line = psp2chDrawResText(re, &skip, line, lineEnd, 0, endX, resColor, &drawLine);
            re++;
            if (re >= res.count)
            {
                pgFillvram(resColor.bg, 0, pgCursorY, endX, (lineEnd + 1 - line)*LINE_PITCH);
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

    pgCursorX = 0;
    pgCursorY = 0;
    sprintf(buf, " %d 名前:", res + 1);
    str = buf;
    while ((str = pgCountHtml(str, width, 1)))
    {
        pgCursorX = 0;
        count++;
    }
    str = resList[res].name;
    while ((str = pgCountHtml(str, width, 1)))
    {
        pgCursorX = 0;
        count++;
    }
    sprintf(buf, "[%s] ", resList[res].mail);
    str = buf;
    while ((str = pgCountHtml(str, width, 1)))
    {
        pgCursorX = 0;
        count++;
    }
    str = resList[res].date;
    while ((str = pgCountHtml(str, width, 1)))
    {
        pgCursorX = 0;
        count++;
    }
    if (resList[res].id)
    {
        sprintf(buf, " ID:");
        str = buf;
        while ((str = pgCountHtml(str, width, 1)))
        {
            pgCursorX = 0;
            count++;
        }
        str = resList[res].id;
        while ((str = pgCountHtml(str, width, 1)))
        {
            pgCursorX = 0;
            count++;
        }
    }
    pgCursorX = 0;
    count++;
    str = resList[res].text;
    while ((str = pgCountHtml(str, width, 1)))
    {
        pgCursorX = 0;
        count++;
    }
    pgCursorX = 0;
    count++;
    return count;
}

/*****************************
レスのウィンドウ表示
lineFlag:0=変更なし; 1=下に1行追加; 2=上に1行追加; その他:全画面再描画
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
    drawLine = anchor.start;
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
            skip -= psp2chCountRes(a.res[i], scrW);
            skip--;
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
            skip -= psp2chCountRes(a.res[i], scrW);
            skip--;
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
            skip -= psp2chCountRes(a.res[i], scrW);
            skip--;
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
        while (line <= lineEnd)
        {
            line = psp2chDrawResHeader(a.res[re], &skip, line, lineEnd, startX, scrW+startX, resAColor, resAHeaderColor, &drawLine);
            if (line > lineEnd)
            {
                break;
            }
            line = psp2chDrawResText(a.res[re], &skip, line, lineEnd, startX, scrW+startX, resAColor, &drawLine);
            re++;
            if (re >= a.resCount || a.res[re] >= res.count)
            {
                pgFillvram(resAColor.bg, startX-2, pgCursorY, scrW+2, (lineEnd - line + 1)*LINE_PITCH);
                break;
            }
        }
    }
}

void psp2chResAnchor(int anc)
{
    int keyStart = 0, keyRepeat = 0;
    int totalLine = 0;
    int i, j, lineFlag;
    clock_t keyTime = 0;
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
    totalLine = j;
    bar.total = j * LINE_PITCH;
    bar.start = 0;
    anchor.start = 0;
    anchor.select = 0;
    printBuf = winPixels;
    while (running)
    {
        if(sceCtrlPeekBufferPositive(&pad, 1))
        {
            lineFlag = 0;
            if (pad.Buttons != oldPad.Buttons || keyRepeat)
            {
                if (pad.Buttons != oldPad.Buttons)
                {
                    keyStart = 1;
                }
                else
                {
                    keyStart = 0;
                }
                keyTime = clock();
                keyRepeat = 0;
                if((pad.Buttons & PSP_CTRL_UP && !tateFlag) || (pad.Buttons & PSP_CTRL_RIGHT && tateFlag))
                {
                    lineFlag = 2;
                    anchor.start--;
                    if (anchor.start < 0)
                    {
                        anchor.start = 0;
                    }
                }
                if((pad.Buttons & PSP_CTRL_DOWN && !tateFlag) || (pad.Buttons & PSP_CTRL_LEFT && tateFlag))
                {
                    lineFlag = 1;
                    anchor.start++;
                    if (anchor.start > totalLine - lineEnd)
                    {
                        anchor.start = totalLine - lineEnd;
                    }
                }
                if((pad.Buttons & PSP_CTRL_LEFT && !tateFlag) || (pad.Buttons & PSP_CTRL_UP && tateFlag))
                {
                    lineFlag = 3;
                    anchor.start -= 13;
                    if (anchor.start < 0)
                    {
                        anchor.start = 0;
                    }
                }
                if((pad.Buttons & PSP_CTRL_RIGHT && !tateFlag) || (pad.Buttons & PSP_CTRL_DOWN && tateFlag))
                {
                    lineFlag = 3;
                    anchor.start += 13;
                    if (anchor.start > totalLine - lineEnd)
                    {
                        anchor.start = totalLine - lineEnd;
                    }
                }
                /*
                if (!keyRepeat && (pad.Buttons & PSP_CTRL_SELECT))
                {
                    tateFlag = (tateFlag) ? 0 : 1;
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
                    totalLine = j;
                    bar.total = j * LINE_PITCH;
                    totalLine = j;
                    bar.total = j * LINE_PITCH;
                    if (res.start > totalLine - lineEnd)
                    {
                        res.start = totalLine - lineEnd;
                    }
                    lineFlag = 3;
                }
                */
                psp2chDrawResAnchor(a, anchor, lineFlag);
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
            pgCopyWindow(anchor.start * LINE_PITCH, startX, startY, scrX, scrY);
            pgWindowFrame(startX, startY, startX + scrX + barW, startY + scrY);
            bar.start = anchor.start * LINE_PITCH;
            pgScrollbar(bar, resABarColor);
            pgMenuBar("　× : 戻る　　　");
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
            memset(&mh,0,sizeof(MESSAGE_HELPER));
            sprintf(mh.message, "HTTP error\nhost %s path %s\nStatus code %d", urlAnchor[anchor].host, urlAnchor[anchor].path, ret);
            pspShowMessageDialog(&mh, DIALOG_LANGUAGE_AUTO);
            psp2chCloseSocket(mySocket);
            sceCtrlPeekBufferPositive(&oldPad, 1);
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
    sprintf(path, "http://%s/%s からデータを転送しています...", urlAnchor[anchor].host, urlAnchor[anchor].path);
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

void psp2chIdAnchor(int anc)
{
    int keyStart = 0, keyRepeat = 0;
    int totalLine = 0;
    int i, j, lineFlag;
    clock_t keyTime = 0;
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
    totalLine = j;
    bar.total = j * LINE_PITCH;
    bar.start = 0;
    anchor.start = 0;
    anchor.select = 0;
    printBuf = winPixels;
    while (running)
    {
        if(sceCtrlPeekBufferPositive(&pad, 1))
        {
            lineFlag = 0;
            if (pad.Buttons != oldPad.Buttons || keyRepeat)
            {
                if (pad.Buttons != oldPad.Buttons)
                {
                    keyStart = 1;
                }
                else
                {
                    keyStart = 0;
                }
                keyTime = clock();
                keyRepeat = 0;
                if((pad.Buttons & PSP_CTRL_UP && !tateFlag) || (pad.Buttons & PSP_CTRL_RIGHT && tateFlag))
                {
                    lineFlag = 2;
                    anchor.start--;
                    if (anchor.start < 0)
                    {
                        anchor.start = 0;
                    }
                }
                if((pad.Buttons & PSP_CTRL_DOWN && !tateFlag) || (pad.Buttons & PSP_CTRL_LEFT && tateFlag))
                {
                    lineFlag = 1;
                    anchor.start++;
                    if (anchor.start > totalLine - lineEnd)
                    {
                        anchor.start = totalLine - lineEnd;
                    }
                }
                if((pad.Buttons & PSP_CTRL_LEFT && !tateFlag) || (pad.Buttons & PSP_CTRL_UP && tateFlag))
                {
                    lineFlag = 3;
                    anchor.start -= 13;
                    if (anchor.start < 0)
                    {
                        anchor.start = 0;
                    }
                }
                if((pad.Buttons & PSP_CTRL_RIGHT && !tateFlag) || (pad.Buttons & PSP_CTRL_DOWN && tateFlag))
                {
                    lineFlag = 3;
                    anchor.start += 13;
                    if (anchor.start > totalLine - lineEnd)
                    {
                        anchor.start = totalLine - lineEnd;
                    }
                }
                /*
                if (!keyRepeat && (pad.Buttons & PSP_CTRL_SELECT))
                {
                    tateFlag = (tateFlag) ? 0 : 1;
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
                    totalLine = j;
                    bar.total = j * LINE_PITCH;
                    totalLine = j;
                    bar.total = j * LINE_PITCH;
                    if (res.start > totalLine - lineEnd)
                    {
                        res.start = totalLine - lineEnd;
                    }
                    lineFlag = 3;
                }
                */
                psp2chDrawResAnchor(a, anchor, lineFlag);
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
            pgCopyWindow(anchor.start * LINE_PITCH, startX, startY, scrX, scrY);
            pgWindowFrame(startX, startY, startX + scrX + barW, startY + scrY);
            bar.start = anchor.start * LINE_PITCH;
            pgScrollbar(bar, resABarColor);
            pgMenuBar("　× : 戻る　　　");
            sceDisplayWaitVblankStart();
            framebuffer = sceGuSwapBuffers();
        }
    }
    return;
}
