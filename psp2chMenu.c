/*
* $Id$
*/

#include <stdio.h>
#include <malloc.h>
#include <pspdebug.h>
#include "pg.h"
#include "psp2ch.h"
#include "psp2chMenu.h"
#include "psp2chRes.h"

extern S_2CH s2ch; // psp2ch.c
extern char keyWords[128]; //psp2ch.c
extern unsigned int pixels[BUF_WIDTH*BUF_HEIGHT]; // pg.c
extern unsigned int winPixels[BUF_WIDTH*BUF_HEIGHT]; // pg.c
extern unsigned int* printBuf; // pg.c
extern const char *sBtnH[];
extern const char *sBtnV[];

const char* ngNameFile = "ngname.txt";
const char* ngIDFile = "ngid.txt";

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

void psp2chMenuSetMenuString(void)
{
    int index1, index2, index3;
    int i, tmp;

    getIndex(s2ch.menuWinH.ok, index1);
    getIndex(s2ch.menuWinH.esc, index2);
    sprintf(s2ch.menuWinH.main, "　%s : 決定　　　　　%s : 戻る",
            sBtnH[index1], sBtnH[index2]);

    getIndex(s2ch.menuWinV.ok, index1);
    getIndex(s2ch.menuWinV.esc, index2);
    sprintf(s2ch.menuWinV.main, "　%s : 決定　　　　　%s : 戻る",
            sBtnV[index1], sBtnV[index2]);

    getIndex(s2ch.menuNGH.save, index1);
    getIndex(s2ch.menuNGH.esc, index2);
    getIndex(s2ch.menuNGH.del, index3);
    sprintf(s2ch.menuNGH.main, "　%s : 保存　　　　%s : 取消し　　　%s : 削除",
            sBtnH[index1], sBtnH[index2], sBtnH[index3]);

    getIndex(s2ch.menuNGV.save, index1);
    getIndex(s2ch.menuNGV.esc, index2);
    getIndex(s2ch.menuNGV.del, index3);
    sprintf(s2ch.menuNGV.main, "　%s : 保存　　　　%s : 取消し　　　%s : 削除",
            sBtnV[index1], sBtnV[index2], sBtnV[index3]);
}

/****************
メニュー選択ウィンドウ
****************/
#define MENU_WIDTH (80)
#define MENU_HEIGHT (52)
#define MENU_ITEM (4)
int psp2chMenu(int pixelsX, int pixelsY)
{
    const char* menuList[] = {"NG 設定", "LAN 切断"};
    static char* menuStr = "";
    int lineEnd;
    static S_2CH_SCREEN menu;
    int startX, startY, scrX, scrY;

    if (s2ch.tateFlag)
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
    while (s2ch.running)
    {
        if(sceCtrlPeekBufferPositive(&s2ch.pad, 1))
        {
            psp2chCursorSet(&menu, lineEnd, 0);
            if (s2ch.pad.Buttons != s2ch.oldPad.Buttons)
            {
                s2ch.oldPad = s2ch.pad;
                if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.menuWinH.ok) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.menuWinV.ok))
                {
                    switch (menu.select)
                    {
                    case 0:
                        psp2chMenuNG(pixelsX, pixelsY);
                        break;
                    case 1:
                        sceNetApctlDisconnect();
                        break;
                    }
                    printBuf = pixels;
                    pgCopy(pixelsX, pixelsY);
                    framebuffer = sceGuSwapBuffers();
                    pgCopy(pixelsX, pixelsY);
                    printBuf = winPixels;
                }
                else if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.menuWinH.esc) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.menuWinV.esc))
                {
                    printBuf = pixels;
                    break;
                }
            }
            if (s2ch.tateFlag)
            {
                menuStr = s2ch.menuWinV.main;
            }
            else
            {
                menuStr = s2ch.menuWinH.main;
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

/****************
NG設定ウィンドウ
****************/
#define MENU_NG_WIDTH (80)
#define MENU_NG_HEIGHT (52)
#define MENU_NG_ITEM (4)
void psp2chMenuNG(int pixelsX, int pixelsY)
{
    const unsigned short text1[] = {0x004E,0x0047,0x767B,0x9332,0x3059,0x308B,0x540D,0x524D,0x3092,0x5165,0x529B,0x3057,0x3066,0x304F,0x3060,0x3055,0x3044,0};
    char* text2 = "NGネーム";
    const char* menuList[] = {"NG名前登録", "NG名前削除", "NGID削除"};
    static char* menuStr = "";
    int lineEnd;
    static S_2CH_SCREEN menu;
    int startX, startY, scrX, scrY;

    if (s2ch.tateFlag)
    {
        startX = (SCR_HEIGHT - MENU_NG_WIDTH) / 2;
        startY = (SCR_WIDTH - MENU_NG_HEIGHT) / 2;    }
    else
    {
        startX = (SCR_WIDTH - MENU_NG_WIDTH) / 2;
        startY = (SCR_HEIGHT - MENU_NG_HEIGHT) / 2;
    }
    scrX = MENU_NG_WIDTH;
    scrY = MENU_NG_HEIGHT;
    lineEnd = MENU_NG_ITEM;
    menu.count = 3;
    printBuf = pixels;
    pgCopy(pixelsX, pixelsY);
    framebuffer = sceGuSwapBuffers();
    pgCopy(pixelsX, pixelsY);
    printBuf = winPixels;
    while (s2ch.running)
    {
        if(sceCtrlPeekBufferPositive(&s2ch.pad, 1))
        {
            psp2chCursorSet(&menu, lineEnd, 0);
            if (s2ch.pad.Buttons != s2ch.oldPad.Buttons)
            {
                s2ch.oldPad = s2ch.pad;
                if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.menuWinH.ok) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.menuWinV.ok))
                {
                    switch (menu.select)
                    {
                    case 0: // NG name add
                        pgFillvram(s2ch.menuWinColor.bg, 0, 0, BUF_WIDTH, BUF_HEIGHT);
                        pgCopy(0,0);
                        framebuffer = sceGuSwapBuffers();
                        if (psp2chInputDialog(text1, text2) == 0 && keyWords[0])
                        {
                            psp2chNGAdd(ngNameFile, keyWords);
                        }
                        break;
                    case 1: // NG name del
                        psp2chNGDel(ngNameFile, pixelsX, pixelsY);
                        break;
                    case 2: // NG ID del
                        psp2chNGDel(ngIDFile, pixelsX, pixelsY);
                        break;
                    }
                    printBuf = pixels;
                    pgCopy(pixelsX, pixelsY);
                    framebuffer = sceGuSwapBuffers();
                    pgCopy(pixelsX, pixelsY);
                    printBuf = winPixels;
                }
                else if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.menuWinH.esc) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.menuWinV.esc))
                {
                    break;
                }
            }
            if (s2ch.tateFlag)
            {
                menuStr = s2ch.menuWinV.main;
            }
            else
            {
                menuStr = s2ch.menuWinH.main;
            }
            psp2chDrawMenu((char**)menuList, menu, startX, startY, scrX, scrY);
            pgCopyWindow(0, startX, startY, scrX, scrY);
            pgWindowFrame(startX, startY, startX + scrX, startY + scrY);
            pgMenuBar(menuStr);
            sceDisplayWaitVblankStart();
            framebuffer = sceGuSwapBuffers();
        }
    }
}

