/*
* $Id$
*/

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <time.h>
#include <sys/unistd.h>
#include <arpa/inet.h>
#include <pspdebug.h>
#include <pspsdk.h>
#include <pspwlan.h>
#include "psp2ch.h"
#include "psp2chIta.h"
#include "psp2chThread.h"
#include "psp2chRes.h"
#include "psp2chFavorite.h"
#include "psp2chSearch.h"
#include "utf8.h"
#include "pg.h"
#include "cp932.h"
#include "intraFont.h"
#include "libCat/Cat_Network.h"
#include "libCat/Cat_Resolver.h"

extern unsigned int list[512*512]; // pg.c
extern intraFont* jpn0; // pg.c

const char* userAgent = "Monazilla/1.00 (Compatible; PSP; ja) owata(^o^)/0.5.2";
S_2CH s2ch;
char cookie[128] = {0};
char keyWords[128];

/*********************************
メインループ
runningが真の間ループ（×ボタンの終了やhomeボタンでの終了でrunningが0）
セレクター(sel)で各関数へ分岐
レス表示はお気に入り、板一覧、レス表示中リンクジャンプ、全板検索結果からラッパー関数へ移動
移動元に戻るためretSelを使用（レス表示からの"戻る"でsel = retSel実行）
全板検索も移動元に戻るためretSel使用
*********************************/
int psp2ch(void)
{
    int retSel = 0;

    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
    while (s2ch.running)
    {
        switch (s2ch.sel)
        {
        case 1:
            psp2chFavorite();
            retSel = 1;
            break;
        case 2:
            psp2chIta();
            retSel = 2;
            break;
        case 3:
            psp2chThread(retSel);
            retSel = 3;
            break;
        case 4:
            retSel = psp2chFavoriteRes(retSel);
            break;
        case 5:
            retSel = psp2chThreadRes(retSel);
            break;
        case 6:
            retSel = psp2chJumpRes(retSel);
            break;
        case 7:
            psp2chSearch(retSel);
            retSel = 7;
            break;
        case 8:
            psp2chSearchRes(retSel);
            break;
        default:
            psp2chStart();
            break;
        }
    }
    return 0;
}

/*****************************
スタート画面
*****************************/
void psp2chStart(void)
{
    if(sceCtrlPeekBufferPositive(&s2ch.pad, 1))
    {
        if (s2ch.pad.Buttons != s2ch.oldPad.Buttons)
        {
            s2ch.oldPad = s2ch.pad;
            if(s2ch.pad.Buttons & PSP_CTRL_CROSS)
            {
                if (psp2chOwata())
                {
                    return;
                }
            }
            else if(s2ch.pad.Buttons & PSP_CTRL_START)
            {
                s2ch.sel = 2;
                return;
            }
            else if(s2ch.pad.Buttons & PSP_CTRL_CIRCLE)
            {
                s2ch.sel = 1;
                return;
            }
        }
        pgPrintMona();
        pgCopy(0, 0);
        sceDisplayWaitVblankStart();
        framebuffer = sceGuSwapBuffers();
    }
}

/*****************************
終了画面
*****************************/
int psp2chOwata(void)
{
    memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
    s2ch.mh.options = PSP_UTILITY_MSGDIALOG_OPTION_TEXT | PSP_UTILITY_MSGDIALOG_OPTION_YESNO_BUTTONS;
    strcpy(s2ch.mh.message, TEXT_3);
    pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
    sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
    if (s2ch.mh.buttonPressed == PSP_UTILITY_MSGDIALOG_RESULT_YES)
    {
        s2ch.tateFlag = 0;
        pgPrintOwata();
        pgCopy(0, 0);
        framebuffer = sceGuSwapBuffers();
        pgWaitVn(10);
        s2ch.running = 0;
        return 1;
    }
    return 0;
}

/*****************************
カーソル移動ルーチン
S_2CH_SCREEN構造体の
start:表示開始行
select:カーソル選択行
を変更しRボタン情報を返す
*****************************/
int psp2chCursorSet(S_2CH_SCREEN* line, int lineEnd)
{
    static int keyStart = 0, keyRepeat = 0;
    static clock_t keyTime = 0;
    int rMenu;
    int padUp = 0, padDown = 0;

    if (s2ch.tateFlag)
    {
        if (s2ch.pad.Lx == 255)
        {
            padUp = 1;
        }
        else if (s2ch.pad.Lx == 0)
        {
            padDown = 1;
        }
    }
    else
    {
        if (s2ch.pad.Ly == 0)
        {
            padUp = 1;
        }
        else if (s2ch.pad.Ly == 255)
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
    if (s2ch.pad.Buttons != s2ch.oldPad.Buttons || keyRepeat || padUp || padDown)
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
                line->start = 0;
                line->select = 0;
            }
            else
            {
                line->select--;
                if (line->select < 0)
                {
                    line->select = 0;
                }
                if (line->select < line->start)
                {
                    line->start = line->select;
                }
            }
        }
        if((s2ch.pad.Buttons & PSP_CTRL_DOWN && !s2ch.tateFlag) || (s2ch.pad.Buttons & PSP_CTRL_LEFT && s2ch.tateFlag) || padDown)
        {
            if (rMenu && !padDown)
            {
                line->start = line->count - lineEnd;
                if (line->start < 0)
                {
                    line->start = 0;
                }
                line->select = line->count - 1;
            }
            else
            {
                line->select++;
                if (line->select >= line->count)
                {
                    line->select = line->count -1;
                }
                if (line->select >= line->start + lineEnd)
                {
                    line->start = line->select - lineEnd + 1;
                }
                if (line->start < 0)
                {
                    line->start = 0;
                }
            }
        }
        if((s2ch.pad.Buttons & PSP_CTRL_LEFT && !s2ch.tateFlag) || (s2ch.pad.Buttons & PSP_CTRL_UP && s2ch.tateFlag))
        {
            if (line->select == line->start)
            {
                line->start -= lineEnd;
                if (line->start < 0)
                {
                    line->start = 0;
                }
            }
            line->select = line->start;
        }
        if((s2ch.pad.Buttons & PSP_CTRL_RIGHT && !s2ch.tateFlag) || (s2ch.pad.Buttons & PSP_CTRL_DOWN && s2ch.tateFlag))
        {
            if (line->select == line->start + lineEnd - 1)
            {
                line->start += lineEnd;
                if (line->start + lineEnd >= line->count)
                {
                    line->start = line->count - lineEnd;
                }
                if (line->start < 0)
                {
                    line->start = 0;
                }
            }
            line->select = line->start + lineEnd - 1;
            if (line->select >= line->count)
            {
                line->select = line->count -1;
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
            if (clock() - keyTime > 5000)
            {
                keyRepeat = 1;
            }
        }
    }
    return rMenu;
}

