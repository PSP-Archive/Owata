/*
* $Id$
*/

#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include "psp2ch.h"
#include "psp2chIta.h"
#include "psp2chThread.h"
#include "psp2chRes.h"
#include "psp2chFavorite.h"
#include "psp2chSearch.h"
#include "psp2chMenu.h"
#include "psp2chNet.h"
#include "psp2chIni.h"
#include "utf8.h"
#include "pg.h"
#include "cp932.h"
#include "intraFont.h"

extern unsigned int list[512*512]; // pg.c
extern unsigned short pixels[BUF_WIDTH*BUF_HEIGHT*2]; // pg.c
extern unsigned short winPixels[BUF_WIDTH*BUF_HEIGHT*2]; // pg.c
extern unsigned short* printBuf; // pg.c
extern intraFont* jpn0; // pg.c
extern RECT barSrcRectH; // pg.c
extern RECT barSrcRectV; // pg.c
extern RECT menuDstRectH; // pg.c
extern RECT menuDstRectV; // pg.c
extern RECT titleDstRectH; // pg.c
extern RECT titleDstRectV; // pg.c

char* ver = "0.8.2";
S_2CH s2ch;
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
{	static int start = 0;
	if (start == 0)
	{
		pgPrintMona();
		s2ch.pgCursorX = 450;
		s2ch.pgCursorY = 260;
		pgPrint(ver, BLUE, WHITE, SCR_WIDTH);
		pgWaitVn(20);
		start = 1;
	}
    if(sceCtrlPeekBufferPositive(&s2ch.pad, 1))
    {
        if (s2ch.pad.Buttons != s2ch.oldPad.Buttons)
        {
            s2ch.oldPad = s2ch.pad;
			start = 0;
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
        pgWaitVn(20);
        pgCopy(0, 0);
        sceDisplayWaitVblankStart();
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
int psp2chCursorSet(S_2CH_SCREEN* line, int lineEnd, int shift, int* change)
{
    static int keyStart = 0, keyRepeat = 0;
    static clock_t keyTime = 0;
    int rMenu, start, select;
    int padUp = 0, padDown = 0;

	*change = 0;
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

    if(s2ch.pad.Buttons & shift)
    {
        rMenu = 1;
    }
    else
    {
        rMenu = 0;
    }
	start = line->start;
	select = line->select;
	if (padUp)
	{
        line->select--;
		if (line->start)
		{
			line->start--;
		}
		else
		{
            if (line->select < 0)
            {
                line->select = 0;
            }
		}
	}
	else if (padDown)
	{
        line->select++;
		if (line->start <= (line->count - lineEnd))
		{
			line->start++;
		}
		else
		{
            if (line->select >= line->count)
            {
                line->select = line->count -1;
            }
		}
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
        if((s2ch.pad.Buttons & s2ch.listH.up && !s2ch.tateFlag) || (s2ch.pad.Buttons & s2ch.listV.up && s2ch.tateFlag))
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
        if((s2ch.pad.Buttons & s2ch.listH.down && !s2ch.tateFlag) || (s2ch.pad.Buttons & s2ch.listV.down && s2ch.tateFlag))
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
        if((s2ch.pad.Buttons & s2ch.listH.pUp && !s2ch.tateFlag) || (s2ch.pad.Buttons & s2ch.listV.pUp && s2ch.tateFlag))
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
        if((s2ch.pad.Buttons & s2ch.listH.pDown && !s2ch.tateFlag) || (s2ch.pad.Buttons & s2ch.listV.pDown && s2ch.tateFlag))
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
        if((s2ch.pad.Buttons & s2ch.listH.top && !s2ch.tateFlag) || (s2ch.pad.Buttons & s2ch.listV.top && s2ch.tateFlag))
        {
            if (rMenu && !padUp)
            {
                line->start = 0;
                line->select = 0;
            }
        }
        if((s2ch.pad.Buttons & s2ch.listH.end && !s2ch.tateFlag) || (s2ch.pad.Buttons & s2ch.listV.end && s2ch.tateFlag))
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
            if (clock() - keyTime > 1000)
            {
                keyRepeat = 1;
            }
        }
    }
	if (start != line->start || select != line->select)
	{
		*change = 1;
	}
    return rMenu;
}

/*****************************
アナログパッドで横スクロール
s2ch.cfg.xReverseを-1にするとスクロール方向が反転する
*****************************/
int psp2chPadSet(int scrollX)
{
    if (s2ch.tateFlag)
    {
        if (s2ch.pad.Ly == 0)
        {
            scrollX += 8 * s2ch.cfg.padReverse;
        }
        else if (s2ch.pad.Ly == 255)
        {
            scrollX -= 8 * s2ch.cfg.padReverse;
        }
        if (scrollX > BUF_HEIGHT * 2 - SCR_HEIGHT)
        {
            scrollX = BUF_HEIGHT * 2 - SCR_HEIGHT;
        }
    }
    else
    {
        if (s2ch.pad.Lx == 0)
        {
            scrollX += 4 * s2ch.cfg.padReverse;
        }
        else if (s2ch.pad.Lx == 255)
        {
            scrollX -= 4 * s2ch.cfg.padReverse;
        }
        if (scrollX > BUF_WIDTH * 2 - SCR_WIDTH)
        {
            scrollX = BUF_WIDTH * 2 - SCR_WIDTH;
        }
    }
    if (scrollX < 0)
    {
        scrollX = 0;
    }
    return scrollX;
}

/***********************************
フォントのパラメータをセット
************************************/
void psp2chSetFontParam(void)
{
    switch(s2ch.font.pitch)
    {
    case 10:
        s2ch.font.lineH = 26;
        s2ch.font.lineV = 46;
        break;
    case 11:
        s2ch.font.lineH = 24;
        s2ch.font.lineV = 42;
        break;
    case 12:
        s2ch.font.lineH = 22;
        s2ch.font.lineV = 38;
        break;
    case 13:
        s2ch.font.lineH = 20;
        s2ch.font.lineV = 35;
        break;
    case 14:
        s2ch.font.lineH = 18;
        s2ch.font.lineV = 32;
        break;
    case 15:
        s2ch.font.lineH = 17;
        s2ch.font.lineV = 30;
        break;
    case 16:
        s2ch.font.lineH = 16;
        s2ch.font.lineV = 28;
        break;
    }
}

/***********************************
フォントサイズでメニュー・タイトルバーのパラメータをセット
************************************/
void psp2chSetBarParam(void)
{
	barSrcRectH.left = 0;
	barSrcRectH.top = 0;
	barSrcRectH.right = SCR_WIDTH;
	barSrcRectH.bottom = FONT_HEIGHT;

	barSrcRectV.left = 0;
	barSrcRectV.top = 0;
	barSrcRectV.right = SCR_HEIGHT;
	barSrcRectV.bottom = FONT_HEIGHT + LINE_PITCH;

	menuDstRectH.left = 0;
	menuDstRectH.top = SCR_HEIGHT - FONT_HEIGHT;
	menuDstRectH.right = SCR_WIDTH;
	menuDstRectH.bottom = SCR_HEIGHT;

	menuDstRectV.left = 0;
	menuDstRectV.top = 0;
	menuDstRectV.right = FONT_HEIGHT + LINE_PITCH;
	menuDstRectV.bottom = SCR_HEIGHT;

	titleDstRectH.left = 0;
	titleDstRectH.top = 0;
	titleDstRectH.right = SCR_WIDTH;
	titleDstRectH.bottom = FONT_HEIGHT;

	titleDstRectV.left = SCR_WIDTH - (FONT_HEIGHT + LINE_PITCH);
	titleDstRectV.top = 0;
	titleDstRectV.right = SCR_WIDTH;
	titleDstRectV.bottom = SCR_HEIGHT;
}

/***********************************
ネットモジュールのロード
初期化
************************************/
int psp2chInit(void)
{
	if (psp2chNetInit() < 0)
	{
		return -1;
	}
    s2ch.logDir = "log";
    s2ch.fontDir = "font";
    s2ch.colorDir = "color";
    psp2chIniSetColor(NULL);
    psp2chIniSetButtons();
    psp2chItaSetMenuString();
    psp2chFavSetMenuString();
    psp2chThreadSetMenuString();
    psp2chSearchSetMenuString();
    psp2chResSetMenuString();
    psp2chMenuSetMenuString();
    psp2chIniLoadConfig();
    psp2chMenuFontSet(0);
    s2ch.running = 1;
    s2ch.sel = 0;
    s2ch.tateFlag = 0;
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
	psp2chSetFontParam();
	psp2chSetBarParam();
	pgCursorColorSet();
    return 0;
}

/**************************
終了前の後始末
**************************/
int psp2chTerm(void)
{
	psp2chNetTerm();
    return 0;
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
	printBuf = winPixels;
	pgFillvram(0x8000, 0, 0, SCR_WIDTH, SCR_HEIGHT, 2);
    pgEditBox(WHITE, 140, 85, 340, 101);
	intraFontSetStyle(jpn0, 1.0f, 0xFF00FFFF, 0, INTRAFONT_ALIGN_CENTER);
    pgPrintMenuBar("　○ : 入力　　　× : 戻る　　　□ : 決定");
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
                else if(s2ch.pad.Buttons & PSP_CTRL_CROSS)
                {
                    s2ch.tateFlag = temp;
					printBuf = pixels;
                    return -1;
                }
                else if(s2ch.pad.Buttons & PSP_CTRL_SQUARE)
                {
                    break;
                }
            }
        }
		printBuf = pixels;
		s2ch.tateFlag = temp;
		pgCopy(s2ch.viewX, 0);
		printBuf = winPixels;
		s2ch.tateFlag = 0;
		pgEditBox(WHITE, 140, 85, 340, 101);
		s2ch.pgCursorX = 142;
		s2ch.pgCursorY =  87;
		pgPrint(keyWords, BLACK, WHITE, 340);
		pgWaitVn(10);
		pgCopy(0, 0);
		sceGuStart(GU_DIRECT, list);
		intraFontPrintUCS2(jpn0, 240, 77, text1);
		sceGuFinish();
		sceGuSync(0,0);
		pgCopyMenuBar();
		sceDisplayWaitVblankStart();
		framebuffer = sceGuSwapBuffers();
    }
    s2ch.tateFlag = temp;
	printBuf = pixels;
    return 0;
}

/****************
エラーダイアログ表示
*****************/
void psp2chErrorDialog(const char* fmt, ...)
{
    va_list list;

    va_start(list, fmt);
    memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
    vsprintf(s2ch.mh.message, fmt, list);
    pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
    sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
    va_end(list);
}
