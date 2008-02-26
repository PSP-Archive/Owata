/*
* $Id$
*/

#include "pspdialogs.h"
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <time.h>
#include <sys/unistd.h>
#include <arpa/inet.h>
#include <pspdebug.h>
#include <pspctrl.h>
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

extern int running; //main.c
extern char cwDir[256]; //main.c
extern void* framebuffer; // pg.c
extern unsigned long pgCursorX, pgCursorY; // pg.c
extern unsigned int list[512*512]; // pg.c
extern intraFont* jpn0; // pg.c

int tateFlag = 0;
int sel = 0;
const char* userAgent = "Monazilla/1.00 (Compatible; PSP; ja) owata\(^o^)/0.5.1";
const char* logDir = "log";
char cookie[128] = {0};
char keyWords[128];
SceCtrlData pad;
SceCtrlData oldPad;
MESSAGE_HELPER mh;
S_2CH_HEADER_COLOR resHeaderColor;
S_2CH_RES_COLOR resColor;
S_2CH_BAR_COLOR resBarColor;
S_2CH_HEADER_COLOR resAHeaderColor;
S_2CH_RES_COLOR resAColor;
S_2CH_BAR_COLOR resABarColor;
S_2CH_MENU_COLOR menuColor;
S_2CH_THREAD_COLOR threadColor;
S_2CH_ITA_COLOR cateOnColor;
S_2CH_ITA_COLOR cateOffColor;
S_2CH_FORM_COLOR formColor;
S_2CH_TXT_COLOR menuWinColor;

