/*
* $Id$
*/

#include "psp2ch.h"
#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include <time.h>
#include <arpa/inet.h>
#include <pspwlan.h>
#include "psp2chNet.h"
#include "pg.h"
#include "utf8.h"
#include "libCat/Cat_Network.h"
#include "libCat/Cat_Resolver.h"

extern S_2CH s2ch;
extern char* ver;

static int connectThread = -1;
static int connectSleep = 1;
static int recvThread = -1;
static int recvSleep = 1;
static int recvSize = 0;
static char *recvBuf, *recvHeader, *recvBody;
static int mySocket;
static struct sockaddr_in sain;
const char* userAgent = "Monazilla/1.00 (Compatible; PSP; ja) owata(^o^)";

/*****************************
�ڑ��X���b�h
*****************************/
int psp2chConnectThread(SceSize args, void *argp)
{
	int ret;

	while(1)
	{
		if (connectSleep)
		{
			sceKernelSleepThread();
		}
		ret = connect(mySocket, (struct sockaddr *)&sain, sizeof(sain) );
		if (ret < 0)
		{
			psp2chErrorDialog("Connect errror");
		}
		connectSleep = 2;
	}
	return 0;
}

/*****************************
��M�X���b�h
*****************************/
int psp2chRecvThread(SceSize args, void *argp)
{
	char buf[256];
	int size;

	while(1)
	{
		if (recvSleep)
		{
			sceKernelSleepThread();
		}
		recvSize = 0;
		while ((size = recv(mySocket, buf, sizeof(buf), 0)) > 0)
		{
			recvBuf = (char*)realloc(recvBuf, recvSize + size);
			if (recvBuf == NULL)
			{
				psp2chCloseSocket();
				psp2chErrorDialog("memory error\nnet.body");
				recvSize = 0;
				break;
			}
			memcpy(recvBuf + recvSize, buf, size);
			recvSize += size;
		}
		recvBuf = (char*)realloc(recvBuf, recvSize + 1);
		if (recvBuf != NULL)
		{
			recvBuf[recvSize] = '\0';
		}
		recvSleep = 2;
	}
	return 0;
}

/*****************************
�l�b�g���W���[���̃��[�h
������
*****************************/
int psp2chNetInit(void)
{
    int ret;

    ret = sceUtilityLoadNetModule(PSP_NET_MODULE_COMMON);
    if (ret < 0)
    {
        psp2chErrorDialog("Load module common errror");
        return ret;
    }
    ret = sceUtilityLoadNetModule(PSP_NET_MODULE_INET);
    if (ret < 0)
    {
        psp2chErrorDialog("Load module inet errror");
        return ret;
    }
    ret = sceUtilityLoadNetModule(PSP_NET_MODULE_PARSEURI);
    if (ret < 0)
    {
        psp2chErrorDialog("Load module parseuri errror");
        return ret;
    }
    ret = sceUtilityLoadNetModule(PSP_NET_MODULE_PARSEHTTP);
    if (ret < 0)
    {
        psp2chErrorDialog("Load module parsehttp errror");
        return ret;
    }
    ret = sceUtilityLoadNetModule(PSP_NET_MODULE_HTTP);
    if (ret < 0)
    {
        psp2chErrorDialog("Load module http errror");
        return ret;
    }
    ret = sceUtilityLoadNetModule(PSP_NET_MODULE_SSL);
    if (ret < 0)
    {
        psp2chErrorDialog("Load module ssl errror");
        return ret;
    }
    ret = Cat_NetworkInit();
    if (ret < 0)
    {
        psp2chErrorDialog("Cat_NetworkInit errror\n%d", ret);
        return ret;
    }
	connectThread = sceKernelCreateThread("connect_thread", (SceKernelThreadEntry)&psp2chConnectThread, 0x12, 0x8000, 0, NULL);
	if(connectThread < 0) {
		return -1;
	}
	sceKernelStartThread(connectThread, 0, NULL);
	recvThread = sceKernelCreateThread("recv_thread", (SceKernelThreadEntry)&psp2chRecvThread, 0x18, 0x8000, 0, NULL);
	if(recvThread < 0) {
		return -1;
	}
	sceKernelStartThread(recvThread, 0, NULL);
	return 0;
}

