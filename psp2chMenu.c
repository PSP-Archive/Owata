/*
* $Id$
*/

#include <stdio.h>
#include <malloc.h>
#include <pspdebug.h>
#include "pg.h"
#include "psp2ch.h"
#include "psp2chIni.h"
#include "psp2chMenu.h"
#include "psp2chRes.h"

extern S_2CH s2ch; // psp2ch.c
extern char keyWords[128]; //psp2ch.c
extern unsigned int pixels[BUF_WIDTH*BUF_HEIGHT]; // pg.c
extern unsigned int winPixels[BUF_WIDTH*BUF_HEIGHT]; // pg.c
extern unsigned int* printBuf; // pg.c
extern const char *sBtnH[]; // psp2chRes.c
extern const char *sBtnV[]; // psp2chRes.c

const char* ngNameFile = "ngname.txt";
const char* ngIDFile = "ngid.txt";
const char* ngWordFile = "ngword.txt";
const char* ngMailFile = "ngmail.txt";
const char* colorFile = "color.ini";

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
    int index1, index2;
    int i, tmp;

    getIndex(s2ch.menuWinH.ok, index1);
    getIndex(s2ch.menuWinH.esc, index2);
    sprintf(s2ch.menuWinH.main, "　%s : 決定　　　　　%s : 戻る",
            sBtnH[index1], sBtnH[index2]);

    getIndex(s2ch.menuWinV.ok, index1);
    getIndex(s2ch.menuWinV.esc, index2);
    sprintf(s2ch.menuWinV.main, "　%s : 決定　　　　　%s : 戻る",
            sBtnV[index1], sBtnV[index2]);

    getIndex(s2ch.menuNGH.del, index1);
    getIndex(s2ch.menuNGH.esc, index2);
    sprintf(s2ch.menuNGH.main, "　%s : 削除　　　　%s : 戻る",
            sBtnH[index1], sBtnH[index2]);

    getIndex(s2ch.menuNGV.del, index1);
    getIndex(s2ch.menuNGV.esc, index2);
    sprintf(s2ch.menuNGV.main, "　%s : 削除　　　　%s : 戻る",
            sBtnV[index1], sBtnV[index2]);
}

/****************
メニュー選択ウィンドウ
****************/
#define MENU_WIDTH (80)
#define MENU_ITEM (4)
#define MENU_HEIGHT (MENU_ITEM * LINE_PITCH)
int psp2chMenu(S_SCROLLBAR* bar)
{
    const char* menuList[] = {"NG 設定", "LAN 切断", "フォント変更", "カラー"};
    static char* menuStr = "";
    int lineEnd, change;
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
    menu.count = MENU_ITEM;
    printBuf = winPixels;
    if (s2ch.tateFlag)
    {
        menuStr = s2ch.menuWinV.main;
    }
    else
    {
        menuStr = s2ch.menuWinH.main;
    }
	psp2chDrawMenu((char**)menuList, menu, startX, startY, scrX, scrY);
	pgPrintMenuBar(menuStr);
    printBuf = pixels;
    pgCopy(s2ch.viewX, s2ch.viewY);
	if (bar)
	{
		pgScrollbar(bar, s2ch.resBarColor);
	}
    printBuf = winPixels;
    pgCopyWindow(0, startX, startY, scrX, scrY);
	pgWindowFrame(startX, startY, startX + scrX, startY + scrY);
    pgCopyMenuBar();
    sceDisplayWaitVblankStart();
    framebuffer = sceGuSwapBuffers();
    while (s2ch.running)
    {
        if(sceCtrlPeekBufferPositive(&s2ch.pad, 1))
        {
            psp2chCursorSet(&menu, lineEnd, 0, &change);
            if (s2ch.pad.Buttons != s2ch.oldPad.Buttons)
            {
                s2ch.oldPad = s2ch.pad;
                if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.menuWinH.ok) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.menuWinV.ok))
                {
                    switch (menu.select)
                    {
                    case 0:
                        psp2chMenuNG(bar);
                        break;
                    case 1:
                        sceNetApctlDisconnect();
                        break;
                    case 2:
                        psp2chMenuFont(bar);
                        scrY = MENU_HEIGHT;
                        break;
                    case 3:
                        psp2chMenuColor(bar);
                        break;
                    }
					change = 1;
                }
                else if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.menuWinH.esc) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.menuWinV.esc))
                {
                    printBuf = pixels;
                    break;
                }
            }
			if (change)
			{
				psp2chDrawMenu((char**)menuList, menu, startX, startY, scrX, scrY);
				printBuf = pixels;
				pgCopy(s2ch.viewX, s2ch.viewY);
				if (bar)
				{
					pgScrollbar(bar, s2ch.resBarColor);
				}
				printBuf = winPixels;
				pgCopyWindow(0, startX, startY, scrX, scrY);
				pgWindowFrame(startX, startY, startX + scrX, startY + scrY);
				pgCopyMenuBar();
				sceDisplayWaitVblankStart();
				framebuffer = sceGuSwapBuffers();
			}
        }
    }
    return 0;
}

