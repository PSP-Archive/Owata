/*
* $Id$
*/

#include "pspdialogs.h"
#include <stdio.h>
#include <malloc.h>
#include <time.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include "psp2ch.h"
#include "psp2chIta.h"
#include "psp2chThread.h"
#include "psp2chFavorite.h"
#include "psp2chRes.h"
#include "psp2chMenu.h"
#include "utf8.h"
#include "pg.h"
#include "intraFont.h"

extern int running; //main.c
extern char cwDir[256]; //main.c
extern unsigned long pgCursorX, pgCursorY; // pg.c
extern unsigned int list[512*512]; // pg.c
extern intraFont* jpn0; // pg.c
extern void* framebuffer; // pg.c
extern char* logDir; // psp2ch.c
extern int sel; // psp2ch.c
extern int tateFlag; // psp2ch.c
extern SceCtrlData pad; // psp2ch.c
extern SceCtrlData oldPad; // psp2ch.c
extern MESSAGE_HELPER mh; // psp2ch.c
extern S_2CH_ITA* itaList; // psp2chIta.c
extern S_2CH_SCREEN ita; // psp2chIta.c
extern S_2CH_RES* resList; // psp2chRes.c
extern int preLine; // psp2chRes.c
extern S_2CH_THREAD_COLOR threadColor; // psp2ch.c
extern S_2CH_FAVORITE* findList; // psp2chSearch.c
extern char keyWords[128]; //psp2ch.c

S_2CH_THREAD* threadList;
S_2CH_SCREEN thread;
int* threadSort = NULL;

/****************
 スレッド一覧表示
*****************/
int psp2chThread(void)
{
    static int scrollX = 0;
    static char* menuStr = "";
    int lineEnd, rMenu;

    if (threadList == NULL)
    {
        if (psp2chThreadList(ita.select) < 0)
        {
            sel = 0;
            return -1;
        }
        thread.start = 0;
        thread.select = 0;
    }
    if (tateFlag)
    {
        lineEnd = 35;
    }
    else
    {
        lineEnd = 20;
    }
    if(sceCtrlPeekBufferPositive(&pad, 1))
    {
        rMenu = psp2chCursorSet(&thread, lineEnd);
        if (rMenu)
        {
            menuStr = "　↑ : 先頭　　　↓ : 最後　　　○ : ソ\ート　　　△ : 検索　　　□ : 全板検索";
        }
        else
        {
            menuStr = "　○ : 決定　　　　　× : 戻る　　　　　△ : 更新　　　　　□ : お気に入り　　　R : メニュー切替";
        }
        if (pad.Buttons != oldPad.Buttons)
        {
            oldPad = pad;
            if (pad.Buttons & PSP_CTRL_SELECT)
            {
                tateFlag = (tateFlag) ? 0 : 1;
            }
            // STARTボタン
            else if(pad.Buttons & PSP_CTRL_START)
            {
                psp2chMenu(scrollX, 0);
            }
            else if(pad.Buttons & PSP_CTRL_CIRCLE)
            {
                if (rMenu)
                {
                    psp2chThreadSort();
                }
                else
                {
                    free(resList);
                    resList = NULL;
                    preLine = -2;
                    sel = 5;
                }
            }
            else if(pad.Buttons & PSP_CTRL_CROSS)
            {
                sel = 2;
                return 0;
            }
            else if(pad.Buttons & PSP_CTRL_TRIANGLE)
            {
                if (rMenu)
                {
                    if (psp2chThreadSearch() == 0 && keyWords[0])
                    {
                        psp2chSort(10);
                    }
                }
                else
                {
                    psp2chGetSubject(ita.select);
                    psp2chThreadList(ita.select);
                    thread.start = 0;
                    thread.select = 0;
                }
            }
            else if(pad.Buttons & PSP_CTRL_SQUARE)
            {
                if (rMenu)
                {
                    if (psp2chThreadSearch() == 0 && keyWords[0])
                    {
                        if (findList)
                        {
                            free(findList);
                            findList = NULL;
                        }
                        sel = 7;
                    }
                }
                else
                {
                    sel = 1;
                }
            }
        }scrollX = psp2chPadSet(scrollX);
        psp2chDrawThread(scrollX);
        pgCopy(scrollX, 0);
        pgMenuBar(menuStr);
        sceDisplayWaitVblankStart();
        framebuffer = sceGuSwapBuffers();
    }
    return 0;
}

