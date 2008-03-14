/*
* $Id$
*/

#include <stdio.h>
#include <malloc.h>
#include <time.h>
#include <pspdebug.h>
#include "psp2ch.h"
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
���j���[������̍쐬
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
    sprintf(s2ch.menuThreadH.main, "�@%s : ����@�@%s : �߂�@�@%s : �X�V�@�@%s : ���C�ɓ���@�@%s : ���j���[�ؑ�",
            sBtnH[index1], sBtnH[index2], sBtnH[index3], sBtnH[index4], sBtnH[index5]);

    getIndex(s2ch.listH.top, index1);
    getIndex(s2ch.listH.end, index2);
    getIndex(s2ch.thH.sort, index3);
    getIndex(s2ch.thH.search, index4);
    getIndex(s2ch.thH.search2ch, index5);
    sprintf(s2ch.menuThreadH.sub, "�@%s : �擪�@�@%s : �Ō�@�@%s : �\\�[�g�@�@%s : �����@�@%s : �S����",
            sBtnH[index1], sBtnH[index2], sBtnH[index3], sBtnH[index4], sBtnH[index5]);

    getIndex(s2ch.thV.ok, index1);
    getIndex(s2ch.thV.esc, index2);
    getIndex(s2ch.thV.reload, index3);
    getIndex(s2ch.thV.move, index4);
    getIndex(s2ch.thV.shift, index5);
    sprintf(s2ch.menuThreadV.main, "�@%s : ����@�@�@%s : �߂�@�@�@%s : �X�V\n�@%s : ���C�ɓ���@�@�@%s : ���j���[�ؑ�",
            sBtnV[index1], sBtnV[index2], sBtnV[index3], sBtnV[index4], sBtnV[index5]);

    getIndex(s2ch.listV.top, index1);
    getIndex(s2ch.listV.end, index2);
    getIndex(s2ch.thV.sort, index3);
    getIndex(s2ch.thV.search, index4);
    getIndex(s2ch.thV.search2ch, index5);
    sprintf(s2ch.menuThreadV.sub, "�@%s : �擪�@�@�@%s : �Ō�@�@�@%s : �\\�[�g\n�@%s : �����@�@�@%s : �S����",
            sBtnV[index1], sBtnV[index2], sBtnV[index3], sBtnV[index4], sBtnV[index5]);
}

/****************
 �X���b�h�ꗗ�\��
*****************/
int psp2chThread(int retSel)
{
    static int scrollX = 0;
    static char* menuStr = "";
    static int ret = 0;
    int lineEnd, rMenu;

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
            rMenu = psp2chCursorSet(&s2ch.thread, lineEnd, s2ch.thV.shift);
        }
        else
        {
            rMenu = psp2chCursorSet(&s2ch.thread, lineEnd, s2ch.thH.shift);
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
            // START�{�^��
            else if(s2ch.pad.Buttons & PSP_CTRL_START)
            {
                psp2chMenu(scrollX, 0);
            }
            else if (rMenu)
            {
                // �\�[�g
                if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.thH.sort) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.thV.sort))
                {
                    psp2chThreadSort();
                }
                // ����
                else if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.thH.search) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.thV.search))
                {
                    if (psp2chThreadSearch() == 0 && keyWords[0])
                    {
                        psp2chSort(10);
                    }
                }
                // 2�����˂錟��
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
                // ���X�\��
                if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.thH.ok) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.thV.ok))
                {
                    free(s2ch.resList);
                    s2ch.resList = NULL;
                    preLine = -2;
                    s2ch.sel = 5;
                }
                // �߂�
                else if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.thH.esc) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.thV.esc))
                {
                    s2ch.sel = ret;
                    ret = 0;
                    return 0;
                }
                // �X���ꗗ�̍X�V
                else if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.thH.reload) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.thV.reload))
                {
                    psp2chGetSubject(s2ch.ita.select);
                    psp2chThreadList(s2ch.ita.select);
                    s2ch.thread.start = 0;
                    s2ch.thread.select = 0;
                }
                // ���C�ɓ���ֈړ�
                else if((!s2ch.tateFlag && s2ch.pad.Buttons & s2ch.thH.move) || (s2ch.tateFlag && s2ch.pad.Buttons & s2ch.thV.move))
                {
                    s2ch.sel = 1;
                }
            }
        }
        scrollX = psp2chPadSet(scrollX);
        psp2chDrawThread(scrollX);
        pgCopy(scrollX, 0);
        pgMenuBar(menuStr);
        sceDisplayWaitVblankStart();
        framebuffer = sceGuSwapBuffers();
    }
    return 0;
}