/*********************************
���C�����[�v
running���^�̊ԃ��[�v�i�~�{�^���̏I����home�{�^���ł̏I����running��0�j
�Z���N�^�[(sel)�Ŋe�֐��֕���
���X�\���͂��C�ɓ���A�ꗗ�A���X�\���������N�W�����v�A�S�������ʂ��烉�b�p�[�֐��ֈړ�
�ړ����ɖ߂邽��retSel���g�p�i���X�\�������"�߂�"��sel = retSel���s�j
�S�������ړ����ɖ߂邽��retSel�g�p
*********************************/
int psp2ch(void)
{
    int retSel = 0;

    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
    while (running)
    {
        switch (sel)
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
            psp2chThread();
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
�X�^�[�g���
*****************************/
void psp2chStart(void)
{
    if(sceCtrlPeekBufferPositive(&pad, 1))
    {
        if (pad.Buttons != oldPad.Buttons)
        {
            oldPad = pad;
            if(pad.Buttons & PSP_CTRL_CROSS)
            {
                if (psp2chOwata())
                {
                    return;
                }
            }
            else if(pad.Buttons & PSP_CTRL_START)
            {
                sel = 2;
                return;
            }
            else if(pad.Buttons & PSP_CTRL_CIRCLE)
            {
                sel = 1;
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
�I�����
*****************************/
int psp2chOwata(void)
{
    memset(&mh,0,sizeof(MESSAGE_HELPER));
    mh.options = PSP_UTILITY_MSGDIALOG_OPTION_TEXT | PSP_UTILITY_MSGDIALOG_OPTION_YESNO_BUTTONS;
    strcpy(mh.message, TEXT_3);
    pspShowMessageDialog(&mh, DIALOG_LANGUAGE_AUTO);
    sceCtrlPeekBufferPositive(&oldPad, 1);
    if (mh.buttonPressed == PSP_UTILITY_MSGDIALOG_RESULT_YES)
    {
        tateFlag = 0;
        pgPrintOwata();
        pgCopy(0, 0);
        framebuffer = sceGuSwapBuffers();
        pgWaitVn(10);
        running = 0;
        return 1;
    }
    return 0;
}

/*****************************
�J�[�\���ړ����[�`��
S_2CH_SCREEN�\���̂�
start:�\���J�n�s
select:�J�[�\���I���s
��ύX��R�{�^������Ԃ�
*****************************/
int psp2chCursorSet(S_2CH_SCREEN* line, int lineEnd)
{
    static int keyStart = 0, keyRepeat = 0;
    static clock_t keyTime = 0;
    int rMenu;
    int padUp = 0, padDown = 0;

    if (tateFlag)
    {
        if (pad.Lx == 255)
        {
            padUp = 1;
        }
        else if (pad.Lx == 0)
        {
            padDown = 1;
        }
    }
    else
    {
        if (pad.Ly == 0)
        {
            padUp = 1;
        }
        else if (pad.Ly == 255)
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
    if (pad.Buttons != oldPad.Buttons || keyRepeat || padUp || padDown)
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
        if((pad.Buttons & PSP_CTRL_DOWN && !tateFlag) || (pad.Buttons & PSP_CTRL_LEFT && tateFlag) || padDown)
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
        if((pad.Buttons & PSP_CTRL_LEFT && !tateFlag) || (pad.Buttons & PSP_CTRL_UP && tateFlag))
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
        if((pad.Buttons & PSP_CTRL_RIGHT && !tateFlag) || (pad.Buttons & PSP_CTRL_DOWN && tateFlag))
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
�A�i���O�p�b�h�ŉ��X�N���[��
xReverse��-1�ɂ���ƃX�N���[�����������]����
�ݒ�t�@�C���������ɐݒ荀�ڂƂ���\��
*****************************/
int psp2chPadSet(int scrollX)
{
    static int xReverse = 1;

    if (tateFlag)
    {
        if (pad.Ly == 0)
        {
            scrollX += 8 * xReverse;
        }
        else if (pad.Ly == 255)
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
        if (pad.Lx == 0)
        {
            scrollX += 4 * xReverse;
        }
        else if (pad.Lx == 255)
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
�O���J���[�Z�b�g
color.ini��ǂݍ���Ŋe�J���[���Z�b�g
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

    sprintf(file, "%s/color.ini", cwDir);
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
                setColor("RES_NUMBER", resHeaderColor.num, RED);
                setColor("RES_NAME_HEAD", resHeaderColor.name1, BLACK);
                setColor("RES_NAME_BODY", resHeaderColor.name2, RGB(0x00, 0xCC, 0x00));
                setColor("RES_MAIL", resHeaderColor.mail, RGB(0x99, 0x99, 0x99));
                setColor("RES_DATE", resHeaderColor.date, BLACK);
                setColor("RES_ID_HEAD_1", resHeaderColor.id1, BLUE);
                setColor("RES_ID_HEAD_2", resHeaderColor.id2, RED);
                setColor("RES_ID_BODY", resHeaderColor.id3, BLUE);
                setColor("RES_TEXT", resColor.text, BLACK);
                setColor("RES_BG", resColor.bg, RGB(0xE0, 0xE0, 0xE0));
                setColor("RES_LINK", resColor.link, BLUE);
                setColor("RES_BAR_SLIDER", resBarColor.slider, YELLOW);
                setColor("RES_BAR_BG", resBarColor.bg, RGB(0x66, 0x66, 0xFF));

                setColor("RES_A_NUMBER", resAHeaderColor.num, RED);
                setColor("RES_A_NAME_HEAD", resAHeaderColor.name1, BLACK);
                setColor("RES_A_NAME_BODY", resAHeaderColor.name2, RGB(0x00, 0xCC, 0x00));
                setColor("RES_A_MAIL", resAHeaderColor.mail, RGB(0x99, 0x99, 0x99));
                setColor("RES_A_DATE", resAHeaderColor.date, BLACK);
                setColor("RES_A_ID_HEAD_1", resAHeaderColor.id1, BLUE);
                setColor("RES_A_ID_HEAD_2", resAHeaderColor.id2, RED);
                setColor("RES_A_ID_BODY", resAHeaderColor.id3, BLUE);
                setColor("RES_A_TEXT", resAColor.text, BLACK);
                setColor("RES_A_BG", resAColor.bg, RGB(0xFF, 0xFF, 0xCC));
                setColor("RES_A_LINK", resAColor.link, BLUE);
                setColor("RES_A_BAR_SLIDER", resABarColor.slider, RGB(0x00, 0xFF, 0xCC));
                setColor("RES_A_BAR_BG", resABarColor.bg, RGB(0xCC, 0xFF, 0xFF));

                setColor("MENU_TEXT", menuColor.text, WHITE);
                setColor("MENU_BG", menuColor.bg, BLACK);
                setColor("MENU_BATTERY_1", menuColor.bat1, GREEN);
                setColor("MENU_BATTERY_2", menuColor.bat2, YELLOW);
                setColor("MENU_BATTERY_3", menuColor.bat3, RED);

                setColor("THREAD_NUMBER", threadColor.num, RED);
                setColor("THREAD_CATEGORY", threadColor.category, RED);
                setColor("THREAD_TEXT_1", threadColor.text1, BLUE);
                setColor("THREAD_TEXT_2", threadColor.text2, RED);
                setColor("THREAD_BG", threadColor.bg, RGB(0xCC, 0xFF, 0xCC));
                setColor("THREAD_COUNT_1", threadColor.count1, BLACK);
                setColor("THREAD_COUNT_2", threadColor.count2, BLACK);
                setColor("THREAD_SELECT_NUMBER", threadColor.s_num, RED);
                setColor("THREAD_SELECT_CATEGORY", threadColor.s_category, RGB(0x99, 0x00, 0x00));
                setColor("THREAD_SELECT_TEXT_1", threadColor.s_text1, RGB(0x00, 0x00, 0x99));
                setColor("THREAD_SELECT_TEXT_2", threadColor.s_text2, RGB(0x99, 0x00, 0x00));
                setColor("THREAD_SELECT_BG", threadColor.s_bg, GRAY);
                setColor("THREAD_SELECT_COUNT_1", threadColor.s_count1, BLACK);
                setColor("THREAD_SELECT_COUNT_2", threadColor.s_count2, BLACK);

                setColor("CATE_ON_TEXT", cateOnColor.cate.text, RGB(0xCC, 0x33, 0x00));
                setColor("CATE_ON_BG", cateOnColor.cate.bg, WHITE);
                setColor("CATE_ON_S_TEXT", cateOnColor.cate.s_text, WHITE);
                setColor("CATE_ON_S_BG", cateOnColor.cate.s_bg, RGB(0xCC, 0x33, 0x00));
                setColor("ITA_OFF_TEXT", cateOnColor.ita.text, RGB(0x66, 0x66, 0xFF));
                setColor("ITA_OFF_BG", cateOnColor.ita.bg, GRAY);
                setColor("ITA_OFF_S_TEXT", cateOnColor.ita.s_text, GRAY);
                setColor("ITA_OFF_S_BG", cateOnColor.ita.s_bg, RGB(0x66, 0x66, 0xFF));
                setColor("CATE_ON_BASE", cateOnColor.base, WHITE);

                setColor("CATE_OFF_TEXT", cateOffColor.cate.text, RGB(0x88, 0x99, 0x66));
                setColor("CATE_OFF_BG", cateOffColor.cate.bg, GRAY);
                setColor("CATE_OFF_S_TEXT", cateOffColor.cate.s_text, GRAY);
                setColor("CATE_OFF_S_BG", cateOffColor.cate.s_bg, RGB(0x88, 0x99, 0x66));
                setColor("ITA_ON_TEXT", cateOffColor.ita.text, BLUE);
                setColor("ITA_ON_BG", cateOffColor.ita.bg, WHITE);
                setColor("ITA_ON_S_TEXT", cateOffColor.ita.s_text, WHITE);
                setColor("ITA_ON_S_BG", cateOffColor.ita.s_bg, BLUE);
                setColor("CATE_OFF_BASE", cateOffColor.base, WHITE);

                setColor("FORM_TITLE_TEXT", formColor.title, WHITE);
                setColor("FORM_TITLE_BG", formColor.title_bg, RED);

                setColor("MENU_WIN_TEXT", menuWinColor.text, GRAY);
                setColor("MENU_WIN_BG", menuWinColor.bg, BLACK);
                setColor("MENU_WIN_S_TEXT", menuWinColor.s_text, WHITE);
                setColor("MENU_WIN_S_BG", menuWinColor.s_bg, BLUE);
                free(buf);
                return;
            }
            else
            {
                free(buf);
            }
        }
    }
    // ini�t�@�C�����Ȃ����̃f�t�H���g
    // ���X�{��
    resHeaderColor.num = RED;
    resHeaderColor.name1 = BLACK;
    resHeaderColor.name2 = RGB(0x00, 0xCC, 0x00);
    resHeaderColor.mail = RGB(0x99, 0x99, 0x99);
    resHeaderColor.date = BLACK;
    resHeaderColor.id1 = BLUE;
    resHeaderColor.id2 = RED;
    resHeaderColor.id3 = BLUE;
    resColor.text = BLACK;
    resColor.bg = RGB(0xE0, 0xE0, 0xE0);
    resColor.link = BLUE;
    resBarColor.slider = YELLOW;
    resBarColor.bg = RGB(0x66, 0x66, 0xFF);
    // ���X�A���J�[�@
    resAHeaderColor.num = RED;
    resAHeaderColor.name1 = BLACK;
    resAHeaderColor.name2 = RGB(0x00, 0xCC, 0x00);
    resAHeaderColor.mail = RGB(0x99, 0x99, 0x99);
    resAHeaderColor.date = BLACK;
    resAHeaderColor.id1 = BLUE;
    resAHeaderColor.id2 = RED;
    resAHeaderColor.id3 = BLUE;
    resAColor.text = BLACK;
    resAColor.bg = RGB(0xFF, 0xFF, 0xCC);
    resAColor.link = BLUE;
    resABarColor.slider = RGB(0x00, 0xFF, 0xCC);
    resABarColor.bg = RGB(0xCC, 0xFF, 0xFF);
    // ���j���[�o�[�@
    menuColor.text = WHITE;
    menuColor.bg = BLACK;
    menuColor.bat1 = GREEN;
    menuColor.bat2 = YELLOW;
    menuColor.bat3 = RED;
    // �X���b�h�ꗗ
    threadColor.num = RED;
    threadColor.category = RED;
    threadColor.text1 = BLUE;
    threadColor.text2 = RED;
    threadColor.bg = RGB(0xCC, 0xFF,0xCC);
    threadColor.count1 = BLACK;
    threadColor.count2 = BLACK;
    threadColor.s_num = RED;
    threadColor.s_category = RGB(0x99, 0x00, 0x00);
    threadColor.s_text1 = RGB(0x00, 0x00, 0x99);
    threadColor.s_text2 = RGB(0x99, 0x00, 0x00);
    threadColor.s_bg = GRAY;
    threadColor.s_count1 = BLACK;
    threadColor.s_count2 = BLACK;
    // �J�e�S���[�E�ꗗ
    cateOnColor.cate.text = RGB(0xCC, 0x33, 0x00);
    cateOnColor.cate.bg = WHITE;
    cateOnColor.cate.s_text = WHITE;
    cateOnColor.cate.s_bg = RGB(0xCC, 0x33, 0x00);
    cateOnColor.ita.text = RGB(0x66, 0x66, 0xFF);
    cateOnColor.ita.bg = GRAY;
    cateOnColor.ita.s_text = GRAY;
    cateOnColor.ita.s_bg = RGB(0x66, 0x66, 0xFF);
    cateOnColor.base = WHITE;
    cateOffColor.cate.text = RGB(0x88, 0x99, 0x66);
    cateOffColor.cate.bg = GRAY;
    cateOffColor.cate.s_text = GRAY;
    cateOffColor.cate.s_bg = RGB(0x88, 0x99, 0x66);
    cateOffColor.ita.text = BLUE;
    cateOffColor.ita.bg = WHITE;
    cateOffColor.ita.s_text = WHITE;
    cateOffColor.ita.s_bg = BLUE;
    cateOffColor.base = WHITE;
    // ���M�t�H�[��
    formColor.title = WHITE;
    formColor.title_bg = RED;
    // ���j���[�E�B���h�E
    menuWinColor.text = GRAY;
    menuWinColor.bg = BLACK;
    menuWinColor.s_text = WHITE;
    menuWinColor.s_bg = BLUE;
}

/***********************************
�l�b�g���W���[���̃��[�h������
************************************/
int psp2chInit(void)
{
    int ret;

    ret = sceUtilityLoadNetModule(PSP_NET_MODULE_COMMON);
    if (ret < 0)
    {
        memset(&mh,0,sizeof(MESSAGE_HELPER));
        strcpy(mh.message, "Load module common errror");
        pspShowMessageDialog(&mh, DIALOG_LANGUAGE_AUTO);
        return ret;
    }
    ret = sceUtilityLoadNetModule(PSP_NET_MODULE_INET);
    if (ret < 0)
    {
        memset(&mh,0,sizeof(MESSAGE_HELPER));
        strcpy(mh.message, "Load module inet errror");
        pspShowMessageDialog(&mh, DIALOG_LANGUAGE_AUTO);
        return ret;
    }
    ret = Cat_NetworkInit();
    if (ret < 0)
    {
        memset(&mh,0,sizeof(MESSAGE_HELPER));
        strcpy(mh.message, "Cat_NetworkInit errror");
        pspShowMessageDialog(&mh, DIALOG_LANGUAGE_AUTO);
        return ret;
    }
    psp2chSetColor();
    return 0;
}

/**************************
�I���O�̌�n��
**************************/
int psp2chTerm(void)
{
    Cat_NetworkTerm();
    return 0;
}

/*****************************
�A�N�Z�X�|�C���g�֐ڑ����ă\�P�b�g�쐬
�������ɂ̓\�P�b�g(>=0)��Ԃ�
���s���ɂ�<0��Ԃ�
psp2chRequest()��psp2chPost()����Ă΂�܂��B
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
�\�P�b�g�����
*****************************/
int psp2chCloseSocket(int mySocket)
{
    shutdown(mySocket, SHUT_RDWR);
    return close(mySocket);
}

/*****************************
���O�����@�z�X�g����IP�A�h���X�ɕϊ�
Sfiya�L����̃p�b�`�g�p
*****************************/
int psp2chResolve(const char* host, struct in_addr* addr)
{
    SceUID fd;
    char buf[256];
    char hosts[2048];
    char *p;

    sprintf(buf, "  %s �̃A�h���X���������Ă��܂�", host);
    pgMenuBar(buf);
    sceDisplayWaitVblankStart();
    framebuffer = sceGuSwapBuffers();
    sprintf(buf, "%s/hosts", cwDir);
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
HTTP �� GET���N�G�X�g
�\�P�b�g���쐬
�A�h���X�������ă��N�G�X�g�w�b�_�𑗐M���܂�
�������ɂ̓\�P�b�g(>=0)��Ԃ��܂�
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
    sprintf(buf, "  http://%s/%s �ɐڑ����Ă��܂�", host, path);
    pgMenuBar(buf);
    sceDisplayWaitVblankStart();
    framebuffer = sceGuSwapBuffers();
    ret = connect(mySocket,(struct sockaddr *)&sain, sizeof(sain) );
    if (ret < 0) {
        memset(&mh,0,sizeof(MESSAGE_HELPER));
        strcpy(mh.message, "Connect errror");
        pspShowMessageDialog(&mh, DIALOG_LANGUAGE_AUTO);
        sceCtrlPeekBufferPositive(&oldPad, 1);
        return ret;
    }
    pgMenuBar("�ڑ����܂���");
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
HTTP �� POST ���M���܂��B
�\�P�b�g�쐬
�A�h���X�������ăw�b�_�ƃ{�f�B�𑗐M
�������ɂ̓\�P�b�g(>=0)��Ԃ�
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
    sprintf(buf, "  http://%s/test/bbs.cgi �ɐڑ����Ă��܂�", host);
    pgCopy(0, 0);
    pgMenuBar(buf);
    sceDisplayWaitVblankStart();
    framebuffer = sceGuSwapBuffers();
    ret = connect( mySocket,(struct sockaddr *)&sain, sizeof(sain) );
    if (ret < 0) {
        memset(&mh,0,sizeof(MESSAGE_HELPER));
        sprintf(mh.message, "Can't connect bbs.cgi");
        pspShowMessageDialog(&mh, DIALOG_LANGUAGE_AUTO);
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
�\�P�b�g����ŏ���1�s�i�X�e�[�^�X���C���j��ǂݍ��݂܂��B
�X�e�[�^�X�R�[�h��int�ɕϊ����ĕԂ�
HTTP/1.1 200 OK
�Ȃ�200��Ԃ�
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
�\�P�b�g���烌�X�|���X�w�b�_��ǂݍ��݂܂��B
HTTP_HEADERS�\���̂�
Content-Length
Content-Type
Last-Modified
ETag
�̓��e���R�s�[����܂��B
cookie[]��Set-Cookie�̓��e���ǉ�����܂��B
Content-Length��int�ŕԂ���܂�
Content-Length��Ԃ��Ȃ��ꍇ������̂Œ��ӁiCGI���j
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
    // �`�揈��
    // �_�C�A���O�\���̂����̔w�i��`�悷��

    minimalRender();
}

static void screen_update_callback( void* pvUserData ) {
    // �X�V����
    // �t���[���o�b�t�@�̃X���b�v�Ȃ�

    sceGuSync( 0, 0 );
    sceDisplayWaitVblankStartCB();
    framebuffer = sceGuSwapBuffers();
}

/*****************************
�w�肳�ꂽ�ݒ�ԍ��̃A�N�Z�X�|�C���g�ɐڑ����܂�
psp2chApConnect()����Ă΂�܂��B
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
        memset(&mh,0,sizeof(MESSAGE_HELPER));
        strcpy(mh.message, "sceNetApctlConnect error");
        pspShowMessageDialog(&mh, DIALOG_LANGUAGE_AUTO);
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
            memset(&mh,0,sizeof(MESSAGE_HELPER));
            sprintf(mh.message, "sceNetApctlGetState error\n0x%X", err);
            pspShowMessageDialog(&mh, DIALOG_LANGUAGE_AUTO);
            break;
        }
        if (state == 2)
        {
            strcpy(buf, "�@�A�N�Z�X�|�C���g�ɐڑ����ł��B");
            stateLast = state;
        }
        if (state == 3)
        {
            strcpy(buf, "�@IP�A�h���X���擾���ł��B");
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
        pgMenuBar("  �ڑ��Ɏ��s���܂����B");
    }
    else
    {
        pgMenuBar("  �ڑ����܂����B");
        Cat_ResolverInitEngine();
    }
    sceDisplayWaitVblankStart();
    framebuffer = sceGuSwapBuffers();
    return 0;
}

/*****************************
PSP���̃A�N�Z�X�|�C���g�ݒ���������܂�
�ݒ肪2�ȏ゠��ΑI���_�C�A���O��\�����܂��B
�ݒ肪1�݂̂̏ꍇ�͂��̒l�Őڑ����܂��B
�ݒ肪�Ȃ��ꍇ��-1���Ԃ���܂��B
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

    memset(&mh,0,sizeof(MESSAGE_HELPER));
    mh.options = PSP_UTILITY_MSGDIALOG_OPTION_TEXT;
    if (sceWlanGetSwitchState() == 0)
    {
        strcpy(mh.message, TEXT_1);
        pspShowMessageDialog(&mh, DIALOG_LANGUAGE_AUTO);
        return -1;
    }
    else if (sceWlanDevIsPowerOn() == 0)
    {
        strcpy(mh.message, TEXT_2);
        pspShowMessageDialog(&mh, DIALOG_LANGUAGE_AUTO);
        return -1;
    }
    if (sceNetApctlGetInfo(8, buf) == 0)
    {
        // IP�擾�ς�
        return 0;
    }
    pgMenuBar("�ڑ��̐ݒ���������Ă��܂�...");
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
        // �Ԃ�l < 0 : �G���[
        // �Ԃ�l = 0 : �ڑ�����
        // �Ԃ�l > 0 : �L�����Z�����ꂽ
        return Cat_NetworkConnect( draw_callback, screen_update_callback, 0 );
        // �S��0�Ȃ̂��ȁH�H�H
    }
    return -1;
}

/*****************************
OnScreenKeyboard�ŕ�������
title: OSK�̉E���ɕ\�������^�C�g��������(SJIS)
text:���͕�����ۑ�����|�C���^
num:���͕�����̒���
lines:���͕�����̍s��
OSK����̓��C�h����(UCS)�ŋA���Ă���̂�SJIS�ɕϊ����Ă��܂��B
*****************************/
void psp2chGets(char* title, char* text, int num, int lines)
{
    unsigned short* wc;
    unsigned char* p;
    char* s;
    int sjis, i;
    unsigned short string[128];
    unsigned short pretext[256];
    OSK_HELPER helper;
    char* buf = "�l������_(^o^)�^";

    if (num > 256)
    {
        num = 256;
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
    while (*p && i < 255)
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
    helper.textlimit = 100;
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
���̓_�C�A���O�\��
text1: �_�C�A���O�ɕ\�������^�C�g��
text2: OSK�ɕ\������^�C�g��
�߂�l 0=����, -1=�����
*****************/
int psp2chInputDialog(const unsigned short* text1, char* text2)
{
    int temp;

    temp = tateFlag;
    tateFlag = 0;
    keyWords[0] = '\0';
    while (running)
    {
        if(sceCtrlPeekBufferPositive(&pad, 1))
        {
            if (pad.Buttons != oldPad.Buttons)
            {
                oldPad = pad;
                if(pad.Buttons & PSP_CTRL_CIRCLE)
                {
                    psp2chGets(text2, keyWords, 128, 1);
                }
                if(pad.Buttons & PSP_CTRL_CROSS)
                {
                    tateFlag = temp;
                    return -1;
                }
                if(pad.Buttons & PSP_CTRL_SQUARE)
                {
                    break;
                }
            }
            sceGuStart(GU_DIRECT, list);
            sceGuClearDepth(0);
            sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
            pgFillvram(BLUE, 80, 60, 320, 45);
            pgEditBox(WHITE, 140, 85, 340, 100);
            pgCursorX = 142;
            pgCursorY =  87;
            pgPrint(keyWords, BLACK, WHITE, 340);
            pgCopy(0, 0);
            pgMenuBar("�@�� : ���́@�@�@�~ : �߂�@�@�@�� : ����");
            pgCursorX = 240;
            pgCursorY =  77;
            intraFontSetStyle(jpn0, 1.0f, YELLOW, 0, INTRAFONT_ALIGN_CENTER);
            intraFontPrintUCS2(jpn0, pgCursorX, pgCursorY, text1);
            sceGuFinish();
            sceGuSync(0,0);
        }
        sceDisplayWaitVblankStart();
        framebuffer = sceGuSwapBuffers();
    }
    tateFlag = temp;
    return 0;
}