/****************
NGファイルがあればバッファを確保し
ファイル内容を読み込んで返す
****************/
char* psp2chGetNGBuf(const char* file, char* buf)
{
    SceUID fd;
    SceIoStat st;
    char path[256];
    int ret;

    sprintf(path, "%s/%s/%s", s2ch.cwDir, s2ch.logDir, file);
    ret = sceIoGetstat(path, &st);
    if (ret < 0)
    {
        return NULL;
    }
    buf = (char*)malloc(st.st_size + 1);
    if (buf == NULL)
    {
        memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
        strcpy(s2ch.mh.message, "memorry error\npsp2chNGDel() buf");
        pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
        sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
        return NULL;
    }
    fd = sceIoOpen(path, PSP_O_RDONLY, 0777);
    if (fd < 0)
    {
        free(buf);
        memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
        sprintf(s2ch.mh.message, "NG File open error\n%s", path);
        pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
        sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
        return NULL;
    }
    sceIoRead(fd, buf, st.st_size);
    sceIoClose(fd);
    buf[st.st_size] = '\0';
    return buf;
}

/****************
NG削除
****************/
#define MENU_NGLIST_WIDTH (200)
int psp2chNGDel(const char* file, int pixelsX, int pixelsY)
{
    SceUID fd;
    char path[256];
    char *buf, *p, *q, *menuStr;
    char** list;
    int i, lineEnd;
    static S_2CH_SCREEN menu;
    int startX, startY, scrX, scrY;

    if (s2ch.tateFlag)
    {
        startX = (SCR_HEIGHT - MENU_NGLIST_WIDTH) / 2;
        startY = (SCR_WIDTH - 400) / 2;
        scrY = 390;
        lineEnd = 30;
    }
    else
    {
        startX = (SCR_WIDTH - MENU_NGLIST_WIDTH) / 2;
        startY = (SCR_HEIGHT - 200) / 2;
        scrY = 195;
        lineEnd = 15;
    }
    scrX = MENU_NGLIST_WIDTH;
    printBuf = pixels;
    pgCopy(pixelsX, pixelsY);
    framebuffer = sceGuSwapBuffers();
    pgCopy(pixelsX, pixelsY);
    printBuf = winPixels;
    buf = NULL;
    buf = psp2chGetNGBuf(file, buf);
    if (buf == NULL)
    {
        return -1;
    }
    p= buf;
    menu.count = 0;
    while (*p)
    {
        if (*p++ == '\n')
        {
            menu.count++;
        }
    }
    list = (char**)malloc(sizeof(char*) * menu.count);
    if (list == NULL)
    {
        free(buf);
        memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
        sprintf(s2ch.mh.message, "memorry error\npsp2chNGDel() list");
        pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
        sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
        return -1;
    }
    p = buf;
    for (i = 0; i < menu.count; i++)
    {
        q = strchr(p, '\n');
        if (q == NULL)
        {
            break;
        }
        *q = '\0';
        list[i] = p;
        p = q + 1;
    }
    sprintf(path, "%s/%s/%s", s2ch.cwDir, s2ch.logDir, file);
    while (s2ch.running)
    {
        if(sceCtrlPeekBufferPositive(&s2ch.pad, 1))
        {
            psp2chCursorSet(&menu, lineEnd, 0);
            if (s2ch.pad.Buttons != s2ch.oldPad.Buttons)
            {
                s2ch.oldPad = s2ch.pad;
                if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.menuNGH.save) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.menuNGV.save))
                {
                    fd = sceIoOpen(path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
                    if (fd < 0)
                    {
                        free(list);
                        free(buf);
                        memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
                        sprintf(s2ch.mh.message, "NG File open error\n%s", path);
                        pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
                        sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
                        return -1;
                    }
                    for (i = 0; i < menu.count; i++)
                    {
                        sceIoWrite(fd, list[i], strlen(list[i]));
                        sceIoWrite(fd, "\n", 1);
                    }
                    sceIoClose(fd);
                    psp2chResCheckNG();
                    break;
                }
                else if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.menuNGH.esc) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.menuNGV.esc))
                {
                    break;
                }
                else if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.menuNGH.del) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.menuNGV.del))
                {
                    menu.count--;
                    for (i = menu.select; i < menu.count; i++)
                    {
                        list[i] = list[i + 1];
                    }
                }
            }
            if (s2ch.tateFlag)
            {
                menuStr = s2ch.menuNGV.main;
            }
            else
            {
                menuStr = s2ch.menuNGH.main;
            }
            psp2chDrawMenu(list, menu, startX, startY, scrX, scrY);
            pgCopyWindow(0, startX, startY, scrX, scrY);
            pgWindowFrame(startX, startY, startX + scrX, startY + scrY);
            pgMenuBar(menuStr);
            sceDisplayWaitVblankStart();
            framebuffer = sceGuSwapBuffers();
        }
    }
    free(list);
    free(buf);
    return 0;
}