/*****************************
アナログパッドで横スクロール
xReverseを-1にするとスクロール方向が反転する
設定ファイル実装時に設定項目とする予定
*****************************/
int psp2chPadSet(int scrollX)
{
    static int xReverse = 1;

    if (s2ch.tateFlag)
    {
        if (s2ch.pad.Ly == 0)
        {
            scrollX += 8 * xReverse;
        }
        else if (s2ch.pad.Ly == 255)
        {
            scrollX -= 8 * xReverse;
        }
        if (scrollX > BUF_HEIGHT - SCR_HEIGHT)
        {
            scrollX = BUF_HEIGHT - SCR_HEIGHT;
        }
    }
    else
    {
        if (s2ch.pad.Lx == 0)
        {
            scrollX += 4 * xReverse;
        }
        else if (s2ch.pad.Lx == 255)
        {
            scrollX -= 4 * xReverse;
        }
        if (scrollX > BUF_WIDTH - SCR_WIDTH)
        {
            scrollX = BUF_WIDTH - SCR_WIDTH;
        }
    }
    if (scrollX < 0)
    {
        scrollX = 0;
    }
    return scrollX;
}

/***********************************
外部カラーセット
color.iniを読み込んで各カラーをセット
************************************/
#define setColor(A, B, C) \
    p = strstr(buf, (A));\
    if (p) {\
        p = strstr(p, "0x");\
        if(p){\
            p+=2;\
            sscanf(p, "%X", &(B));\
        }\
        else{(B) = (C);}\
    }\
    else{(B) = (C);}