/****************
メモステに保存されたsubject.txtを読み込んで
threadList構造体を作成
番号順にソート
*****************/
int psp2chThreadList(int ita)
{
    SceUID fd, dfd;
    SceIoStat st;
    SceIoDirent dir;
    char file[256];
    char line[256];
    char *buf, *p, *q;
    int i, dat;

    sprintf(file, "%s/%s/%s/subject.txt", cwDir, logDir, itaList[ita].title);
    i = sceIoGetstat(file, &st);
    if (i < 0)
    {
        if (psp2chGetSubject(ita) < 0)
        {
            return -1;
        }
        i = sceIoGetstat(file, &st);
        if (i< 0)
        {
            memset(&mh,0,sizeof(MESSAGE_HELPER));
            sprintf(mh.message, "File stat error\n%s", file);
            pspShowMessageDialog(&mh, DIALOG_LANGUAGE_AUTO);
            sceCtrlPeekBufferPositive(&oldPad, 1);
            return -1;
        }
    }
    buf = (char*)malloc(st.st_size + 1);
    if (buf == NULL)
    {
        memset(&mh,0,sizeof(MESSAGE_HELPER));
        strcpy(mh.message, "memorry error\npsp2chThreadList() buf");
        pspShowMessageDialog(&mh, DIALOG_LANGUAGE_AUTO);
        sceCtrlPeekBufferPositive(&oldPad, 1);
        return -1;
    }
    fd = sceIoOpen(file, PSP_O_RDONLY, 0777);
    if (fd < 0)
    {
        free(buf);
        memset(&mh,0,sizeof(MESSAGE_HELPER));
        sprintf(mh.message, "File open error\n%s", file);
        pspShowMessageDialog(&mh, DIALOG_LANGUAGE_AUTO);
        sceCtrlPeekBufferPositive(&oldPad, 1);
        return -1;
    }
    sceIoRead(fd, buf, st.st_size);
    sceIoClose(fd);
    buf[st.st_size] = '\0';
    thread.count = 0;
    p = buf;
    while (*p)
    {
        if (*p++ == '\n')
        {
            thread.count++;
        }
    }
    thread.count -= 2;
    threadList = (S_2CH_THREAD*)realloc(threadList, sizeof(S_2CH_THREAD) * thread.count);
    if (threadList == NULL )
    {
        free(buf);
        memset(&mh,0,sizeof(MESSAGE_HELPER));
        strcpy(mh.message, "memorry error\nThreadList");
        pspShowMessageDialog(&mh, DIALOG_LANGUAGE_AUTO);
        sceCtrlPeekBufferPositive(&oldPad, 1);
        return -1;
    }
    threadSort = (int*)realloc(threadSort, sizeof(int) * thread.count);
    if (threadSort == NULL)
    {
        free(buf);
        memset(&mh,0,sizeof(MESSAGE_HELPER));
        strcpy(mh.message, "memorry error\nThreadSort");
        pspShowMessageDialog(&mh, DIALOG_LANGUAGE_AUTO);
        sceCtrlPeekBufferPositive(&oldPad, 1);
        return -1;
    }
    thread.count = 0;
    q = buf;
    while (*q++ != '\n');
    while (*q++ != '\n');
    p = q;
    while(*q)
    {
        threadList[thread.count].id = thread.count;
        sscanf(p, "%d", &threadList[thread.count].dat);
        p = strchr(q, '>');
        p++;
        while (*q != '\n')
        {
            q++;
        }
        *q = '\0';
        q++;
        strcpy(line, p);
        p = strrchr(line, '(');
        *p = '\0';
        strcpy(threadList[thread.count].title, line);
        p++;
        sscanf(p, "%d", &threadList[thread.count].res);
        threadList[thread.count].old = 0;
        thread.count++;
        p = q;
    }
    free(buf);
    pgMenuBar("取得済みスレッドの検索中");
    sceDisplayWaitVblankStart();
    framebuffer = sceGuSwapBuffers();
    sprintf(file, "%s/%s/%s", cwDir, logDir, itaList[ita].title);
    dfd = sceIoDopen(file);
    if (dfd >= 0)
    {
        memset(&dir, 0, sizeof(dir)); // 初期化しないとreadに失敗する
        while (sceIoDread(dfd, &dir) > 0)
        {
            if (strstr(dir.d_name, ".idx"))
            {
                sscanf(dir.d_name, "%d", &dat);
                for (i = 0; i < thread.count; i++)
                {
                    if (threadList[i].dat == dat)
                    {
                        sprintf(file, "%s/%s/%s/%s", cwDir, logDir, itaList[ita].title, dir.d_name);
                        fd = sceIoOpen(file, PSP_O_RDONLY, 0777);
                        if (fd >= 0)
                        {
                            sceIoRead(fd, file, 128);
                            sceIoClose(fd);
                            p = strchr(file, '\n'); // Last-Modified
                            p++;
                            p =  strchr(p, '\n'); // ETag
                            p++;
                            p =  strchr(p, '\n'); // Range
                            p++;
                            p =  strchr(p, '\n'); // res.start
                            p++;
                            p =  strchr(p, '\n'); // res.select
                            p++;
                            sscanf(p, "%d", &threadList[i].old);
                        }
                        break;
                    }
                }
            }
        }
        sceIoDclose(dfd);
    }
    psp2chSort(1);
    return 0;
}

