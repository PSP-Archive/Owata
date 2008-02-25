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
extern S_2CH_THREAD_COLOR threadColor; // psp2ch.c

#define MENU_WIDTH (80)
#define MENU_HEIGHT (52)
#define MENU_ITEM (4)
int psp2chMenu(void)
{
    static char* menuStr = "";
    int lineEnd, rMenu;
    const char* menuList[] = {"基本設定", "NG設定"};
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
    menu.count = 2;
    printBuf = winPixels;
    while (running)
    {
        if(sceCtrlPeekBufferPositive(&pad, 1))
        {
            rMenu = psp2chCursorSet(&menu, lineEnd);
            if (pad.Buttons != oldPad.Buttons)
            {
                oldPad = pad;
                if(pad.Buttons & PSP_CTRL_CIRCLE)
                {
                    if (rMenu)
                    {
                    }
                    else
                    {
                        switch (menu.select)
                        {
                        case 0:
                            break;
                        case 1:
                            break;
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
                        printBuf = pixels;
                        break;
                    }
                }
                else if(pad.Buttons & PSP_CTRL_TRIANGLE)
                {
                    if (rMenu)
                    {
                    }
                    else
                    {
                    }
                }
                else if(pad.Buttons & PSP_CTRL_SQUARE)
                {
                    if (rMenu)
                    {
                    }
                    else
                    {
                    }
                }
            }
            if (rMenu)
            {
            }
            else
            {
                    menuStr = "　○ : 決定　　　　× : 戻る　　　";
            }
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

void psp2chDrawMenu(char** menuList, S_2CH_SCREEN menu, int x, int y, int width, int height)
{
    int i;

    pgCursorX = x;
    pgCursorY = y;
    pgFillvram(threadColor.bg, x, y, width, height);
    for (i = 0; i < menu.count; i++)
    {
        if (i == menu.select)
        {
            pgFillvram(threadColor.s_bg, x, pgCursorY, width, LINE_PITCH);
            pgPrint(menuList[i], threadColor.s_text1, threadColor.s_bg, x + width);
        }
        else
        {
            pgPrint(menuList[i], threadColor.text1, threadColor.bg, x + width);
        }
        pgCursorX = x;
        pgCursorY += LINE_PITCH;
    }
}

