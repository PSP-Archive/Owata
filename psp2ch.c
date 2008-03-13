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
#include "psp2chMenu.h"
#include "psp2chIni.h"
#include "utf8.h"
#include "pg.h"
#include "cp932.h"
#include "intraFont.h"
#include "libCat/Cat_Network.h"
#include "libCat/Cat_Resolver.h"

extern unsigned int list[512*512]; // pg.c
extern intraFont* jpn0; // pg.c

const char* userAgent = "Monazilla/1.00 (Compatible; PSP; ja) owata(^o^)";
char* ver = "0.6.1";
S_2CH s2ch;
char cookie[128] = {0};
char keyWords[128];

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
�X�^�[�g���
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
        s2ch.pgCursorX = 450;
        s2ch.pgCursorY = 260;
        pgPrint(ver, BLUE, WHITE, SCR_WIDTH);
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
�J�[�\���ړ����[�`��
S_2CH_SCREEN�\���̂�
start:�\���J�n�s
select:�J�[�\���I���s
��ύX��R�{�^������Ԃ�
*****************************/
int psp2chCursorSet(S_2CH_SCREEN* line, int lineEnd, int shift)
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

    if(s2ch.pad.Buttons & shift)
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
        if((s2ch.pad.Buttons & s2ch.listH.up && !s2ch.tateFlag) || (s2ch.pad.Buttons & s2ch.listV.up && s2ch.tateFlag) || padUp)
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
        if((s2ch.pad.Buttons & s2ch.listH.down && !s2ch.tateFlag) || (s2ch.pad.Buttons & s2ch.listV.down && s2ch.tateFlag) || padDown)
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
        if((s2ch.pad.Buttons & s2ch.listH.top && !s2ch.tateFlag) || (s2ch.pad.Buttons & s2ch.listV.top && s2ch.tateFlag) || padUp)
        {
            if (rMenu && !padUp)
            {
                line->start = 0;
                line->select = 0;
            }
        }
        if((s2ch.pad.Buttons & s2ch.listH.end && !s2ch.tateFlag) || (s2ch.pad.Buttons & s2ch.listV.end && s2ch.tateFlag) || padDown)
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
s2ch.cfg.xReverse��-1�ɂ���ƃX�N���[�����������]����
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
        if (scrollX > BUF_HEIGHT - SCR_HEIGHT)
        {
            scrollX = BUF_HEIGHT - SCR_HEIGHT;
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
�l�b�g���W���[���̃��[�h
������
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
    psp2chIniSetColor();
    psp2chIniSetButtons();
    psp2chItaSetMenuString();
    psp2chFavSetMenuString();
    psp2chThreadSetMenuString();
    psp2chSearchSetMenuString();
    psp2chResSetMenuString();
    psp2chMenuSetMenuString();
    psp2chIniLoadConfig();
    s2ch.running = 1;
    s2ch.sel = 0;
    s2ch.tateFlag = 0;
    s2ch.logDir = "log";
    s2ch.fontDir = "font";
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
        "User-Agent: %s/%s\r\n"
        "%s"
        "Connection: close\r\n\r\n"
        , path, host, userAgent, ver, header);
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
        memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
        strcpy(s2ch.mh.message, "Connect errror");
        pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
        sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
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
        "User-Agent: %s/%s\r\n"
        "Referer: http://%s/test/read.cgi/%s/%d/l50\r\n"
        "Cookie: %s\r\n"
        "Content-Type: application/x-www-form-urlencoded\r\n"
        "Content-Length: %d\r\n\r\n"
        , host, userAgent, ver, host, dir, dat, cook, strlen(body));
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
    unsigned short pretext[2048];
    OSK_HELPER helper;
    char* buf = "�l������_(^o^)�^";

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
���̓_�C�A���O�\��
text1: �_�C�A���O�ɕ\�������^�C�g��
text2: OSK�ɕ\������^�C�g��
�߂�l 0=����, -1=�����
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
                else if(s2ch.pad.Buttons & PSP_CTRL_CROSS)
                {
                    s2ch.tateFlag = temp;
                    return -1;
                }
                else if(s2ch.pad.Buttons & PSP_CTRL_SQUARE)
                {
                    break;
                }
            }
            sceGuStart(GU_DIRECT, list);
            sceGuClearDepth(0);
            sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
            pgFillvram(BLUE, 70, 60, 340, 45);
            pgEditBox(WHITE, 140, 85, 340, 100);
            s2ch.pgCursorX = 142;
            s2ch.pgCursorY =  87;
            pgPrint(keyWords, BLACK, WHITE, 340);
            pgCopy(0, 0);
            pgMenuBar("�@�� : ���́@�@�@�~ : �߂�@�@�@�� : ����");
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