/****************
�����X�e�ɕۑ����ꂽsubject.txt��ǂݍ����
threadList�\���̂��쐬
�ԍ����Ƀ\�[�g
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
            memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
            sprintf(s2ch.mh.message, "File stat error\n%s", file);
            pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
            sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
            return -1;
        }
    }
    buf = (char*)malloc(st.st_size + 1);
    if (buf == NULL)
    {
        memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
        strcpy(s2ch.mh.message, "memorry error\npsp2chThreadList() buf");
        pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
        sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
        return -1;
    }
    fd = sceIoOpen(file, PSP_O_RDONLY, 0777);
    if (fd < 0)
    {
        free(buf);
        memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
        sprintf(s2ch.mh.message, "File open error\n%s", file);
        pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
        sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
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
    s2ch.threadList = (S_2CH_THREAD*)realloc(s2ch.threadList, sizeof(S_2CH_THREAD) * s2ch.thread.count);
    if (s2ch.threadList == NULL )
    {
        free(buf);
        memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
        strcpy(s2ch.mh.message, "memorry error\nThreadList");
        pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
        sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
        return -1;
    }
    threadSort = (int*)realloc(threadSort, sizeof(int) * s2ch.thread.count);
    if (threadSort == NULL)
    {
        free(buf);
        memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
        strcpy(s2ch.mh.message, "memorry error\nThreadSort");
        pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
        sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
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
    pgMenuBar("�擾�ς݃X���b�h�̌�����");
    sceDisplayWaitVblankStart();
    framebuffer = sceGuSwapBuffers();
    sprintf(file, "%s/%s/%s", s2ch.cwDir, s2ch.logDir, s2ch.itaList[ita].title);
    dfd = sceIoDopen(file);
    if (dfd >= 0)
    {
        memset(&dir, 0, sizeof(dir)); // ���������Ȃ���read�Ɏ��s����
        while (sceIoDread(dfd, &dir) > 0)
        {
            if (strstr(dir.d_name, ".idx"))
            {
                sscanf(dir.d_name, "%d", &dat);
                for (i = 0; i < s2ch.thread.count; i++)
                {
                    if (s2ch.threadList[i].dat == dat)
                    {
                        sprintf(file, "%s/%s/%s/%s", s2ch.cwDir, s2ch.logDir, s2ch.itaList[ita].title, dir.d_name);
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
                            sscanf(p, "%d", &s2ch.threadList[i].old);
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
�̃f�B���N�g�����Ȃ���΍쐬
�̃f�B���N�g����subject.txt������Γǂݍ���
subject.txt�̎擾���f�[�^������2ch�ɃA�N�Z�X
�X�V����Ă���ΐV�����f�[�^���擾���ĕۑ�
�Ȃ���Ε��ʂɎ擾���ĕۑ�
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
    sprintf(path, "%s/%s/%s", s2ch.cwDir, s2ch.logDir, s2ch.itaList[ita].title);
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
    mySocket = psp2chRequest(s2ch.itaList[ita].host, path, buf);
    if (mySocket < 0)
    {
        return mySocket;
    }
    ret = psp2chGetStatusLine(mySocket);
    switch(ret)
    {
        case 200: // OK
            break;
        case 301: // Moved Permanently
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
            psp2chCloseSocket(mySocket);
            return 0;
        default:
            memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
            sprintf(s2ch.mh.message, "HTTP error\nhost %s path %s\nStatus code %d", s2ch.itaList[ita].host, path, ret);
            pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
            sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
            psp2chCloseSocket(mySocket);
            return -1;
    }
    // Receive and Save subject
    sprintf(path, "%s/%s/%s/subject.txt", s2ch.cwDir, s2ch.logDir, s2ch.itaList[ita].title);
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
    contentLength = psp2chGetHttpHeaders(mySocket, &resHeader, NULL);
    if (contentLength <= 0)
    {
        psp2chCloseSocket(mySocket);
        sceIoClose(fd);
        return -1;
    }
    sceIoWrite(fd, resHeader.Last_Modified, strlen(resHeader.Last_Modified));
    sceIoWrite(fd, resHeader.ETag, strlen(resHeader.ETag));
    sprintf(buf, "http://%s/%s/subject.txt ����f�[�^��]�����Ă��܂�...", s2ch.itaList[ita].host, s2ch.itaList[ita].dir);
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
�X���b�h���\�[�g����
threadSort�z��Ƀ\�[�g�f�[�^������
sort:
0=���ǃX����
1=�ԍ���
2=�쐬��(�~��)
3=�쐬��(����)
4=����
10=�����P�ꏇ
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
        break;
    case 2:
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
    case 3:
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
    case 4:
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
�\�[�g�p�_�C�A���O�\��
*****************/
#define MAX_SORT_COUNT (5)
void psp2chThreadSort(void)
{
    const unsigned short title[] = {0x3069,0x306E,0x9805,0x76EE,0x3067,0x30BD,0x30FC,0x30C8,0x3057,0x307E,0x3059,0x304B,0};
    const unsigned short text1[] = {0x65E2,0x8AAD,0x30B9,0x30EC,0}; // ���ǃX��
    const unsigned short text2[] = {0x756A,0x53F7,0x9806,0}; // �ԍ���
    const unsigned short text3[] = {0x4F5C,0x6210,0x65E5,0x0028,0x964D,0x9806,0x0029,0}; // �쐬��(�~��)
    const unsigned short text4[] = {0x4F5C,0x6210,0x65E5,0x0028,0x6607,0x9806,0x0029,0}; // �쐬��(����)
    const unsigned short text5[] = {0x52E2,0x3044,0}; // ����
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
            sceGuClearColor(BLUE);
            sceGuClearDepth(0);
            sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
            s2ch.pgCursorX = 240;
            s2ch.pgCursorY =  77;
            intraFontSetStyle(jpn0, 1.0f, YELLOW, BLUE, INTRAFONT_ALIGN_CENTER);
            intraFontPrintUCS2(jpn0, s2ch.pgCursorX, s2ch.pgCursorY, title);
            s2ch.pgCursorX = 240;
            s2ch.pgCursorY += 25;
            for (i = 0; i < MAX_SORT_COUNT; i++)
            {
                if (select == i)
                {
                    intraFontSetStyle(jpn0, 0.9f, WHITE, BLACK, INTRAFONT_ALIGN_CENTER);
                    intraFontPrintUCS2(jpn0, s2ch.pgCursorX, s2ch.pgCursorY, text[i]);
                }
                else
                {
                    intraFontSetStyle(jpn0, 0.9f, GRAY, 0, INTRAFONT_ALIGN_CENTER);
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
�����_�C�A���O�\��
*****************/
int psp2chThreadSearch(void)
{
    const unsigned short text1[] = {0x691C,0x7D22,0x6587,0x5B57,0x5217,0x3092,0x5165,0x529B,0x3057,0x3066,0x304F,0x3060,0x3055,0x3044,0};
    char* text2 = "����������";

    return psp2chInputDialog(text1, text2);
}

/****************
�X���ꗗ�̕`�惋�[�`��
*****************/
void psp2chDrawThread(int scrollX)
{
    int start;
    int i;
    int lineEnd, scrW, scrH, resCount;

    if (s2ch.tateFlag)
    {
        lineEnd = DRAW_LINE_V;
        scrW = SCR_HEIGHT;
        scrH = SCR_WIDTH;
        resCount = scrW - FONT_HEIGHT * 4 + scrollX;
    }
    else
    {
        lineEnd = DRAW_LINE_H;
        scrW = SCR_WIDTH;
        scrH = SCR_HEIGHT;
        resCount = scrW - FONT_HEIGHT * 4 + scrollX;
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
    pgFillvram(s2ch.threadColor.bg, 0, 0, BUF_WIDTH, BUF_HEIGHT);
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
            pgFillvram(s2ch.threadColor.s_bg, 0, s2ch.pgCursorY, BUF_WIDTH, LINE_PITCH);
            pgPrintNumber(s2ch.threadList[threadSort[i]].id + 1, s2ch.threadColor.s_num, s2ch.threadColor.s_bg);
        }
        else
        {
            pgPrintNumber(s2ch.threadList[threadSort[i]].id + 1, s2ch.threadColor.num, s2ch.threadColor.bg);
        }
        s2ch.pgCursorX = FONT_HEIGHT * 2;
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