/*****************************
�I������
*****************************/
void psp2chNetTerm(void)
{
	sceKernelDeleteThread(connectThread);
	sceKernelDeleteThread(recvThread);
    Cat_NetworkTerm();
	sceUtilityUnloadNetModule(PSP_NET_MODULE_SSL);
	sceUtilityUnloadNetModule(PSP_NET_MODULE_HTTP);
	sceUtilityUnloadNetModule(PSP_NET_MODULE_PARSEHTTP);
	sceUtilityUnloadNetModule(PSP_NET_MODULE_PARSEURI);
	sceUtilityUnloadNetModule(PSP_NET_MODULE_INET);
	sceUtilityUnloadNetModule(PSP_NET_MODULE_COMMON);
}

/*****************************
�A�N�Z�X�|�C���g�֐ڑ����ă\�P�b�g�쐬
�������ɂ̓\�P�b�g(>=0)��Ԃ�
���s���ɂ�<0��Ԃ�
psp2chRequest()��psp2chPost()����Ă΂�܂��B
*****************************/
int psp2chOpenSocket(void)
{
    int ret;

    ret = psp2chApConnect();
    sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
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
int psp2chCloseSocket(void)
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
    pgPrintMenuBar(buf);
	pgCopyMenuBar();
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
        pgPrintMenuBar("  Cat_ResolverURL error");
        sceNetApctlDisconnect();
		pgCopyMenuBar();
        sceDisplayWaitVblankStart();
        framebuffer = sceGuSwapBuffers();
        return -1;
    }
    return 0;
}

/*****************************
HTTP �� GET���N�G�X�g
�\�P�b�g���쐬
psp2chRequest();�Ń��N�G�X�g�w�b�_���M
�{����M
�\�P�b�g�����
�������ɂ�0��Ԃ�
*****************************/
int psp2chGet(const char* host, const char* path, const char* header, char* cook, S_NET* net)
{
    char requestText[512];
    char *p;

    if (psp2chOpenSocket() < 0)
    {
        return -1;
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
        , path, host, userAgent, ver, header
    );
    memset(net, 0, sizeof(S_NET));
    // ���N�G�X�g���M
    if (psp2chRequest(host, path, requestText, net) < 0)
    {
        return -1;
    }
    // ���X�|���X ��M
    if (psp2chResponse(host, path, net) < 0)
    {
        return -1;
    }
    // Status code ��M
    if ((net->status = psp2chGetStatusLine()) <= 0)
    {
        return -1;
    }
    // Response Header ��M
    psp2chGetHttpHeaders(net, cook);
    // Response Body ��M
    psp2chGetHttpBody(net);
    // Close
    psp2chCloseSocket();
    return 0;
}

/*****************************
HTTP �� POST ���M���܂��B
�\�P�b�g�쐬
psp2chRequest();�Ń��N�G�X�g�w�b�_���M
�{�f�B�𑗐M
�\�P�b�g�����
�������ɂ�0��Ԃ�
*****************************/
int psp2chPost(char* host, char* dir, int dat, char* cook, S_NET* net)
{
    char requestText[512];
    char *path = "test/bbs.cgi";

    if (psp2chOpenSocket() < 0)
    {
        return -1;
    }
    sprintf(requestText,
        "POST /%s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "User-Agent: %s/%s\r\n"
        "Referer: http://%s/test/read.cgi/%s/%d/l50\r\n"
        "Cookie: %s\r\n"
        "Content-Type: application/x-www-form-urlencoded\r\n"
        "Content-Length: %d\r\n\r\n"
        , path, host, userAgent, ver, host, dir, dat, cook, strlen(net->body)
    );
    // ���N�G�X�g���M
    if (psp2chRequest(host, path, requestText, net) < 0)
    {
        return -1;
    }
    // �{�����M
    send(mySocket, net->body, strlen(net->body), 0 );
    // ���X�|���X ��M
    if (psp2chResponse(host, path, net) < 0)
    {
        return -1;
    }
    // Status code ��M
    if ((net->status = psp2chGetStatusLine()) <= 0)
    {
        return -1;
    }
    // Response Header ��M
    psp2chGetHttpHeaders(net, cook);
    // Response Body ��M
    psp2chGetHttpBody(net);
    // Close
    psp2chCloseSocket();
    return 0;
}

