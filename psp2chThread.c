/*
* $Id$
*/

#include <stdio.h>
#include <malloc.h>
#include <time.h>
#include <pspdebug.h>
#include "psp2ch.h"
#include "psp2chNet.h"
#include "psp2chIta.h"
#include "psp2chThread.h"
#include "psp2chFavorite.h"
#include "psp2chRes.h"
#include "psp2chMenu.h"
#include "utf8.h"
#include "pg.h"
#include "intraFont.h"

extern S_2CH s2ch; // psp2ch.c
extern unsigned int list[512*512]; // pg.c
extern intraFont* jpn0; // pg.c
extern int preLine; // psp2chRes.c
extern char keyWords[128]; //psp2ch.c
extern const char *sBtnH[]; // psp2chRes.c
extern const char *sBtnV[]; // psp2chRes.c

int* threadSort = NULL;

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

void psp2chThreadSetMenuString(void)
{
    int index1, index2, index3, index4, index5;
    int i, tmp;

    getIndex(s2ch.thH.ok, index1);
    getIndex(s2ch.thH.esc, index2);
    getIndex(s2ch.thH.reload, index3);
    getIndex(s2ch.thH.move, index4);
    getIndex(s2ch.thH.shift, index5);
    sprintf(s2ch.menuThreadH.main, "　%s : 決定　　%s : 戻る　　%s : 更新　　%s : お気に入り　　%s : メニュー切替",
            sBtnH[index1], sBtnH[index2], sBtnH[index3], sBtnH[index4], sBtnH[index5]);

    getIndex(s2ch.listH.top, index1);
    getIndex(s2ch.listH.end, index2);
    getIndex(s2ch.thH.sort, index3);
    getIndex(s2ch.thH.search, index4);
    getIndex(s2ch.thH.search2ch, index5);
    sprintf(s2ch.menuThreadH.sub, "　%s : 先頭　　%s : 最後　　%s : ソ\ート　　%s : 検索　　%s : 全板検索",
            sBtnH[index1], sBtnH[index2], sBtnH[index3], sBtnH[index4], sBtnH[index5]);

    getIndex(s2ch.thV.ok, index1);
    getIndex(s2ch.thV.esc, index2);
    getIndex(s2ch.thV.reload, index3);
    getIndex(s2ch.thV.move, index4);
    getIndex(s2ch.thV.shift, index5);
    sprintf(s2ch.menuThreadV.main, "　%s : 決定　　　%s : 戻る　　　%s : 更新\n　%s : お気に入り　　　%s : メニュー切替",
            sBtnV[index1], sBtnV[index2], sBtnV[index3], sBtnV[index4], sBtnV[index5]);

    getIndex(s2ch.listV.top, index1);
    getIndex(s2ch.listV.end, index2);
    getIndex(s2ch.thV.sort, index3);
    getIndex(s2ch.thV.search, index4);
    getIndex(s2ch.thV.search2ch, index5);
    sprintf(s2ch.menuThreadV.sub, "　%s : 先頭　　　%s : 最後　　　%s : ソ\ート\n　%s : 検索　　　%s : 全板検索",
            sBtnV[index1], sBtnV[index2], sBtnV[index3], sBtnV[index4], sBtnV[index5]);
}