/****************
NG登録
****************/
int psp2chNGAdd(const char* file, char* val)
{
    SceUID fd;
    char path[256];

    sprintf(path, "%s/%s/%s", s2ch.cwDir, s2ch.logDir, file);
    fd = sceIoOpen(path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_APPEND, 0777);
    if (fd < 0)
    {
        memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
        sprintf(s2ch.mh.message, "NG File open error\n%s", path);
        pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
        sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
        return -1;
    }
    sceIoWrite(fd, val, strlen(val));
    sceIoWrite(fd, "\n", 1);
    sceIoClose(fd);
    pgMenuBar("登録しました");
    sceDisplayWaitVblankStart();
    framebuffer = sceGuSwapBuffers();
    pgWaitVn(60);
    psp2chResCheckNG();
    return 0;
}

/****************
メニューウィンドウ描画
****************/
void psp2chDrawMenu(char** menuList, S_2CH_SCREEN menu, int x, int y, int width, int height)
{
    int i, start, lineEnd;

    s2ch.pgCursorX = x;
    s2ch.pgCursorY = y;
    lineEnd = height / LINE_PITCH;
    start = menu.start;
    if (start + lineEnd > menu.count)
    {
        start = menu.count - lineEnd;
    }
    if (start < 0)
    {
        start = 0;
    }
    pgFillvram(s2ch.menuWinColor.bg, x, y, width, height);
    for (i = start; i < start + lineEnd; i++)
    {
        if (i >= menu.count)
        {
            return;
        }
        if (i == menu.select)
        {
            pgFillvram(s2ch.menuWinColor.s_bg, x, s2ch.pgCursorY, width, LINE_PITCH);
            pgPrint(menuList[i], s2ch.menuWinColor.s_text, s2ch.menuWinColor.s_bg, x + width);
        }
        else
        {
            pgPrint(menuList[i], s2ch.menuWinColor.text, s2ch.menuWinColor.bg, x + width);
        }
        s2ch.pgCursorX = x;
        s2ch.pgCursorY += LINE_PITCH;
    }
}