/****************
板のディレクトリがなければ作成
板のディレクトリにsubject.txtがあれば読み込む
subject.txtの取得日データをつけて2chにアクセス
更新されていれば新しいデータを取得して保存
なければ普通に取得して保存
*****************/
int psp2chGetSubject(int ita)
{
    int ret, mySocket, contentLength;
    HTTP_HEADERS resHeader;
    SceUID fd;
    char path[256];
    char buf[256];
    char lastModified[32];
    char eTag[32];
    char *p, *q;

    // Make ita directory
    sprintf(path, "%s/%s/%s", cwDir, logDir, itaList[ita].title);
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
    // check ita subject.txt
    strcat(path, "/subject.txt");
    fd = sceIoOpen(path, PSP_O_RDONLY, 0777);
    if (fd < 0)
    {
        buf[0] = '\0';
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
        sprintf(buf, "If-Modified-Since: %s\r\nIf-None-Match: %s\r\n", lastModified, eTag);
    }
    sprintf(path, "%s/subject.txt", itaList[ita].dir);
    mySocket = psp2chRequest(itaList[ita].host, path, buf);
    if (mySocket < 0)
    {
        return mySocket;
    }
    ret = psp2chGetStatusLine(mySocket);
    switch(ret)
    {
        case 200: // OK
            break;
        case 301:
            memset(&mh,0,sizeof(MESSAGE_HELPER));
            mh.options = PSP_UTILITY_MSGDIALOG_OPTION_TEXT | PSP_UTILITY_MSGDIALOG_OPTION_YESNO_BUTTONS;
            strcpy(mh.message, TEXT_7);
            pspShowMessageDialog(&mh, DIALOG_LANGUAGE_AUTO);
            sceCtrlPeekBufferPositive(&oldPad, 1);
            if (mh.buttonPressed == PSP_UTILITY_MSGDIALOG_RESULT_YES)
            {
                psp2chGetMenu();
            }
            return -1;
        case 304: // Not modified
            psp2chCloseSocket(mySocket);
            return 0;
        default:
            memset(&mh,0,sizeof(MESSAGE_HELPER));
            sprintf(mh.message, "HTTP error\nhost %s path %s\nStatus code %d", itaList[ita].host, path, ret);
            pspShowMessageDialog(&mh, DIALOG_LANGUAGE_AUTO);
            sceCtrlPeekBufferPositive(&oldPad, 1);
            psp2chCloseSocket(mySocket);
            return -1;
    }
    // Receive and Save subject
    sprintf(path, "%s/%s/%s/subject.txt", cwDir, logDir, itaList[ita].title);
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
    contentLength = psp2chGetHttpHeaders(mySocket, &resHeader);
    if (contentLength <= 0)
    {
        psp2chCloseSocket(mySocket);
        sceIoClose(fd);
        return -1;
    }
    sceIoWrite(fd, resHeader.Last_Modified, strlen(resHeader.Last_Modified));
    sceIoWrite(fd, resHeader.ETag, strlen(resHeader.ETag));
    sprintf(buf, "http://%s/%s/subject.txt からデータを転送しています...", itaList[ita].host, itaList[ita].dir);
    pgMenuBar(buf);
    sceDisplayWaitVblankStart();
    framebuffer = sceGuSwapBuffers();
    while((ret = recv(mySocket, buf, sizeof(buf), 0)) > 0)
    {
        sceIoWrite(fd, buf, ret);
    }
    psp2chCloseSocket(mySocket);
    sceIoClose(fd);
    return 0;
}