/****************
 スレッド一覧表示
*****************/
int psp2chThread(int retSel)
{
    static char* menuStr = "";
    static int ret = 0;
    int lineEnd, rMenu, change, res;

    if (ret == 0)
    {
        ret = retSel;
    }
    if (s2ch.tateFlag)
    {
        lineEnd = DRAW_LINE_V;
    }
    else
    {
        lineEnd = DRAW_LINE_H;
    }
    if(sceCtrlPeekBufferPositive(&s2ch.pad, 1))
    {
        if (s2ch.tateFlag)
        {
            rMenu = psp2chCursorSet(&s2ch.thread, lineEnd, s2ch.thV.shift, &change);
        }
        else
        {
            rMenu = psp2chCursorSet(&s2ch.thread, lineEnd, s2ch.thH.shift, &change);
        }
        if (rMenu)
        {
            if (s2ch.tateFlag)
            {
                menuStr = s2ch.menuThreadV.sub;
            }
            else
            {
                menuStr = s2ch.menuThreadH.sub;
            }
        }
        else
        {
            if (s2ch.tateFlag)
            {
                menuStr = s2ch.menuThreadV.main;
            }
            else
            {
                menuStr = s2ch.menuThreadH.main;
            }
        }
        if (s2ch.pad.Buttons != s2ch.oldPad.Buttons)
        {
            s2ch.oldPad = s2ch.pad;
            if (s2ch.pad.Buttons & PSP_CTRL_SELECT)
            {
                s2ch.tateFlag = (s2ch.tateFlag) ? 0 : 1;
            }
            // STARTボタン
            else if(s2ch.pad.Buttons & PSP_CTRL_START)
            {
                psp2chMenu();
            }
            else if (rMenu)
            {
                // ソート
                if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.thH.sort) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.thV.sort))
                {
                    psp2chThreadSort();
                }
                // 検索
                else if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.thH.search) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.thV.search))
                {
                    if (psp2chThreadSearch() == 0 && keyWords[0])
                    {
                        psp2chSort(10);
                    }
                }
                // 2ちゃんねる検索
                else if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.thH.search2ch) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.thV.search2ch))
                {
                    if (psp2chThreadSearch() == 0 && keyWords[0])
                    {
                        if (s2ch.findList)
                        {
                            free(s2ch.findList);
                            s2ch.findList = NULL;
                        }
                        s2ch.sel = 7;
                    }
                }
            }
            else
            {
                // レス表示
                if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.thH.ok) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.thV.ok))
                {
                    free(s2ch.resList);
                    s2ch.resList = NULL;
                    preLine = -2;
                    s2ch.sel = 5;
                }
                // 戻る
                else if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.thH.esc) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.thV.esc))
                {
                    s2ch.sel = ret;
                    ret = 0;
                    return 0;
                }
                // スレ一覧の更新
                else if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.thH.reload) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.thV.reload))
                {
                    psp2chGetSubject(s2ch.ita.select);
                    psp2chThreadList(s2ch.ita.select);
                    s2ch.thread.start = 0;
                    s2ch.thread.select = 0;
                }
                // お気に入りへ移動
                else if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.thH.move) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.thV.move))
                {
                    s2ch.sel = 1;
                }
            }
			pgPrintMenuBar(menuStr);
			change = 1;
        }
		res = psp2chPadSet(s2ch.viewX);
		if (res != s2ch.viewX || change)
		{
			s2ch.viewX = res;
	        psp2chDrawThread();
		}
        pgCopy(s2ch.viewX, 0);
        pgCopyMenuBar();
        sceDisplayWaitVblankStart();
        framebuffer = sceGuSwapBuffers();
    }
    return 0;
}