/****************
NG設定ウィンドウ
****************/
#define MENU_NG_WIDTH (80)
#define MENU_NG_ITEM (7)
#define MENU_NG_HEIGHT (MENU_NG_ITEM * LINE_PITCH)
void psp2chMenuNG(S_SCROLLBAR* bar)
{
    const unsigned short title1[] = {0x004E,0x0047,0x767B,0x9332,0x3059,0x308B,0x540D,0x524D,0x3092,0x5165,0x529B,0x3057,0x3066,0x304F,0x3060,0x3055,0x3044,0};
    const unsigned short title2[] = {0x004E,0x0047,0x767B,0x9332,0x3059,0x308B,0x5358,0x8A9E,0x3092,0x5165,0x529B,0x3057,0x3066,0x304F,0x3060,0x3055,0x3044,0};
    const unsigned short title3[] = {0x004E,0x0047,0x767B,0x9332,0x3059,0x308B,0x30A2,0x30C9,0x30EC,0x30B9,0x3092,0x5165,0x529B,0x3057,0x3066,0x304F,0x3060,0x3055,0x3044,0};
    char* text = "NGネーム";
    const char* menuList[] = {"NG名前登録", "NG名前削除", "NGID削除", "NGワード登録", "NGワード削除", "NGメール登録", "NGメール削除"};
    static char* menuStr = "";
    int lineEnd, change;
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
    menu.count = MENU_NG_ITEM;
    if (s2ch.tateFlag)
    {
        menuStr = s2ch.menuWinV.main;
    }
    else
    {
        menuStr = s2ch.menuWinH.main;
    }
	psp2chDrawMenu((char**)menuList, menu, startX, startY, scrX, scrY);
	pgPrintMenuBar(menuStr);
    printBuf = pixels;
    pgCopy(s2ch.viewX, s2ch.viewY);
	if (bar)
	{
		pgScrollbar(bar, s2ch.resBarColor);
	}
    printBuf = winPixels;
    pgCopyWindow(0, startX, startY, scrX, scrY);
    pgWindowFrame(startX, startY, startX + scrX, startY + scrY);
    pgCopyMenuBar();
    sceDisplayWaitVblankStart();
    framebuffer = sceGuSwapBuffers();
    while (s2ch.running)
    {
        if(sceCtrlPeekBufferPositive(&s2ch.pad, 1))
        {
            psp2chCursorSet(&menu, lineEnd, 0, &change);
            if (s2ch.pad.Buttons != s2ch.oldPad.Buttons)
            {
                s2ch.oldPad = s2ch.pad;
                if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.menuWinH.ok) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.menuWinV.ok))
                {
                    switch (menu.select)
                    {
                    case 0: // NG name add
                        if (psp2chInputDialog(title1, text) == 0 && keyWords[0])
                        {
                            psp2chNGAdd(ngNameFile, keyWords);
                        }
						printBuf = winPixels;
                        break;
                    case 1: // NG name del
                        psp2chNGDel(ngNameFile, bar);
                        break;
                    case 2: // NG ID del
                        psp2chNGDel(ngIDFile, bar);
                        break;
                    case 3: // NG word add
                        if (psp2chInputDialog(title2, text) == 0 && keyWords[0])
                        {
                            psp2chNGAdd(ngWordFile, keyWords);
                        }
						printBuf = winPixels;
                        break;
                    case 4: // NG word del
                        psp2chNGDel(ngWordFile, bar);
                        break;
                    case 5: // NG mail add
                        if (psp2chInputDialog(title3, text) == 0 && keyWords[0])
                        {
                            psp2chNGAdd(ngMailFile, keyWords);
                        }
						printBuf = winPixels;
                        break;
                    case 6: // NG mail del
                        psp2chNGDel(ngMailFile, bar);
                        break;
                    }
					change = 1;
                }
                else if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.menuWinH.esc) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.menuWinV.esc))
                {
                    break;
                }
            }
			if (change)
			{
	            psp2chDrawMenu((char**)menuList, menu, startX, startY, scrX, scrY);
				printBuf = pixels;
				pgCopy(s2ch.viewX, s2ch.viewY);
				if (bar)
				{
					pgScrollbar(bar, s2ch.resBarColor);
				}
				printBuf = winPixels;
				pgCopyWindow(0, startX, startY, scrX, scrY);
				pgWindowFrame(startX, startY, startX + scrX, startY + scrY);
				pgCopyMenuBar();
				sceDisplayWaitVblankStart();
				framebuffer = sceGuSwapBuffers();
			}
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
        psp2chErrorDialog("memorry error\npsp2chNGDel() buf");
        return NULL;
    }
    fd = sceIoOpen(path, PSP_O_RDONLY, 0777);
    if (fd < 0)
    {
        free(buf);
        psp2chErrorDialog("NG File open error\n%s", path);
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
int psp2chNGDel(const char* file, S_SCROLLBAR* bar)
{
    SceUID fd;
    char path[256];
    char *buf, *p, *q, *menuStr;
    char** list;
    int i, lineEnd, change;
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
        psp2chErrorDialog("memorry error\npsp2chNGDel() list");
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
    if (s2ch.tateFlag)
    {
        menuStr = s2ch.menuNGV.main;
    }
    else
    {
        menuStr = s2ch.menuNGH.main;
    }
	psp2chDrawMenu(list, menu, startX, startY, scrX, scrY);
	pgPrintMenuBar(menuStr);
	printBuf = pixels;
	pgCopy(s2ch.viewX, s2ch.viewY);
	if (bar)
	{
		pgScrollbar(bar, s2ch.resBarColor);
	}
	printBuf = winPixels;
    pgCopyWindow(0, startX, startY, scrX, scrY);
    pgWindowFrame(startX, startY, startX + scrX, startY + scrY);
    pgCopyMenuBar();
    sceDisplayWaitVblankStart();
    framebuffer = sceGuSwapBuffers();
    while (s2ch.running)
    {
        if(sceCtrlPeekBufferPositive(&s2ch.pad, 1))
        {
            psp2chCursorSet(&menu, lineEnd, 0, &change);
            if (s2ch.pad.Buttons != s2ch.oldPad.Buttons)
            {
                s2ch.oldPad = s2ch.pad;
                if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.menuNGH.esc) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.menuNGV.esc))
                {
                    fd = sceIoOpen(path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
                    if (fd < 0)
                    {
                        free(list);
                        free(buf);
                        sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
                        psp2chErrorDialog("NG File open error\n%s", path);
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
                else if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.menuNGH.del) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.menuNGV.del))
                {
                    menu.count--;
                    for (i = menu.select; i < menu.count; i++)
                    {
                        list[i] = list[i + 1];
                    }
                }
				change = 1;
            }
			if (change)
			{
				psp2chDrawMenu(list, menu, startX, startY, scrX, scrY);
				printBuf = pixels;
				pgCopy(s2ch.viewX, s2ch.viewY);
				if (bar)
				{
					pgScrollbar(bar, s2ch.resBarColor);
				}
				printBuf = winPixels;
				pgCopyWindow(0, startX, startY, scrX, scrY);
				pgWindowFrame(startX, startY, startX + scrX, startY + scrY);
				pgCopyMenuBar();
				sceDisplayWaitVblankStart();
				framebuffer = sceGuSwapBuffers();
			}
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
        psp2chErrorDialog("NG File open error\n%s", path);
        return -1;
    }
    sceIoWrite(fd, val, strlen(val));
    sceIoWrite(fd, "\n", 1);
    sceIoClose(fd);
    pgPrintMenuBar("登録しました");
	pgCopyMenuBar();
    sceDisplayWaitVblankStart();
    framebuffer = sceGuSwapBuffers();
    pgWaitVn(50);
    psp2chResCheckNG();
    return 0;
}