void psp2chSetColor(void)
{
    SceUID fd;
    SceIoStat st;
    char file[256];
    char *buf;
    char *p;
    int ret;

    sprintf(file, "%s/color.ini", s2ch.cwDir);
    ret = sceIoGetstat(file, &st);
    if (ret >= 0)
    {
        buf = (char*)malloc(st.st_size + 1);
        if (buf)
        {
            fd = sceIoOpen(file, PSP_O_RDONLY, 0777);
            if (fd >= 0)
            {
                sceIoRead(fd, buf, st.st_size);
                sceIoClose(fd);
                buf[st.st_size] = '\0';
                setColor("RES_NUMBER", s2ch.resHeaderColor.num, RED);
                setColor("RES_NAME_HEAD", s2ch.resHeaderColor.name1, BLACK);
                setColor("RES_NAME_BODY", s2ch.resHeaderColor.name2, RGB(0x00, 0xCC, 0x00));
                setColor("RES_MAIL", s2ch.resHeaderColor.mail, RGB(0x99, 0x99, 0x99));
                setColor("RES_DATE", s2ch.resHeaderColor.date, BLACK);
                setColor("RES_ID_HEAD_1", s2ch.resHeaderColor.id1, BLUE);
                setColor("RES_ID_HEAD_2", s2ch.resHeaderColor.id2, RED);
                setColor("RES_ID_BODY", s2ch.resHeaderColor.id3, BLUE);
                setColor("RES_TEXT", s2ch.resColor.text, BLACK);
                setColor("RES_BG", s2ch.resColor.bg, RGB(0xE0, 0xE0, 0xE0));
                setColor("RES_LINK", s2ch.resColor.link, BLUE);
                setColor("RES_BAR_SLIDER", s2ch.resBarColor.slider, YELLOW);
                setColor("RES_BAR_BG", s2ch.resBarColor.bg, RGB(0x66, 0x66, 0xFF));

                setColor("RES_A_NUMBER", s2ch.resAHeaderColor.num, RED);
                setColor("RES_A_NAME_HEAD", s2ch.resAHeaderColor.name1, BLACK);
                setColor("RES_A_NAME_BODY", s2ch.resAHeaderColor.name2, RGB(0x00, 0xCC, 0x00));
                setColor("RES_A_MAIL", s2ch.resAHeaderColor.mail, RGB(0x99, 0x99, 0x99));
                setColor("RES_A_DATE", s2ch.resAHeaderColor.date, BLACK);
                setColor("RES_A_ID_HEAD_1", s2ch.resAHeaderColor.id1, BLUE);
                setColor("RES_A_ID_HEAD_2", s2ch.resAHeaderColor.id2, RED);
                setColor("RES_A_ID_BODY", s2ch.resAHeaderColor.id3, BLUE);
                setColor("RES_A_TEXT", s2ch.resAColor.text, BLACK);
                setColor("RES_A_BG", s2ch.resAColor.bg, RGB(0xFF, 0xFF, 0xCC));
                setColor("RES_A_LINK", s2ch.resAColor.link, BLUE);
                setColor("RES_A_BAR_SLIDER", s2ch.resABarColor.slider, RGB(0x00, 0xFF, 0xCC));
                setColor("RES_A_BAR_BG", s2ch.resABarColor.bg, RGB(0xCC, 0xFF, 0xFF));

                setColor("MENU_TEXT", s2ch.menuColor.text, WHITE);
                setColor("MENU_BG", s2ch.menuColor.bg, BLACK);
                setColor("MENU_BATTERY_1", s2ch.menuColor.bat1, GREEN);
                setColor("MENU_BATTERY_2", s2ch.menuColor.bat2, YELLOW);
                setColor("MENU_BATTERY_3", s2ch.menuColor.bat3, RED);

                setColor("THREAD_NUMBER", s2ch.threadColor.num, RED);
                setColor("THREAD_CATEGORY", s2ch.threadColor.category, RED);
                setColor("THREAD_TEXT_1", s2ch.threadColor.text1, BLUE);
                setColor("THREAD_TEXT_2", s2ch.threadColor.text2, RED);
                setColor("THREAD_BG", s2ch.threadColor.bg, RGB(0xCC, 0xFF, 0xCC));
                setColor("THREAD_COUNT_1", s2ch.threadColor.count1, BLACK);
                setColor("THREAD_COUNT_2", s2ch.threadColor.count2, BLACK);
                setColor("THREAD_SELECT_NUMBER", s2ch.threadColor.s_num, RED);
                setColor("THREAD_SELECT_CATEGORY", s2ch.threadColor.s_category, RGB(0x99, 0x00, 0x00));
                setColor("THREAD_SELECT_TEXT_1", s2ch.threadColor.s_text1, RGB(0x00, 0x00, 0x99));
                setColor("THREAD_SELECT_TEXT_2", s2ch.threadColor.s_text2, RGB(0x99, 0x00, 0x00));
                setColor("THREAD_SELECT_BG", s2ch.threadColor.s_bg, GRAY);
                setColor("THREAD_SELECT_COUNT_1", s2ch.threadColor.s_count1, BLACK);
                setColor("THREAD_SELECT_COUNT_2", s2ch.threadColor.s_count2, BLACK);

                setColor("CATE_ON_TEXT", s2ch.cateOnColor.cate.text, RGB(0xCC, 0x33, 0x00));
                setColor("CATE_ON_BG", s2ch.cateOnColor.cate.bg, WHITE);
                setColor("CATE_ON_S_TEXT", s2ch.cateOnColor.cate.s_text, WHITE);
                setColor("CATE_ON_S_BG", s2ch.cateOnColor.cate.s_bg, RGB(0xCC, 0x33, 0x00));
                setColor("ITA_OFF_TEXT", s2ch.cateOnColor.ita.text, RGB(0x66, 0x66, 0xFF));
                setColor("ITA_OFF_BG", s2ch.cateOnColor.ita.bg, GRAY);
                setColor("ITA_OFF_S_TEXT", s2ch.cateOnColor.ita.s_text, GRAY);
                setColor("ITA_OFF_S_BG", s2ch.cateOnColor.ita.s_bg, RGB(0x66, 0x66, 0xFF));
                setColor("CATE_ON_BASE", s2ch.cateOnColor.base, WHITE);

                setColor("CATE_OFF_TEXT", s2ch.cateOffColor.cate.text, RGB(0x88, 0x99, 0x66));
                setColor("CATE_OFF_BG", s2ch.cateOffColor.cate.bg, GRAY);
                setColor("CATE_OFF_S_TEXT", s2ch.cateOffColor.cate.s_text, GRAY);
                setColor("CATE_OFF_S_BG", s2ch.cateOffColor.cate.s_bg, RGB(0x88, 0x99, 0x66));
                setColor("ITA_ON_TEXT", s2ch.cateOffColor.ita.text, BLUE);
                setColor("ITA_ON_BG", s2ch.cateOffColor.ita.bg, WHITE);
                setColor("ITA_ON_S_TEXT", s2ch.cateOffColor.ita.s_text, WHITE);
                setColor("ITA_ON_S_BG", s2ch.cateOffColor.ita.s_bg, BLUE);
                setColor("CATE_OFF_BASE", s2ch.cateOffColor.base, WHITE);

                setColor("FORM_TITLE_TEXT", s2ch.formColor.title, WHITE);
                setColor("FORM_TITLE_BG", s2ch.formColor.title_bg, RED);

                setColor("MENU_WIN_TEXT", s2ch.menuWinColor.text, GRAY);
                setColor("MENU_WIN_BG", s2ch.menuWinColor.bg, BLACK);
                setColor("MENU_WIN_S_TEXT", s2ch.menuWinColor.s_text, WHITE);
                setColor("MENU_WIN_S_BG", s2ch.menuWinColor.s_bg, BLUE);
                free(buf);
                return;
            }
            else
            {
                free(buf);
            }
        }
    }
    // iniファイルがない時のデフォルト
    // レス本文
    s2ch.resHeaderColor.num = RED;
    s2ch.resHeaderColor.name1 = BLACK;
    s2ch.resHeaderColor.name2 = RGB(0x00, 0xCC, 0x00);
    s2ch.resHeaderColor.mail = RGB(0x99, 0x99, 0x99);
    s2ch.resHeaderColor.date = BLACK;
    s2ch.resHeaderColor.id1 = BLUE;
    s2ch.resHeaderColor.id2 = RED;
    s2ch.resHeaderColor.id3 = BLUE;
    s2ch.resColor.text = BLACK;
    s2ch.resColor.bg = RGB(0xE0, 0xE0, 0xE0);
    s2ch.resColor.link = BLUE;
    s2ch.resBarColor.slider = YELLOW;
    s2ch.resBarColor.bg = RGB(0x66, 0x66, 0xFF);
    // レスアンカー　
    s2ch.resAHeaderColor.num = RED;
    s2ch.resAHeaderColor.name1 = BLACK;
    s2ch.resAHeaderColor.name2 = RGB(0x00, 0xCC, 0x00);
    s2ch.resAHeaderColor.mail = RGB(0x99, 0x99, 0x99);
    s2ch.resAHeaderColor.date = BLACK;
    s2ch.resAHeaderColor.id1 = BLUE;
    s2ch.resAHeaderColor.id2 = RED;
    s2ch.resAHeaderColor.id3 = BLUE;
    s2ch.resAColor.text = BLACK;
    s2ch.resAColor.bg = RGB(0xFF, 0xFF, 0xCC);
    s2ch.resAColor.link = BLUE;
    s2ch.resABarColor.slider = RGB(0x00, 0xFF, 0xCC);
    s2ch.resABarColor.bg = RGB(0xCC, 0xFF, 0xFF);
    // メニューバー　
    s2ch.menuColor.text = WHITE;
    s2ch.menuColor.bg = BLACK;
    s2ch.menuColor.bat1 = GREEN;
    s2ch.menuColor.bat2 = YELLOW;
    s2ch.menuColor.bat3 = RED;
    // スレッド一覧
    s2ch.threadColor.num = RED;
    s2ch.threadColor.category = RED;
    s2ch.threadColor.text1 = BLUE;
    s2ch.threadColor.text2 = RED;
    s2ch.threadColor.bg = RGB(0xCC, 0xFF,0xCC);
    s2ch.threadColor.count1 = BLACK;
    s2ch.threadColor.count2 = BLACK;
    s2ch.threadColor.s_num = RED;
    s2ch.threadColor.s_category = RGB(0x99, 0x00, 0x00);
    s2ch.threadColor.s_text1 = RGB(0x00, 0x00, 0x99);
    s2ch.threadColor.s_text2 = RGB(0x99, 0x00, 0x00);
    s2ch.threadColor.s_bg = GRAY;
    s2ch.threadColor.s_count1 = BLACK;
    s2ch.threadColor.s_count2 = BLACK;
    // カテゴリー・板一覧
    s2ch.cateOnColor.cate.text = RGB(0xCC, 0x33, 0x00);
    s2ch.cateOnColor.cate.bg = WHITE;
    s2ch.cateOnColor.cate.s_text = WHITE;
    s2ch.cateOnColor.cate.s_bg = RGB(0xCC, 0x33, 0x00);
    s2ch.cateOnColor.ita.text = RGB(0x66, 0x66, 0xFF);
    s2ch.cateOnColor.ita.bg = GRAY;
    s2ch.cateOnColor.ita.s_text = GRAY;
    s2ch.cateOnColor.ita.s_bg = RGB(0x66, 0x66, 0xFF);
    s2ch.cateOnColor.base = WHITE;
    s2ch.cateOffColor.cate.text = RGB(0x88, 0x99, 0x66);
    s2ch.cateOffColor.cate.bg = GRAY;
    s2ch.cateOffColor.cate.s_text = GRAY;
    s2ch.cateOffColor.cate.s_bg = RGB(0x88, 0x99, 0x66);
    s2ch.cateOffColor.ita.text = BLUE;
    s2ch.cateOffColor.ita.bg = WHITE;
    s2ch.cateOffColor.ita.s_text = WHITE;
    s2ch.cateOffColor.ita.s_bg = BLUE;
    s2ch.cateOffColor.base = WHITE;
    // 送信フォーム
    s2ch.formColor.title = WHITE;
    s2ch.formColor.title_bg = RED;
    // メニューウィンドウ
    s2ch.menuWinColor.text = GRAY;
    s2ch.menuWinColor.bg = BLACK;
    s2ch.menuWinColor.s_text = WHITE;
    s2ch.menuWinColor.s_bg = BLUE;
}