/****************
DATからスレタイ取得
*****************/
void psp2chThreadGetTitle(char* dir, int dat, char* title, int len)
{
    SceUID fd;
    char path[4096];
    char *p, *q;

    sprintf(path, "%s/%s/%s/%d.dat", s2ch.cwDir, s2ch.logDir, dir, dat);
    fd = sceIoOpen(path, PSP_O_RDONLY, 0777);
    if (fd < 0)
    {
        title[0] = '\0';
    }
    else
    {
        sceIoRead(fd, path, 4096);
        sceIoClose(fd);
        p = strstr(path, "<>"); // name
        p++;
        p =  strstr(p, "<>"); // mail
        p++;
        p =  strstr(p, "<>"); // date
        p++;
        p =  strstr(p, "<>"); // text
        p += 2;
        q =  strchr(p, '\n'); // title
        *q = '\0';
        p[len] = '\0';
        strcpy(title, p);
    }
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
    int i, dat, tmpCount, setFlag;;
    time_t tm;

    sprintf(file, "%s/%s/%s/subject.txt", s2ch.cwDir, s2ch.logDir, s2ch.itaList[ita].title);
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
            psp2chErrorDialog("File stat error\n%s", file);
            return -1;
        }
    }
    buf = (char*)malloc(st.st_size + 1);
    if (buf == NULL)
    {
        psp2chErrorDialog("memorry error\npsp2chThreadList() buf");
        return -1;
    }
    fd = sceIoOpen(file, PSP_O_RDONLY, 0777);
    if (fd < 0)
    {
        free(buf);
        psp2chErrorDialog("File open error\n%s", file);
        return -1;
    }
    sceIoRead(fd, buf, st.st_size);
    sceIoClose(fd);
    buf[st.st_size] = '\0';
    s2ch.thread.count = 0;
    p = buf;
    while (*p)
    {
        if (*p++ == '\n')
        {
            s2ch.thread.count++;
        }
    }
    s2ch.thread.count -= 2;
    sprintf(file, "%s/%s/%s", s2ch.cwDir, s2ch.logDir, s2ch.itaList[ita].title);
    dfd = sceIoDopen(file);
    if (dfd >= 0)
    {
        memset(&dir, 0, sizeof(dir)); // 初期化しないとreadに失敗する
        while (sceIoDread(dfd, &dir) > 0)
        {
            s2ch.thread.count++;
        }
        sceIoDclose(dfd);
    }
    s2ch.threadList = (S_2CH_THREAD*)realloc(s2ch.threadList, sizeof(S_2CH_THREAD) * s2ch.thread.count);
    if (s2ch.threadList == NULL )
    {
        free(buf);
        psp2chErrorDialog("memorry error\nThreadList");
        return -1;
    }
    s2ch.thread.count = 0;
    q = buf;
    while (*q++ != '\n');
    while (*q++ != '\n');
    p = q;
    sceKernelLibcTime (&tm);
    while(*q)
    {
        s2ch.threadList[s2ch.thread.count].id = s2ch.thread.count;
        sscanf(p, "%d", &s2ch.threadList[s2ch.thread.count].dat);
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
        strcpy(s2ch.threadList[s2ch.thread.count].title, line);
        p++;
        sscanf(p, "%d", &s2ch.threadList[s2ch.thread.count].res);
        s2ch.threadList[s2ch.thread.count].old = 0;
        if (tm > dat)
        {
            s2ch.threadList[s2ch.thread.count].ikioi = s2ch.threadList[s2ch.thread.count].res * 60 *60 * 24 / (tm - s2ch.threadList[s2ch.thread.count].dat);
        }
        s2ch.thread.count++;
        p = q;
    }
    free(buf);
    tmpCount = s2ch.thread.count;
    pgPrintMenuBar("取得済みスレッドの検索中");
    pgCopyMenuBar();
    sceDisplayWaitVblankStart();
    framebuffer = sceGuSwapBuffers();
    dfd = sceIoDopen(file);
    if (dfd >= 0)
    {
        memset(&dir, 0, sizeof(dir));
        while (sceIoDread(dfd, &dir) > 0)
        {
            if (strstr(dir.d_name, ".dat"))
            {
                sscanf(dir.d_name, "%d", &dat);
                setFlag = 0;
                for (i = 0; i < tmpCount; i++)
                {
                    if (s2ch.threadList[i].dat == dat)
                    {
                        s2ch.threadList[i].old = psp2chGetResCount(s2ch.itaList[ita].title, dat); // psp2chFavorite.c
                        setFlag = 1;
                        break;
                    }
                }
                // DAT落ちスレの追加
                if (setFlag == 0)
                {
                    s2ch.threadList[s2ch.thread.count].id = -1;
                    s2ch.threadList[s2ch.thread.count].dat = dat;
                    s2ch.threadList[s2ch.thread.count].res = 0;
                    s2ch.threadList[s2ch.thread.count].old = psp2chGetResCount(s2ch.itaList[ita].title, dat);
                    s2ch.threadList[s2ch.thread.count].ikioi = 0;
                    psp2chThreadGetTitle(s2ch.itaList[ita].title, dat, s2ch.threadList[s2ch.thread.count].title, 128);
                    s2ch.thread.count++;
                }
            }
        }
        sceIoDclose(dfd);
    }
    // s2ch.thread.counがDAT落ちでない分少なくなるので縮める
    s2ch.threadList = (S_2CH_THREAD*)realloc(s2ch.threadList, sizeof(S_2CH_THREAD) * s2ch.thread.count);
    if (s2ch.threadList == NULL )
    {
        psp2chErrorDialog("memorry error\nThreadList");
        return -1;
    }
    threadSort = (int*)realloc(threadSort, sizeof(int) * s2ch.thread.count);
    if (threadSort == NULL)
    {
        psp2chErrorDialog("memorry error\nThreadSort");
        return -1;
    }
    psp2chSort(2);
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
    int ret;
    S_NET net;
    SceUID fd;
    char path[256];
    char buf[256];
    char lastModified[32];
    char eTag[32];
    char *p, *q;

    // Make ita directory
    sprintf(path, "%s/%s/%s", s2ch.cwDir, s2ch.logDir, s2ch.itaList[ita].title);
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
    sprintf(path, "%s/subject.txt", s2ch.itaList[ita].dir);
    ret = psp2chGet(s2ch.itaList[ita].host, path, buf, NULL, &net);
    if (ret < 0)
    {
        return ret;
    }
    switch(net.status)
    {
        case 200: // OK
            break;
        case 301: // Moved Permanently
            free(net.body);
            memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
            s2ch.mh.options = PSP_UTILITY_MSGDIALOG_OPTION_TEXT | PSP_UTILITY_MSGDIALOG_OPTION_YESNO_BUTTONS;
            strcpy(s2ch.mh.message, TEXT_7);
            pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
            sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
            if (s2ch.mh.buttonPressed == PSP_UTILITY_MSGDIALOG_RESULT_YES)
            {
                psp2chGetMenu();
            }
            return -1;
        case 304: // Not modified
            free(net.body);
            return 0;
        default:
            free(net.body);
            psp2chErrorDialog("HTTP error\nhost %s path %s\nStatus code %d", s2ch.itaList[ita].host, path, ret);
            return -1;
    }
    // Receive and Save subject
    sprintf(path, "%s/%s/%s/subject.txt", s2ch.cwDir, s2ch.logDir, s2ch.itaList[ita].title);
    fd = sceIoOpen(path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
    if (fd < 0)
    {
        free(net.body);
        psp2chErrorDialog("File open error\n%s", path);
        return fd;
    }
    sceIoWrite(fd, net.head.Last_Modified, strlen(net.head.Last_Modified));
    sceIoWrite(fd, net.head.ETag, strlen(net.head.ETag));
    sceIoWrite(fd, net.body, net.length);
    sceIoClose(fd);
    free(net.body);
    return 0;
}