/*****************************
�A�h���X�������ă��N�G�X�g�w�b�_�𑗐M���܂�
�������ɂ�0��Ԃ��܂�
*****************************/
int psp2chRequest(const char* host, const char* path, const char* requestText, S_NET* net)
{
    int ret;
    char buf[512];
    struct in_addr addr;

    ret = psp2chResolve(host, &addr);
    if (ret < 0) {
        return ret;
    }
    sprintf(buf, "  %s (%s)", host, inet_ntoa(addr));
    pgPrintMenuBar(buf);
	pgCopyMenuBar();
    sceDisplayWaitVblankStart();
    framebuffer = sceGuSwapBuffers();
    // Tell the socket to connect to the IP address we found, on port 80 (HTTP)
    sain.sin_family = AF_INET;
    sain.sin_port = htons(80);
    sain.sin_addr.s_addr = addr.s_addr;
    sprintf(buf, "  http://%s/%s �ɐڑ����Ă��܂�", host, path);
    pgPrintMenuBar(buf);
	pgWaitVn(5);
	pgCopyMenuBar();
    sceDisplayWaitVblankStart();
    framebuffer = sceGuSwapBuffers();
	connectSleep = 0;
	sceKernelWakeupThread(connectThread);
    while (1)
    {
		if (connectSleep == 2)
		{
			break;
		}
		sceCtrlPeekBufferPositive(&s2ch.pad, 1);
		if (s2ch.pad.Buttons & PSP_CTRL_CROSS)
		{
			s2ch.oldPad = s2ch.pad;
			connectSleep = 1;
			sceKernelTerminateThread(connectThread);
			sceKernelStartThread(connectThread, 0, NULL);
			return -1;
		}
		sceKernelDelayThread(1000);
    }
    pgPrintMenuBar("�ڑ����܂���");
	pgWaitVn(5);
	pgCopyMenuBar();
    sceDisplayWaitVblankStart();
    framebuffer = sceGuSwapBuffers();
    // send our request
    send(mySocket, requestText, strlen(requestText), 0 );
    return 0;
}

/*****************************
Response ����M
*****************************/
int psp2chResponse(const char* host, const char* path, S_NET* net)
{
    char buf[512];

    sprintf(buf, "http://%s/%s ����f�[�^��]�����Ă��܂�...", host, path);
    pgPrintMenuBar(buf);
	pgWaitVn(5);
	pgCopyMenuBar();
    sceDisplayWaitVblankStart();
    framebuffer = sceGuSwapBuffers();
	// ��M�X���b�h���N����
	recvSleep = 0;
	sceKernelWakeupThread(recvThread);
    while (1)
    {
		if (recvSleep == 2)
		{
			break;
		}
		sceCtrlPeekBufferPositive(&s2ch.pad, 1);
		if (s2ch.pad.Buttons & PSP_CTRL_CROSS)
		{
			s2ch.oldPad = s2ch.pad;
			recvSleep = 1;
			sceKernelTerminateThread(recvThread);
			sceKernelStartThread(recvThread, 0, NULL);
			recvSize = 0;
			free(recvBuf);
			recvBuf = NULL;
			break;
		}
		sceKernelDelayThread(1000);
    }
	if (recvBuf == NULL)
	{
		return -1;
	}
    sprintf(buf, "����(%dBytes)", recvSize);
    pgPrintMenuBar(buf);
	pgWaitVn(5);
	// 1�s�ڂ̃X�e�[�^�X���C���ŋ�؂�
	recvHeader = strstr(recvBuf, "\r\n");
	if (recvHeader == NULL)
	{
		return -1;
	}
	*recvHeader = '\0';
	recvSize -= strlen(recvBuf);
	recvSize -= 2;
	recvHeader += 2;
	// �w�b�_�ƃ{�f�B�̊Ԃ̋�s�ŋ�؂�
	recvBody = strstr(recvHeader, "\r\n\r\n");
	if (recvBody == NULL)
	{
		return -1;
	}
	recvBody += 2;
	*recvBody = '\0';
	recvSize -= strlen(recvHeader);
	recvSize -= 2;
    net->length = recvSize;
	recvBody += 2;
	pgCopyMenuBar();
    sceDisplayWaitVblankStart();
    framebuffer = sceGuSwapBuffers();
    return 0;
}