/********************
レス表示画面 ボタンの割り当て
********************/
#define setButton(A, B, C) \
    p = strstr(buf, (A));\
    if (p) {\
        p = strstr(p, "=");\
        if(p){\
            p++;\
            sscanf(p, "%d", &(B));\
        }\
        else{(B) = (C);}\
    }\
    else{(B) = (C);}
void psp2chSetResButtons(void)
{
    SceUID fd;
    SceIoStat st;
    char file[256];
    char *buf;
    char *p;
    int ret;

    sprintf(file, "%s/button.ini", s2ch.cwDir);
    ret = sceIoGetstat(file, &st);
    if (ret >= 0)
    {
        buf = (char*)malloc(st.st_size + 1);
        if (buf)
        {
            fd = sceIoOpen(file, PSP_O_RDONLY, 0777);
            if (fd >= 0)
            {
                sceIoRead(fd, buf, st.st_size);
                sceIoClose(fd);
                buf[st.st_size] = '\0';
                setButton("RES_FORM_H", s2ch.btnResH.form, 8192);
                setButton("RES_BACK_H", s2ch.btnResH.back, 16384);
                setButton("RES_RELOAD_H", s2ch.btnResH.reload, 4096);
                setButton("RES_DATDEL_H", s2ch.btnResH.datDel, 32768);
                setButton("RES_SHIFT_H", s2ch.btnResH.change, 512);
                setButton("RES_FORM_V", s2ch.btnResV.form, 8192);
                setButton("RES_BACK_V", s2ch.btnResV.back, 16384);
                setButton("RES_RELOAD_V", s2ch.btnResV.reload, 4096);
                setButton("RES_DATDEL_V", s2ch.btnResV.datDel, 32768);
                setButton("RES_SHIFT_V", s2ch.btnResV.change, 512);

                setButton("RES_TOP_H", s2ch.btnResH.top, 16);
                setButton("RES_END_H", s2ch.btnResH.end, 64);
                setButton("RES_ADDFAV_H", s2ch.btnResH.addFav, 8192);
                setButton("RES_DELFAV_H", s2ch.btnResH.delFav, 32768);
                setButton("RES_TOP_V", s2ch.btnResV.top, 16);
                setButton("RES_END_V", s2ch.btnResV.end, 64);
                setButton("RES_ADDFAV_V", s2ch.btnResV.addFav, 8192);
                setButton("RES_DELFAV_V", s2ch.btnResV.delFav, 32768);

                setButton("RES_NUMRES_H", s2ch.btnResH.resForm, 8192);
                setButton("RES_NUMRES_V", s2ch.btnResV.resForm, 8192);

                setButton("RES_IDVIEW_H", s2ch.btnResH.idView, 8192);
                setButton("RES_IDNG_H", s2ch.btnResH.idNG, 32768);
                setButton("RES_IDVIEW_V", s2ch.btnResV.idView, 8192);
                setButton("RES_IDNG_V", s2ch.btnResV.idNG, 32768);

                setButton("RES_RESVIEW_H", s2ch.btnResH.resView, 8192);
                setButton("RES_RESMOVE_H", s2ch.btnResH.resMove, 4096);
                setButton("RES_RESVIEW_V", s2ch.btnResV.resView, 8192);
                setButton("RES_RESMOVE_V", s2ch.btnResV.resMove, 4096);

                setButton("RES_URL_H", s2ch.btnResH.url, 8192);
                setButton("RES_URL_V", s2ch.btnResV.url, 8192);

                setButton("RES_UP_H", s2ch.btnResH.up, 16);
                setButton("RES_PAGEUP_H", s2ch.btnResH.pUp, 128);
                setButton("RES_DOWN_H", s2ch.btnResH.down, 64);
                setButton("RES_PAGEDOWN_H", s2ch.btnResH.pDown, 32);
                setButton("RES_UP_V", s2ch.btnResV.up, 32);
                setButton("RES_PAGEUP_V", s2ch.btnResV.pUp, 16);
                setButton("RES_DOWN_V", s2ch.btnResV.down, 128);
                setButton("RES_PAGEDOWN_V", s2ch.btnResV.pDown, 64);
                free(buf);
                return;
            }
            else
            {
                free(buf);
            }
        }
    }
    s2ch.btnResH.form     = PSP_CTRL_CIRCLE;
    s2ch.btnResH.back     = PSP_CTRL_CROSS;
    s2ch.btnResH.reload   = PSP_CTRL_TRIANGLE;
    s2ch.btnResH.datDel   = PSP_CTRL_SQUARE;
    s2ch.btnResH.change   = PSP_CTRL_RTRIGGER;

    s2ch.btnResH.top      = PSP_CTRL_UP;
    s2ch.btnResH.end      = PSP_CTRL_DOWN;
    s2ch.btnResH.addFav   = PSP_CTRL_CIRCLE;
    s2ch.btnResH.delFav   = PSP_CTRL_SQUARE;

    s2ch.btnResH.resForm  = PSP_CTRL_CIRCLE;

    s2ch.btnResH.idView   = PSP_CTRL_CIRCLE;
    s2ch.btnResH.idNG     = PSP_CTRL_SQUARE;

    s2ch.btnResH.resView  = PSP_CTRL_CIRCLE;
    s2ch.btnResH.resMove  = PSP_CTRL_TRIANGLE;

    s2ch.btnResH.url      = PSP_CTRL_CIRCLE;

    s2ch.btnResH.up       = PSP_CTRL_UP;
    s2ch.btnResH.pUp      = PSP_CTRL_LEFT;
    s2ch.btnResH.down     = PSP_CTRL_DOWN;
    s2ch.btnResH.pDown    = PSP_CTRL_RIGHT;

    s2ch.btnResV.form     = PSP_CTRL_CIRCLE;
    s2ch.btnResV.back     = PSP_CTRL_CROSS;
    s2ch.btnResV.reload   = PSP_CTRL_TRIANGLE;
    s2ch.btnResV.datDel   = PSP_CTRL_SQUARE;
    s2ch.btnResV.change   = PSP_CTRL_RTRIGGER;

    s2ch.btnResV.top      = PSP_CTRL_RIGHT;
    s2ch.btnResV.end      = PSP_CTRL_LEFT;
    s2ch.btnResV.addFav   = PSP_CTRL_CIRCLE;
    s2ch.btnResV.delFav   = PSP_CTRL_SQUARE;

    s2ch.btnResV.resForm  = PSP_CTRL_CIRCLE;

    s2ch.btnResV.idView   = PSP_CTRL_CIRCLE;
    s2ch.btnResV.idNG     = PSP_CTRL_SQUARE;

    s2ch.btnResV.resView  = PSP_CTRL_CIRCLE;
    s2ch.btnResV.resMove  = PSP_CTRL_TRIANGLE;

    s2ch.btnResV.url      = PSP_CTRL_CIRCLE;

    s2ch.btnResV.up       = PSP_CTRL_RIGHT;
    s2ch.btnResV.pUp      = PSP_CTRL_UP;
    s2ch.btnResV.down     = PSP_CTRL_LEFT;
    s2ch.btnResV.pDown    = PSP_CTRL_DOWN;
}