/****************
スレッドをソートする
threadSort配列にソートデータが入る
sort:
0=既読スレ順
1=番号順
2=作成日(降順)
3=作成日(昇順)
10=検索単語順
*****************/
void psp2chSort(int sort)
{
    static int s = 1;
    int i, j, tmp;
    char haystack[128];
    char *p;

    if (sort >= 0)
    {
        s = sort;
    }
    switch (s)
    {
    case 0:
        for (i = 0, j = 0; i < thread.count; i++)
        {
            if (threadList[i].old > 0)
            {
                threadSort[j] = i;
                j++;
            }
        }
        for (i = 0; i < thread.count; i++)
        {
            if (threadList[i].old == 0)
            {
                threadSort[j] = i;
                j++;
            }
        }
        break;
    case 1:
        for (i = 0; i < thread.count; i++)
        {
            threadSort[i] = i;
        }
        break;
    case 2:
        for (i = 0; i < thread.count; i++)
        {
            threadSort[i] = i;
        }
        for (i = 0; i < thread.count-1; i++)
        {
            for (j = i; j < thread.count; j++)
            {
                if (threadList[threadSort[j]].dat > threadList[threadSort[i]].dat)
                {
                    tmp = threadSort[j];
                    threadSort[j] = threadSort[i];
                    threadSort[i] = tmp;
                }
            }
        }
        break;
    case 3:
        for (i = 0; i < thread.count; i++)
        {
            threadSort[i] = i;
        }
        for (i = 0; i < thread.count-1; i++)
        {
            for (j = i; j < thread.count; j++)
            {
                if (threadList[threadSort[j]].dat < threadList[threadSort[i]].dat)
                {
                    tmp = threadSort[j];
                    threadSort[j] = threadSort[i];
                    threadSort[i] = tmp;
                }
            }
        }
        break;
    case 10:
        p = keyWords;
        if (*p >= 'a' && *p <= 'z')
        {
            *p -= ('a' - 'A');
        }
        p++;
        while (*p)
        {
            if (*(p-1) > 0 && *p >= 'a' && *p <= 'z')
            {
                *p -= ('a' - 'A');
            }
            p++;
        }
        for (i = 0, j = 0; i < thread.count; i++)
        {
            strcpy(haystack, threadList[i].title);
            p = haystack;
            if (*p >= 'a' && *p <= 'z')
            {
                *p -= ('a' - 'A');
            }
            p++;
            while (*p)
            {
                if (*(p-1) > 0 && *p >= 'a' && *p <= 'z')
                {
                    *p -= ('a' - 'A');
                }
                p++;
            }
            if (strstr(haystack, keyWords))
            {
                threadSort[j] = i;
                j++;
            }
        }
        for (i = 0; i < thread.count; i++)
        {
            strcpy(haystack, threadList[i].title);
            p = haystack;
            if (*p >= 'a' && *p <= 'z')
            {
                *p -= ('a' - 'A');
            }
            p++;
            while (*p)
            {
                if (*(p-1) > 0 && *p >= 'a' && *p <= 'z')
                {
                    *p -= ('a' - 'A');
                }
                p++;
            }
            if (strstr(haystack, keyWords) == NULL)
            {
                threadSort[j] = i;
                j++;
            }
        }
        break;
    }
}

/****************
ソート用ダイアログ表示
*****************/
void psp2chThreadSort(void)
{
    const unsigned short text1[] = {0x3069,0x306E,0x9805,0x76EE,0x3067,0x30BD,0x30FC,0x30C8,0x3057,0x307E,0x3059,0x304B,0};
    const unsigned short text2[] = {0x65E2,0x8AAD,0x30B9,0x30EC,0}; // 既読スレ
    const unsigned short text3[] = {0x756A,0x53F7,0x9806,0}; // 番号順
    const unsigned short text4[] = {0x4F5C,0x6210,0x65E5,0x0028,0x964D,0x9806,0x0029,0}; // 作成日(降順)
    const unsigned short text5[] = {0x4F5C,0x6210,0x65E5,0x0028,0x6607,0x9806,0x0029,0}; // 作成日(昇順)
    const unsigned short* text[] = {text2, text3, text4, text5};
    int i, select = 0;

    while (running)
    {
        if(sceCtrlPeekBufferPositive(&pad, 1))
        {
            if (pad.Buttons != oldPad.Buttons)
            {
                oldPad = pad;
                if(pad.Buttons & PSP_CTRL_UP)
                {
                    if (select)
                    {
                        select--;
                    }
                }
                if(pad.Buttons & PSP_CTRL_DOWN)
                {
                    if (select < 3)
                    {
                        select++;
                    }
                }
                if(pad.Buttons & PSP_CTRL_CIRCLE)
                {
                    break;
                }
                if(pad.Buttons & PSP_CTRL_CROSS)
                {
                    return;
                }
            }
            sceGuStart(GU_DIRECT, list);
            sceGuClearColor(BLUE);
            sceGuClearDepth(0);
            sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
            pgCursorX = 240;
            pgCursorY =  77;
            intraFontSetStyle(jpn0, 1.0f, YELLOW, BLUE, INTRAFONT_ALIGN_CENTER);
            intraFontPrintUCS2(jpn0, pgCursorX, pgCursorY, text1);
            pgCursorX = 240;
            pgCursorY += 25;
            for (i = 0; i < 4; i++)
            {
                if (select == i)
                {
                    intraFontSetStyle(jpn0, 0.9f, WHITE, BLACK, INTRAFONT_ALIGN_CENTER);
                    intraFontPrintUCS2(jpn0, pgCursorX, pgCursorY, text[i]);
                }
                else
                {
                    intraFontSetStyle(jpn0, 0.9f, GRAY, 0, INTRAFONT_ALIGN_CENTER);
                    intraFontPrintUCS2(jpn0, pgCursorX, pgCursorY, text[i]);
                }
                pgCursorX = 240;
                pgCursorY += 20;
            }
            sceGuFinish();
            sceGuSync(0,0);
            sceDisplayWaitVblankStart();
            framebuffer = sceGuSwapBuffers();
        }
    }
    return psp2chSort(select);
}