/*****************************
�\�P�b�g����ŏ���1�s�i�X�e�[�^�X���C���j��ǂݍ��݂܂��B
�X�e�[�^�X�R�[�h��int�ɕϊ����ĕԂ�
HTTP/1.1 200 OK
�Ȃ�200��Ԃ�
*****************************/
int psp2chGetStatusLine(void)
{
    int verMajor, verMinor, code;
    char phrase[256];

    pgPrintMenuBar(recvBuf);
	pgCopyMenuBar();
    sceDisplayWaitVblankStart();
    framebuffer = sceGuSwapBuffers();
    sscanf(recvBuf, "HTTP/%d.%d %d %s\r\n", &verMajor, &verMinor, &code, phrase);
    return code;
}

/*****************************
�\�P�b�g���烌�X�|���X�w�b�_��ǂݍ��݂܂��B
S_NET�\���̂�
Content-Length
Content-Type
Last-Modified
ETag
�̓��e���R�s�[����܂��B
cookie[]��Set-Cookie�̓��e���ǉ�����܂��B
Content-Length��int�ŕԂ���܂�
Content-Length��Ԃ��Ȃ��ꍇ������̂Œ��ӁiCGI���j
*****************************/
int psp2chGetHttpHeaders(S_NET* net, char* cookie)
{
    int contentLength = 0;
    char *line, *p, *q;

	line = recvHeader;
    while(*line)
    {
		q = strstr(line, "\r\n");
		if (q == NULL)
		{
			return -1;
		}
		*q = '\0';
		/*
		pgPrintMenuBar(line);
		pgWaitVn(10);
		pgCopyMenuBar();
		sceDisplayWaitVblankStart();
		framebuffer = sceGuSwapBuffers();
		*/
        if (strstr(line, "Content-Length:"))
        {
            sscanf(line, "Content-Length: %d", &contentLength);
            net->head.Content_Length = contentLength;
        }
        else if (strstr(line, "Content-Type:"))
        {
            strcpy(net->head.Content_Type, &line[14]);
        }
        else if (strstr(line, "Last-Modified:"))
        {
            strcpy(net->head.Last_Modified, &line[15]);
        }
        else if (strstr(line, "ETag:"))
        {
            strcpy(net->head.ETag, &line[6]);
        }
        else if (cookie && strstr(line, "Set-Cookie:"))
        {
            p = strchr(line, ';');
            *p = '\0';
            if (cookie[0])
            {
                strcat(cookie, "; ");
            }
            strcat(cookie, &line[12]);
        }
		line = q + 2;
    }
    return contentLength;
}

/*****************************
�\�P�b�g���烌�X�|���X�{�̂�ǂݍ���
*****************************/
int psp2chGetHttpBody(S_NET* net)
{
    net->body = recvBody;
	return 0;
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
        psp2chErrorDialog("sceNetApctlConnect error");
        return 0;
    }

    pgPrintMenuBar("  Connecting...");
	pgCopyMenuBar();
    sceDisplayWaitVblankStart();
    framebuffer = sceGuSwapBuffers();
    time = clock();
    while (1)
    {
        err = sceNetApctlGetState(&state);
        if (err != 0)
        {
            psp2chErrorDialog("sceNetApctlGetState error\n0x%X", err);
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
        pgPrintMenuBar(buf);
		pgCopyMenuBar();
        sceDisplayWaitVblankStart();
        framebuffer = sceGuSwapBuffers();
        if (clock() > time + 1000000 * 8)
        {
            break;
        }
    }
    if (state != 4)
    {
        pgPrintMenuBar("  �ڑ��Ɏ��s���܂����B");
    }
    else
    {
        pgPrintMenuBar("  �ڑ����܂����B");
        Cat_ResolverInitEngine();
    }
	pgCopyMenuBar();
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
        psp2chErrorDialog(TEXT_1);
        return -1;
    }
    else if (sceWlanDevIsPowerOn() == 0)
    {
        psp2chErrorDialog(TEXT_2);
        return -1;
    }
    if (sceNetApctlGetInfo(8, buf) == 0)
    {
        // IP�擾�ς�
        return 0;
    }
    pgPrintMenuBar("�ڑ��̐ݒ���������Ă��܂�...");
	pgCopyMenuBar();
    sceDisplayWaitVblankStart();
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
        i = Cat_NetworkConnect( draw_callback, screen_update_callback, 0 );
        // �S��0�Ȃ̂��ȁH�H�H
    }
    return i;
}