/***********************************
ネットモジュールのロード初期化
************************************/
int psp2chInit(void)
{
    int ret;

    ret = sceUtilityLoadNetModule(PSP_NET_MODULE_COMMON);
    if (ret < 0)
    {
        memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
        strcpy(s2ch.mh.message, "Load module common errror");
        pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
        return ret;
    }
    ret = sceUtilityLoadNetModule(PSP_NET_MODULE_INET);
    if (ret < 0)
    {
        memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
        strcpy(s2ch.mh.message, "Load module inet errror");
        pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
        return ret;
    }
    ret = Cat_NetworkInit();
    if (ret < 0)
    {
        memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
        strcpy(s2ch.mh.message, "Cat_NetworkInit errror");
        pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
        return ret;
    }
    psp2chSetColor();
    psp2chSetResButtons();
    psp2chResSetMenuString();
    s2ch.running = 1;
    s2ch.sel = 0;
    s2ch.tateFlag = 0;
    s2ch.logDir = "log";
    s2ch.urlAnchorCount = 0;
    s2ch.resAnchorCount = 0;
    s2ch.idAnchorCount = 0;
    s2ch.numAnchorCount = 0;
    s2ch.pgCursorX = 0;
    s2ch.pgCursorY = 0;
    s2ch.categoryList = NULL;
    s2ch.itaList = NULL;
    s2ch.favList = NULL;
    s2ch.findList = NULL;
    s2ch.favItaList = NULL;
    s2ch.threadList = NULL;
    s2ch.resList = NULL;
    return 0;
}