void psp2chMenuFontSet(int select)
{
    if (s2ch.font.set == NULL)
    {
        return;
    }
    if (select >= s2ch.font.count)
    {
        return;
    }
    sscanf(s2ch.font.set[select], "%s %s %s %d %d",
                    s2ch.font.name,
                    s2ch.font.fileA,
                    s2ch.font.fileJ,
                    &s2ch.font.height,
                    &s2ch.font.pitch
    );
    s2ch.font.select = select;
}

/****************
フォント設定ウィンドウ
****************/
#define MENU_FONT_WIDTH (160)
#define MENU_FONT_ITEM (5)
void psp2chMenuFont(S_SCROLLBAR* bar)
{
    char** menuList;
    char* menuStr;
    int lineEnd = MENU_FONT_ITEM, change;
    static S_2CH_SCREEN menu;
    int i, startX, startY, scrX, scrY;

    if (s2ch.font.set == NULL)
    {
        return;
    }
    menuList = (char**)malloc(sizeof(char*) * s2ch.font.count);
    for (i = 0; i < s2ch.font.count; i++)
    {
        menuList[i] = (char*)malloc(sizeof(char) * 32);
        sscanf(s2ch.font.set[i], "%s", menuList[i]);
    }
    if (s2ch.tateFlag)
    {
        startX = (SCR_HEIGHT - MENU_FONT_WIDTH) / 2;
        startY = (SCR_WIDTH - LINE_PITCH * lineEnd) / 2;
    }
    else
    {
        startX = (SCR_WIDTH - MENU_FONT_WIDTH) / 2;
        startY = (SCR_HEIGHT - LINE_PITCH * lineEnd) / 2;
    }
    scrX = MENU_FONT_WIDTH;
    scrY = LINE_PITCH * lineEnd;
    menu.start = 0;
    menu.count = s2ch.font.count;
    menu.select = s2ch.font.select;
    if (menu.select >= MENU_FONT_ITEM)
    {
        menu.start = menu.select - MENU_FONT_ITEM + 1;
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
    pgPrintMenuBar(menuStr);
	printBuf = pixels;
	pgCopy(s2ch.viewX, s2ch.viewY);
	if (bar)
	{
		pgScrollbar(bar, s2ch.resBarColor);
	}
	printBuf = winPixels;
    pgCopyWindow(0, startX, startY, scrX, scrY);
    pgWindowFrame(startX, startY, startX + scrX, startY + scrY);
	pgCopyMenuBar();
    sceDisplayWaitVblankStart();
    framebuffer = sceGuSwapBuffers();
    while (s2ch.running)
    {
        if(sceCtrlPeekBufferPositive(&s2ch.pad, 1))
        {
            psp2chCursorSet(&menu, lineEnd, 0, &change);
            if (s2ch.pad.Buttons != s2ch.oldPad.Buttons)
            {
                s2ch.oldPad = s2ch.pad;
                if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.menuWinH.ok) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.menuWinV.ok))
                {
                    psp2chMenuFontSet(menu.select);
                    pgExtraFontInit();
					psp2chSetFontParam();
                    scrY = LINE_PITCH * lineEnd;
					change = 1;
                }
                else if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.menuWinH.esc) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.menuWinV.esc))
                {
					psp2chSetFontParam();
					psp2chSetBarParam();
                    break;
                }
            }
			if (change)
			{
	            psp2chDrawMenu((char**)menuList, menu, startX, startY, scrX, scrY);
				printBuf = pixels;
				pgCopy(s2ch.viewX, s2ch.viewY);
				if (bar)
				{
					pgScrollbar(bar, s2ch.resBarColor);
				}
				printBuf = winPixels;
				pgCopyWindow(0, startX, startY, scrX, scrY);
				pgWindowFrame(startX, startY, startX + scrX, startY + scrY);
				pgCopyMenuBar();
				sceDisplayWaitVblankStart();
				framebuffer = sceGuSwapBuffers();
			}
        }
    }
    for (i = 0; i < s2ch.font.count; i++)
    {
        free(menuList[i]);
    }
    free(menuList);
}