/****************
検索ダイアログ表示
*****************/
int psp2chThreadSearch(void)
{
    const unsigned short text1[] = {0x691C,0x7D22,0x6587,0x5B57,0x5217,0x3092,0x5165,0x529B,0x3057,0x3066,0x304F,0x3060,0x3055,0x3044,0};
    char* text2 = "検索文字列";

    return psp2chInputDialog(text1, text2);
}

/****************
スレ一覧の描画ルーチン
*****************/
void psp2chDrawThread(int scrollX)
{
    int start;
    int i;
    int lineEnd, scrW, scrH, resCount;

    if (tateFlag)
    {
        lineEnd = 35;
        scrW = SCR_HEIGHT;
        scrH = SCR_WIDTH;
        resCount = scrW - 50 + scrollX;
    }
    else
    {
        lineEnd = 20;
        scrW = SCR_WIDTH;
        scrH = SCR_HEIGHT;
        resCount = THREAD_RES + scrollX;
    }
    start = thread.start;
    if (start + lineEnd > thread.count)
    {
        start = thread.count - lineEnd;
    }
    if (start < 0)
    {
        start = 0;
    }
    pgFillvram(threadColor.bg, 0, 0, BUF_WIDTH, BUF_HEIGHT);
    pgCursorY = 0;
    for (i = start; i < start + lineEnd; i++)
    {
        if (i >= thread.count)
        {
            return;
        }
        pgCursorX = 0;
        if (i == thread.select)
        {
            pgFillvram(threadColor.s_bg, 0, pgCursorY, BUF_WIDTH, LINE_PITCH);
            pgPrintNumber(threadList[threadSort[i]].id + 1, threadColor.s_num, threadColor.s_bg);
        }
        else
        {
            pgPrintNumber(threadList[threadSort[i]].id + 1, threadColor.num, threadColor.bg);
        }
        pgCursorX = THREAD_ID;
        if (i == thread.select)
        {
            if (threadList[threadSort[i]].old > 0)
            {
                pgPrint(threadList[threadSort[i]].title, threadColor.s_text2, threadColor.s_bg, resCount+12);
            }
            else
            {
                pgPrint(threadList[threadSort[i]].title, threadColor.s_text1, threadColor.s_bg, resCount+12);
            }
        }
        else
        {
            if (threadList[threadSort[i]].old > 0)
            {
                pgPrint(threadList[threadSort[i]].title, threadColor.text2, threadColor.bg, resCount+12);
            }
            else
            {
                pgPrint(threadList[threadSort[i]].title, threadColor.text1, threadColor.bg, resCount+12);
            }
        }
        pgCursorX = resCount;
        if (i == thread.select)
        {
            pgPrintNumber(threadList[threadSort[i]].res, threadColor.s_count1, threadColor.s_bg);
            if (threadList[threadSort[i]].old > 0)
            {
                pgCursorX += 2;
                pgPrintNumber(threadList[threadSort[i]].old, threadColor.s_count2, threadColor.s_bg);
            }
        }
        else
        {
            pgPrintNumber(threadList[threadSort[i]].res, threadColor.count1, threadColor.bg);
            if (threadList[threadSort[i]].old > 0)
            {
                pgCursorX += 2;
                pgPrintNumber(threadList[threadSort[i]].old, threadColor.count2, threadColor.bg);
            }
        }
        pgCursorY += LINE_PITCH;
    }
}