/**************************
終了前の後始末
**************************/
int psp2chTerm(void)
{
    Cat_NetworkTerm();
    return 0;
}

/*****************************
アクセスポイントへ接続してソケット作成
成功時にはソケット(>=0)を返す
失敗時には<0を返す
psp2chRequest()とpsp2chPost()から呼ばれます。
*****************************/
int psp2chOpenSocket(void)
{
    int mySocket, ret;

    ret = psp2chApConnect();
    if (ret < 0 && ret != 0x80110405)
    {
        return ret;
    }
    mySocket = socket(PF_INET, SOCK_STREAM, 0 );
#ifdef DEBUG
    pspDebugScreenPrintf("socket = %d\n", mySocket);
#endif
    return mySocket;
}

/*****************************
ソケットを閉じる
*****************************/
int psp2chCloseSocket(int mySocket)
{
    shutdown(mySocket, SHUT_RDWR);
    return close(mySocket);
}

/*****************************
名前解決　ホスト名をIPアドレスに変換
Sfiya猫さんのパッチ使用
*****************************/
int psp2chResolve(const char* host, struct in_addr* addr)
{
    SceUID fd;
    char buf[256];
    char hosts[2048];
    char *p;

    sprintf(buf, "  %s のアドレスを解決しています", host);
    pgMenuBar(buf);
    sceDisplayWaitVblankStart();
    framebuffer = sceGuSwapBuffers();
    sprintf(buf, "%s/hosts", s2ch.cwDir);
    fd = sceIoOpen(buf, PSP_O_RDONLY, 0777);
    if (fd >= 0)
    {
        sceIoRead(fd, hosts, sizeof(hosts));
        sceIoClose(fd);
        p = strstr(hosts, host);
        if (p)
        {
            *p = '\0';
            p = strrchr(hosts, '\n');
            if (p)
            {
                sscanf(p, " %s", buf);
                if (inet_aton(buf, addr))
                {
                    return 0;
                }
            }
        }
    }
    if(Cat_ResolverURL( host, addr ) < 0 )
    {
        pgMenuBar("  Cat_ResolverURL error");
        sceDisplayWaitVblankStart();
        framebuffer = sceGuSwapBuffers();
        return -1;
    }
    return 0;
}

/*****************************
HTTP で GETリクエスト
ソケットを作成
アドレス解決してリクエストヘッダを送信します
成功時にはソケット(>=0)を返します
*****************************/
int psp2chRequest(const char* host, const char* path, const char* header) {
    int mySocket;
    int ret;
    char requestText[512];
    char buf[512];
    struct sockaddr_in sain;
    struct in_addr addr;
    char *p;

    mySocket = psp2chOpenSocket();
    if (mySocket < 0) {
        return mySocket;
    }
    p = strrchr(path, '#');
    if (p)
    {
        *p = '\0';
    }
    sprintf(requestText,
        "GET /%s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "User-Agent: %s\r\n"
        "%s"
        "Connection: close\r\n\r\n"
        , path, host, userAgent, header);
    ret = psp2chResolve(host, &addr);
    if (ret < 0) {
        return ret;
    }
    sprintf(buf, "  %s (%s)", host, inet_ntoa(addr));
    pgMenuBar(buf);
    sceDisplayWaitVblankStart();
    framebuffer = sceGuSwapBuffers();
    // Tell the socket to connect to the IP address we found, on port 80 (HTTP)
    sain.sin_family = AF_INET;
    sain.sin_port = htons(80);
    sain.sin_addr.s_addr = addr.s_addr;
    sprintf(buf, "  http://%s/%s に接続しています", host, path);
    pgMenuBar(buf);
    sceDisplayWaitVblankStart();
    framebuffer = sceGuSwapBuffers();
    ret = connect(mySocket,(struct sockaddr *)&sain, sizeof(sain) );
    if (ret < 0) {
        memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
        strcpy(s2ch.mh.message, "Connect errror");
        pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
        sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
        return ret;
    }
    pgMenuBar("接続しました");
    sceDisplayWaitVblankStart();
    framebuffer = sceGuSwapBuffers();
#ifdef DEBUG
    pspDebugScreenPrintf("send request\n%s\n", requestText);
#endif
    // send our request
    send(mySocket, requestText, strlen(requestText), 0 );

    return mySocket;
}

/*****************************
HTTP で POST 送信します。
ソケット作成
アドレス解決してヘッダとボディを送信
成功時にはソケット(>=0)を返す
*****************************/
int psp2chPost(char* host, char* dir, int dat, char* cook, char* body)
{
    int mySocket;
    int ret;
    char requestText[512];
    char buf[512];
    struct sockaddr_in sain;
    struct in_addr addr;

    mySocket = psp2chOpenSocket();
    if (mySocket < 0) {
        return -1;
    }
    sprintf(requestText,
        "POST /test/bbs.cgi HTTP/1.1\r\n"
        "Host: %s\r\n"
        "User-Agent: %s\r\n"
        "Referer: http://%s/test/read.cgi/%s/%d/l50\r\n"
        "Cookie: %s\r\n"
        "Content-Type: application/x-www-form-urlencoded\r\n"
        "Content-Length: %d\r\n\r\n"
        , host, userAgent, host, dir, dat, cook, strlen(body));
    ret = psp2chResolve(host, &addr);
    if (ret < 0) {
        return -1;
    }
    sprintf(buf, "  %s (%s)", host, inet_ntoa(addr));
    pgMenuBar(buf);
    sceDisplayWaitVblankStart();
    framebuffer = sceGuSwapBuffers();
    // Tell the socket to connect to the IP address we found, on port 80 (HTTP)
    sain.sin_family = AF_INET;
    sain.sin_port = htons(80);
    sain.sin_addr.s_addr = addr.s_addr;
    sprintf(buf, "  http://%s/test/bbs.cgi に接続しています", host);
    pgCopy(0, 0);
    pgMenuBar(buf);
    sceDisplayWaitVblankStart();
    framebuffer = sceGuSwapBuffers();
    ret = connect( mySocket,(struct sockaddr *)&sain, sizeof(sain) );
    if (ret < 0) {
        memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
        sprintf(s2ch.mh.message, "Can't connect bbs.cgi");
        pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
        return -1;
    }
#ifdef DEBUG
    pspDebugScreenPrintf("send request\n%s\n", requestText);
#endif
    // send our request
    send(mySocket, requestText, strlen(requestText), 0 );
    send(mySocket, body, strlen(body), 0 );
    return mySocket;
}

