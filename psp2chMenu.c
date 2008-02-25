/*
* $Id$
*/

#include "pspdialogs.h"
#include <stdio.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include "pg.h"
#include "psp2chMenu.h"

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
extern S_2CH_TXT_COLOR menuWinColor; // psp2ch.c

#define MENU_WIDTH (80)
#define MENU_HEIGHT (52)
#define MENU_ITEM (4)

/****************
メニュー選択ウィンドウ
****************/
int psp2chMenu(int pixelsX, int pixelsY)
{
    static char* menuStr = "";
    int lineEnd;
    const char* menuList[] = {"基本設定", "NG設定"};
    static S_2CH_SCREEN menu;
    int startX, startY, scrX, scrY;

    if (tateFlag)
    {
        startX = (SCR_HEIGHT - MENU_WIDTH) / 2;
        startY = (SCR_WIDTH - MENU_HEIGHT) / 2;    }
    else
    {
        startX = (SCR_WIDTH - 100) / 2;
        startY = (SCR_HEIGHT - MENU_HEIGHT) / 2;
    }
    scrX = 100;
    scrY = MENU_HEIGHT;
    lineEnd = MENU_ITEM;
    menu.count = 2;
    printBuf = winPixels;
    while (running)
    {
        if(sceCtrlPeekBufferPositive(&pad, 1))
        {
            psp2chCursorSet(&menu, lineEnd);
            if (pad.Buttons != oldPad.Buttons)
            {
                oldPad = pad;
                if(pad.Buttons & PSP_CTRL_CIRCLE)
                {
                    switch (menu.select)
                    {
                    case 0:
                        break;
                    case 1:
                        psp2chMenuNG(pixelsX, pixelsY);
                        break;
                    }
                }
                else if(pad.Buttons & PSP_CTRL_CROSS)
                {
                    printBuf = pixels;
                    break;
                }
                else if(pad.Buttons & PSP_CTRL_TRIANGLE)
                {
                }
                else if(pad.Buttons & PSP_CTRL_SQUARE)
                {
                }
            }
            menuStr = "　○ : 決定　　　　× : 戻る　　　";
            psp2chDrawMenu((char**)menuList, menu, startX, startY, scrX, scrY);
            pgCopyWindow(0, startX, startY, scrX, scrY);
            pgWindowFrame(startX, startY, startX + scrX, startY + scrY);
            pgMenuBar(menuStr);
            sceDisplayWaitVblankStart();
            framebuffer = sceGuSwapBuffers();
        }
    }
    return 0;
}

void psp2chMenuNG(int pixelsX, int pixelsY)
{
    static char* menuStr = "";
    int lineEnd;
    const char* menuList[] = {"NG名前登録", "NG名前削除", "NGID削除"};
    static S_2CH_SCREEN menu;
    int startX, startY, scrX, scrY;

    if (tateFlag)
    {
        startX = (SCR_HEIGHT - MENU_WIDTH) / 2;
        startY = (SCR_WIDTH - MENU_HEIGHT) / 2;    }
    else
    {
        startX = (SCR_WIDTH - MENU_WIDTH) / 2;
        startY = (SCR_HEIGHT - MENU_HEIGHT) / 2;
    }
    scrX = MENU_WIDTH;
    scrY = MENU_HEIGHT;
    lineEnd = MENU_ITEM;
    menu.count = 3;
    printBuf = pixels;
    pgCopy(pixelsX, pixelsY);
    framebuffer = sceGuSwapBuffers();
    pgCopy(pixelsX, pixelsY);
    printBuf = winPixels;
    while (running)
    {
        if(sceCtrlPeekBufferPositive(&pad, 1))
        {
            psp2chCursorSet(&menu, lineEnd);
            if (pad.Buttons != oldPad.Buttons)
            {
                oldPad = pad;
                if(pad.Buttons & PSP_CTRL_CIRCLE)
                {
                    switch (menu.select)
                    {
                    case 0: // NG name add
                        break;
                    case 1: // NG name del
                        break;
                    case 2: // NG ID del
                        break;
                    }
                }
                else if(pad.Buttons & PSP_CTRL_CROSS)
                {
                    break;
                }
            }
            menuStr = "　○ : 決定　　　　× : 戻る　　　";
            psp2chDrawMenu((char**)menuList, menu, startX, startY, scrX, scrY);
            pgCopyWindow(0, startX, startY, scrX, scrY);
            pgWindowFrame(startX, startY, startX + scrX, startY + scrY);
            pgMenuBar(menuStr);
            sceDisplayWaitVblankStart();
            framebuffer = sceGuSwapBuffers();
        }
    }
}

void psp2chDrawMenu(char** menuList, S_2CH_SCREEN menu, int x, int y, int width, int height)
{
    int i;

    pgCursorX = x;
    pgCursorY = y;
    pgFillvram(menuWinColor.bg, x, y, width, height);
    for (i = 0; i < menu.count; i++)
    {
        if (i == menu.select)
        {
            pgFillvram(menuWinColor.s_bg, x, pgCursorY, width, LINE_PITCH);
            pgPrint(menuList[i], menuWinColor.s_text, menuWinColor.s_bg, x + width);
        }
        else
        {
            pgPrint(menuList[i], menuWinColor.text, menuWinColor.bg, x + width);
        }
        pgCursorX = x;
        pgCursorY += LINE_PITCH;
    }
}