/****************
スレッドをソートする
threadSort配列にソートデータが入る
sort:
0=既読スレ順
1=勢い
2=番号順
3=作成日(降順)
4=作成日(昇順)
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
        for (i = 0, j = 0; i < s2ch.thread.count; i++)
        {
            if (s2ch.threadList[i].old > 0)
            {
                threadSort[j] = i;
                j++;
            }
        }
        for (i = 0; i < s2ch.thread.count; i++)
        {
            if (s2ch.threadList[i].old == 0)
            {
                threadSort[j] = i;
                j++;
            }
        }
        break;
    case 1:
        for (i = 0; i < s2ch.thread.count; i++)
        {
            threadSort[i] = i;
        }
        for (i = 0; i < s2ch.thread.count-1; i++)
        {
            for (j = i; j < s2ch.thread.count; j++)
            {
                if (s2ch.threadList[threadSort[j]].ikioi > s2ch.threadList[threadSort[i]].ikioi)
                {
                    tmp = threadSort[j];
                    threadSort[j] = threadSort[i];
                    threadSort[i] = tmp;
                }
            }
        }
        break;
    case 2:
        for (i = 0; i < s2ch.thread.count; i++)
        {
            threadSort[i] = i;
        }
        break;
    case 3:
        for (i = 0; i < s2ch.thread.count; i++)
        {
            threadSort[i] = i;
        }
        for (i = 0; i < s2ch.thread.count-1; i++)
        {
            for (j = i; j < s2ch.thread.count; j++)
            {
                if (s2ch.threadList[threadSort[j]].dat > s2ch.threadList[threadSort[i]].dat)
                {
                    tmp = threadSort[j];
                    threadSort[j] = threadSort[i];
                    threadSort[i] = tmp;
                }
            }
        }
        break;
    case 4:
        for (i = 0; i < s2ch.thread.count; i++)
        {
            threadSort[i] = i;
        }
        for (i = 0; i < s2ch.thread.count-1; i++)
        {
            for (j = i; j < s2ch.thread.count; j++)
            {
                if (s2ch.threadList[threadSort[j]].dat < s2ch.threadList[threadSort[i]].dat)
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
        for (i = 0, j = 0; i < s2ch.thread.count; i++)
        {
            strcpy(haystack, s2ch.threadList[i].title);
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
        for (i = 0; i < s2ch.thread.count; i++)
        {
            strcpy(haystack, s2ch.threadList[i].title);
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
#define MAX_SORT_COUNT (5)
void psp2chThreadSort(void)
{
    const unsigned short title[] = {0x3069,0x306E,0x9805,0x76EE,0x3067,0x30BD,0x30FC,0x30C8,0x3057,0x307E,0x3059,0x304B,0};
    const unsigned short text1[] = {0x65E2,0x8AAD,0x30B9,0x30EC,0}; // 既読スレ
    const unsigned short text2[] = {0x52E2,0x3044,0}; // 勢い
    const unsigned short text3[] = {0x756A,0x53F7,0x9806,0}; // 番号順
    const unsigned short text4[] = {0x4F5C,0x6210,0x65E5,0x0028,0x964D,0x9806,0x0029,0}; // 作成日(降順)
    const unsigned short text5[] = {0x4F5C,0x6210,0x65E5,0x0028,0x6607,0x9806,0x0029,0}; // 作成日(昇順)
    const unsigned short* text[MAX_SORT_COUNT] = {text1, text2, text3, text4, text5};
    int i, select = 0;

    while (s2ch.running)
    {
        if(sceCtrlPeekBufferPositive(&s2ch.pad, 1))
        {
            if (s2ch.pad.Buttons != s2ch.oldPad.Buttons)
            {
                s2ch.oldPad = s2ch.pad;
                if(s2ch.pad.Buttons & PSP_CTRL_UP)
                {
                    if (select)
                    {
                        select--;
                    }
                }
                if(s2ch.pad.Buttons & PSP_CTRL_DOWN)
                {
                    if (select < MAX_SORT_COUNT - 1)
                    {
                        select++;
                    }
                }
                if(s2ch.pad.Buttons & PSP_CTRL_CIRCLE)
                {
                    break;
                }
                if(s2ch.pad.Buttons & PSP_CTRL_CROSS)
                {
                    return;
                }
            }
            sceGuStart(GU_DIRECT, list);
			sceGuScissor(0, 0, SCR_WIDTH, SCR_HEIGHT);
            sceGuClearColor(0xFFFF0000);
            sceGuClear(GU_COLOR_BUFFER_BIT);
            s2ch.pgCursorX = 240;
            s2ch.pgCursorY =  77;
            intraFontSetStyle(jpn0, 1.0f, 0xFF00FFFF, 0xFFFF0000, INTRAFONT_ALIGN_CENTER);
            intraFontPrintUCS2(jpn0, s2ch.pgCursorX, s2ch.pgCursorY, title);
            s2ch.pgCursorX = 240;
            s2ch.pgCursorY += 25;
            for (i = 0; i < MAX_SORT_COUNT; i++)
            {
                if (select == i)
                {
                    intraFontSetStyle(jpn0, 0.9f, 0xFFFFFFFF, 0xFF000000, INTRAFONT_ALIGN_CENTER);
                    intraFontPrintUCS2(jpn0, s2ch.pgCursorX, s2ch.pgCursorY, text[i]);
                }
                else
                {
                    intraFontSetStyle(jpn0, 0.9f, 0xFFCCCCCC, 0, INTRAFONT_ALIGN_CENTER);
                    intraFontPrintUCS2(jpn0, s2ch.pgCursorX, s2ch.pgCursorY, text[i]);
                }
                s2ch.pgCursorX = 240;
                s2ch.pgCursorY += 20;
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
void psp2chDrawThread(void)
{
    int start;
    int i;
    int lineEnd, scrW, scrH, resCount;

    if (s2ch.tateFlag)
    {
        lineEnd = DRAW_LINE_V;
        scrW = SCR_HEIGHT;
        scrH = SCR_WIDTH;
        resCount = scrW - FONT_HEIGHT * 4 + s2ch.viewX;
    }
    else
    {
        lineEnd = DRAW_LINE_H;
        scrW = SCR_WIDTH;
        scrH = SCR_HEIGHT;
        resCount = scrW - FONT_HEIGHT * 4 + s2ch.viewX;
    }
    start = s2ch.thread.start;
    if (start + lineEnd > s2ch.thread.count)
    {
        start = s2ch.thread.count - lineEnd;
    }
    if (start < 0)
    {
        start = 0;
    }
    pgFillvram(s2ch.threadColor.bg, s2ch.viewX, 0, s2ch.viewX + scrW, BUF_HEIGHT, 2);
    s2ch.pgCursorY = 0;
    for (i = start; i < start + lineEnd; i++)
    {
        if (i >= s2ch.thread.count)
        {
            return;
        }
        s2ch.pgCursorX = 0;
        if (i == s2ch.thread.select)
        {
            pgFillvram(s2ch.threadColor.s_bg, s2ch.viewX, s2ch.pgCursorY, s2ch.viewX + scrW, LINE_PITCH, 2);
            if (s2ch.threadList[threadSort[i]].id >= 0)
            {
                pgPrintNumber(s2ch.threadList[threadSort[i]].id + 1, s2ch.threadColor.s_num, s2ch.threadColor.s_bg);
            }
        }
        else
        {
            if (s2ch.threadList[threadSort[i]].id >= 0)
            {
                pgPrintNumber(s2ch.threadList[threadSort[i]].id + 1, s2ch.threadColor.num, s2ch.threadColor.bg);
            }
        }
        s2ch.pgCursorX = FONT_HEIGHT * 2 + 2;
        if (i == s2ch.thread.select)
        {
            if (s2ch.threadList[threadSort[i]].old > 0)
            {
                pgPrint(s2ch.threadList[threadSort[i]].title, s2ch.threadColor.s_text2, s2ch.threadColor.s_bg, resCount+12);
            }
            else
            {
                pgPrint(s2ch.threadList[threadSort[i]].title, s2ch.threadColor.s_text1, s2ch.threadColor.s_bg, resCount+12);
            }
        }
        else
        {
            if (s2ch.threadList[threadSort[i]].old > 0)
            {
                pgPrint(s2ch.threadList[threadSort[i]].title, s2ch.threadColor.text2, s2ch.threadColor.bg, resCount+12);
            }
            else
            {
                pgPrint(s2ch.threadList[threadSort[i]].title, s2ch.threadColor.text1, s2ch.threadColor.bg, resCount+12);
            }
        }
        s2ch.pgCursorX = resCount;
        if (i == s2ch.thread.select)
        {
            pgPrintNumber(s2ch.threadList[threadSort[i]].res, s2ch.threadColor.s_count1, s2ch.threadColor.s_bg);
            if (s2ch.threadList[threadSort[i]].old > 0)
            {
                pgPrintNumber(s2ch.threadList[threadSort[i]].old, s2ch.threadColor.s_count2, s2ch.threadColor.s_bg);
            }
        }
        else
        {
            pgPrintNumber(s2ch.threadList[threadSort[i]].res, s2ch.threadColor.count1, s2ch.threadColor.bg);
            if (s2ch.threadList[threadSort[i]].old > 0)
            {
                pgPrintNumber(s2ch.threadList[threadSort[i]].old, s2ch.threadColor.count2, s2ch.threadColor.bg);
            }
        }
        s2ch.pgCursorY += LINE_PITCH;
    }
}