/*****************************
ソケットから最初の1行（ステータスライン）を読み込みます。
ステータスコードをintに変換して返す
HTTP/1.1 200 OK
なら200を返す
*****************************/
int psp2chGetStatusLine(int mySocket)
{
    char in;
    char incomingBuffer[256];
    int i = 0;
    int verMajor, verMinor, code;
    char phrase[256];

     do
     {
        if (recv( mySocket, &in, 1, 0 ) == 0)
        {
            return -1;
        }
        incomingBuffer[i] = in;
        if (++i > 255)
        {
            return -1;
        }
    } while (in != '\n');
    incomingBuffer[i] = '\0';
    pgMenuBar(incomingBuffer);
    sceDisplayWaitVblankStart();
    framebuffer = sceGuSwapBuffers();
    sscanf(incomingBuffer, "HTTP/%d.%d %d %s\r\n", &verMajor, &verMinor, &code, phrase);
    return code;
}

/*****************************
ソケットからレスポンスヘッダを読み込みます。
HTTP_HEADERS構造体に
Content-Length
Content-Type
Last-Modified
ETag
の内容がコピーされます。
cookie[]にSet-Cookieの内容が追加されます。
Content-Lengthがintで返されます
Content-Lengthを返さない場合もあるので注意（CGI等）
*****************************/
int psp2chGetHttpHeaders(int mySocket, HTTP_HEADERS* header)
{
    int i = 0;
    char in;
    char incomingBuffer[256];
    int contentLength = 0;
    char *p;

    while(recv(mySocket, &in, 1, 0) > 0)  // if recv returns 0, the socket has been closed.
    {
        if (in == '\r')
        {
            continue;
        }
        incomingBuffer[i] = in;
        i++;
        if (in == '\n')
        {
            incomingBuffer[i] = 0;
            if (incomingBuffer[0] == '\n')  // End of headers
            {
                break;
            }
            if (strstr(incomingBuffer, "Content-Length:"))
            {
                sscanf(incomingBuffer, "Content-Length: %d\n", &contentLength);
                header->Content_Length = contentLength;
            }
            else if (strstr(incomingBuffer, "Content-Type:"))
            {
                strcpy(header->Content_Type, &incomingBuffer[14]);
            }
            else if (strstr(incomingBuffer, "Last-Modified:"))
            {
                strcpy(header->Last_Modified, &incomingBuffer[15]);
            }
            else if (strstr(incomingBuffer, "ETag:"))
            {
                strcpy(header->ETag, &incomingBuffer[6]);
            }
            else if (strstr(incomingBuffer, "Set-Cookie:"))
            {
                p = strchr(incomingBuffer, ';');
                *p = '\0';
                if (cookie[0])
                {
                    strcat(cookie, "; ");
                }
                strcat(cookie, &incomingBuffer[12]);
            }
#ifdef DEBUG
            pspDebugScreenPrintf("%s", incomingBuffer);
#endif
            i = 0;
        }
    }
    return contentLength;
}

static void draw_callback( void* pvUserData )
{
    // 描画処理
    // ダイアログ表示のさいの背景を描画する

    minimalRender();
}

static void screen_update_callback( void* pvUserData ) {
    // 更新処理
    // フレームバッファのスワップなど

    sceGuSync( 0, 0 );
    sceDisplayWaitVblankStartCB();
    framebuffer = sceGuSwapBuffers();
}

/*****************************
指定された設定番号のアクセスポイントに接続します
psp2chApConnect()から呼ばれます。
*****************************/
int connect_to_apctl(int config)
{
    int err;
    int stateLast = -1;
    int state;
    char buf[32];
    clock_t time;

    err = sceNetApctlConnect(config);
    if (err != 0)
    {
        memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
        strcpy(s2ch.mh.message, "sceNetApctlConnect error");
        pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
        return 0;
    }

    pgMenuBar("  Connecting...");
    sceDisplayWaitVblankStart();
    framebuffer = sceGuSwapBuffers();
    time = clock();
    while (1)
    {
        err = sceNetApctlGetState(&state);
        if (err != 0)
        {
            memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
            sprintf(s2ch.mh.message, "sceNetApctlGetState error\n0x%X", err);
            pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
            break;
        }
        if (state == 2)
        {
            strcpy(buf, "　アクセスポイントに接続中です。");
            stateLast = state;
        }
        if (state == 3)
        {
            strcpy(buf, "　IPアドレスを取得中です。");
            stateLast = state;
        }
        if (state == 4)
        {
            break;  // connected with static IP
        }
        pgMenuBar(buf);
        sceDisplayWaitVblankStart();
        framebuffer = sceGuSwapBuffers();
        if (clock() > time + 1000000 * 8)
        {
            break;
        }
    }
    if (state != 4)
    {
        pgMenuBar("  接続に失敗しました。");
    }
    else
    {
        pgMenuBar("  接続しました。");
        Cat_ResolverInitEngine();
    }
    sceDisplayWaitVblankStart();
    framebuffer = sceGuSwapBuffers();
    return 0;
}