/****************
カラー設定ウィンドウ
****************/
#define MENU_COLOR_WIDTH (100)
#define MENU_COLOR_ITEM (5)
int psp2chMenuColor(S_SCROLLBAR* bar)
{
    SceUID fd;
    SceIoDirent dir;
    char path[256];
    char** menuList;
    char* menuStr;
    int lineEnd = MENU_COLOR_ITEM, change;
    static S_2CH_SCREEN menu;
    int i, startX, startY, scrX, scrY;

    sprintf(path, "%s/%s", s2ch.cwDir, s2ch.colorDir);
    if ((fd = sceIoDopen(path)) < 0)
    {
        psp2chErrorDialog("Dopen error\n%s", path);
        return -1;
    }
    menu.count = 0;
    memset(&dir, 0, sizeof(dir)); // 初期化しないとreadに失敗する
    while (sceIoDread(fd, &dir) > 0)
    {
        if (dir.d_stat.st_attr & FIO_SO_IFDIR)
        {
            continue;
        }
        menu.count++;
    }
    sceIoDclose(fd);
    menuList = (char**)malloc(sizeof(char*) * menu.count);
    if (menuList == NULL )
    {
        psp2chErrorDialog("memorry error\nmenuColorList");
        return -1;
    }
    if ((fd = sceIoDopen(path)) < 0)
    {
        free(menuList);
        psp2chErrorDialog("Dopen error2\n%s", path);
        return -1;
    }
    menuList[0] = "デフォルト";
    i = 1;
    memset(&dir, 0, sizeof(dir));
    while (sceIoDread(fd, &dir) > 0)
    {
        if ((dir.d_stat.st_attr & FIO_SO_IFDIR) || stricmp(dir.d_name, colorFile) == 0)
        {
            continue;
        }
        menuList[i] = (char*)malloc(strlen(dir.d_name) + 1);
        strcpy(menuList[i], dir.d_name);
        i++;
        if (i >= menu.count)
        {
            break;
        }
    }
    sceIoDclose(fd);
    if (s2ch.tateFlag)
    {
        startX = (SCR_HEIGHT - MENU_COLOR_WIDTH) / 2;
        startY = (SCR_WIDTH - LINE_PITCH * lineEnd) / 2;
    }
    else
    {
        startX = (SCR_WIDTH - MENU_COLOR_WIDTH) / 2;
        startY = (SCR_HEIGHT - LINE_PITCH * lineEnd) / 2;
    }
    scrX = MENU_COLOR_WIDTH;
    scrY = LINE_PITCH * lineEnd;
    menu.start = 0;
    menu.select = 0;
    if (menu.select >= MENU_COLOR_ITEM)
    {
        menu.start = menu.select - MENU_COLOR_ITEM + 1;
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
    pgPrintMenuBar(menuStr);
	printBuf = winPixels;
    pgCopyWindow(0, startX, startY, scrX, scrY);
    pgWindowFrame(startX, startY, startX + scrX, startY + scrY);
	pgCopyMenuBar();
    sceDisplayWaitVblankStart();
    framebuffer = sceGuSwapBuffers();
    while (s2ch.running)
    {
        if(sceCtrlPeekBufferPositive(&s2ch.pad, 1))
        {
            psp2chCursorSet(&menu, lineEnd, 0, &change);
            if (s2ch.pad.Buttons != s2ch.oldPad.Buttons)
            {
                s2ch.oldPad = s2ch.pad;
                if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.menuWinH.ok) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.menuWinV.ok))
                {
                    if (menu.select == 0)
                    {
                        psp2chIniSetColor(NULL);
                    }
                    else
                    {
                        psp2chIniSetColor(menuList[menu.select]);
                    }
					change = 1;
                }
                else if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.menuWinH.esc) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.menuWinV.esc))
                {
                    break;
                }
            }
			if (change)
			{
	            psp2chDrawMenu((char**)menuList, menu, startX, startY, scrX, scrY);
				printBuf = pixels;
				pgCopy(s2ch.viewX, s2ch.viewY);
				if (bar)
				{
					pgScrollbar(bar, s2ch.resBarColor);
				}
				printBuf = winPixels;
				pgCopyWindow(0, startX, startY, scrX, scrY);
				pgWindowFrame(startX, startY, startX + scrX, startY + scrY);
				pgCopyMenuBar();
				sceDisplayWaitVblankStart();
				framebuffer = sceGuSwapBuffers();
			}
        }
    }
    for (i = 1; i < menu.count; i++)
    {
        free(menuList[i]);
    }
    free(menuList);
	pgCursorColorSet();
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
    pgFillvram(s2ch.menuWinColor.bg, x, y, width, height, 2);
    for (i = start; i < start + lineEnd; i++)
    {
        if (i >= menu.count)
        {
            return;
        }
        if (i == menu.select)
        {
            pgFillvram(s2ch.menuWinColor.s_bg, x, s2ch.pgCursorY, width, LINE_PITCH, 2);
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