/*****************************
PSP内のアクセスポイント設定を検索します
設定が2個以上あれば選択ダイアログを表示します。
設定が1個のみの場合はその値で接続します。
設定がない場合は-1が返されます。
*****************************/
int psp2chApConnect(void)
{
    struct
    {
        int index;
        char name[128];
    } ap[2];
    int count = 0;
    int i;
    char buf[32];

    memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
    s2ch.mh.options = PSP_UTILITY_MSGDIALOG_OPTION_TEXT;
    if (sceWlanGetSwitchState() == 0)
    {
        strcpy(s2ch.mh.message, TEXT_1);
        pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
        return -1;
    }
    else if (sceWlanDevIsPowerOn() == 0)
    {
        strcpy(s2ch.mh.message, TEXT_2);
        pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
        return -1;
    }
    if (sceNetApctlGetInfo(8, buf) == 0)
    {
        // IP取得済み
        return 0;
    }
    pgMenuBar("接続の設定を検索しています...");
    pgWaitVn(2);
    framebuffer = sceGuSwapBuffers();
    for (i = 1; i < 100; i++) // skip the 0th connection
    {
        if (sceUtilityCheckNetParam(i) != 0) continue;  // more
        sceUtilityGetNetParam(i, 0, (netData*) ap[count].name);
        ap[count].index = i;
        count++;
        if (count > 1) break;  // no more room
    }
    if (count == 1)
    {
        return connect_to_apctl(ap[0].index);
    }
    else if (count > 1)
    {
        // 返り値 < 0 : エラー
        // 返り値 = 0 : 接続した
        // 返り値 > 0 : キャンセルされた
        return Cat_NetworkConnect( draw_callback, screen_update_callback, 0 );
        // 全部0なのかな？？？
    }
    return -1;
}

/*****************************
OnScreenKeyboardで文字入力
title: OSKの右下に表示されるタイトル文字列(SJIS)
text:入力文字を保存するポインタ
num:入力文字列の長さ
lines:入力文字列の行数
OSKからはワイド文字(UCS)で帰ってくるのでSJISに変換しています。
*****************************/
void psp2chGets(char* title, char* text, int num, int lines)
{
    unsigned short* wc;
    unsigned char* p;
    char* s;
    int sjis, i;
    unsigned short string[128];
    unsigned short pretext[2048];
    OSK_HELPER helper;
    char* buf = "人生ｵﾜﾀ＼(^o^)／";

    if (num > 2048)
    {
        num = 2048;
    }
    /* sjis => ucs */
    if (title && *title != '\0')
    {
        p = (unsigned char*)title;
    }
    else
    {
        p = (unsigned char*)buf;
    }
    i = 0;
    while (*p && i < 127)
    {
        sjis = conv_sjiswin_wchar(*p++);
        if (sjis == -1)
        {
            continue;
        }
        string[i] = (unsigned short)sjis;
        i++;
    }
    string[i] = 0;
    i = 0;
    p = (unsigned char*)text;
    while (*p && i < 2048)
    {
        sjis = conv_sjiswin_wchar(*p++);
        if (sjis == -1)
        {
            continue;
        }
        pretext[i] = (unsigned short)sjis;
        i++;
    }
    pretext[i] = 0;
    memset(&helper,0,sizeof(OSK_HELPER));
    helper.title = (unsigned short*)string;
    helper.pretext = (unsigned short*)pretext;
    helper.textlength = num;
    helper.textlimit = num;
    helper.lines = lines;

    pspShowOSK(&helper,DIALOG_LANGUAGE_AUTO);
    /* ucs => sjis */
    wc = (unsigned short*)helper.text;
    s = text;
    i = 0;
    while (*wc)
    {
        sjis = conv_wchar_sjiswin((int)(*wc++));
        if (sjis < 0x100)
        {
            s[i] = sjis;
            i++;
        }
        else
        {
            if (i >= num - 2)
            {
                break;
            }
            s[i] = sjis & 0xFF;
            i++;
            s[i] = (sjis>>8) & 0xFF;
            i++;
        }
        if (i >= num - 1)
        {
            break;
        }
    }
    s[i] = '\0';
}

/****************
入力ダイアログ表示
text1: ダイアログに表示されるタイトル
text2: OSKに表示するタイトル
戻り値 0=入力, -1=取消し
*****************/
int psp2chInputDialog(const unsigned short* text1, char* text2)
{
    int temp;

    temp = s2ch.tateFlag;
    s2ch.tateFlag = 0;
    keyWords[0] = '\0';
    while (s2ch.running)
    {
        if(sceCtrlPeekBufferPositive(&s2ch.pad, 1))
        {
            if (s2ch.pad.Buttons != s2ch.oldPad.Buttons)
            {
                s2ch.oldPad = s2ch.pad;
                if(s2ch.pad.Buttons & PSP_CTRL_CIRCLE)
                {
                    psp2chGets(text2, keyWords, 128, 1);
                }
                if(s2ch.pad.Buttons & PSP_CTRL_CROSS)
                {
                    s2ch.tateFlag = temp;
                    return -1;
                }
                if(s2ch.pad.Buttons & PSP_CTRL_SQUARE)
                {
                    break;
                }
            }
            sceGuStart(GU_DIRECT, list);
            sceGuClearDepth(0);
            sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
            pgFillvram(BLUE, 80, 60, 320, 45);
            pgEditBox(WHITE, 140, 85, 340, 100);
            s2ch.pgCursorX = 142;
            s2ch.pgCursorY =  87;
            pgPrint(keyWords, BLACK, WHITE, 340);
            pgCopy(0, 0);
            pgMenuBar("　○ : 入力　　　× : 戻る　　　□ : 決定");
            s2ch.pgCursorX = 240;
            s2ch.pgCursorY =  77;
            intraFontSetStyle(jpn0, 1.0f, YELLOW, 0, INTRAFONT_ALIGN_CENTER);
            intraFontPrintUCS2(jpn0, s2ch.pgCursorX, s2ch.pgCursorY, text1);
            sceGuFinish();
            sceGuSync(0,0);
        }
        sceDisplayWaitVblankStart();
        framebuffer = sceGuSwapBuffers();
    }
    s2ch.tateFlag = temp;
    return 0;
}
