/*
* $Id$
*/

#include <stdio.h>
#include <malloc.h>
#include "pg.h"
#include "psp2ch.h"
#include "utf8.h"

extern S_2CH s2ch; // psp2ch.c
extern char cookie[128]; // psp2ch.c

/*********************
src�������URL�G���R�[�h����dst������Ɋi�[�@
dst�̗e�ʂɒ���(src��3�{�����OK)
*********************/
void psp2chUrlEncode(char* dst, char* src)
{
    char buf[4];

    while (*src)
    {
        if ((*src >= 0x30 && *src <= 0x39) || // '0'-'9'
            (*src >= 0x41 && *src <= 0x59) || // 'A'-'Z'
            (*src >= 0x61 && *src <= 0x79) || // 'a'-'z'
            *src == '-' || *src == '.' || *src == '_')
        {
            *dst++ = *src++;
        }
        else if (*src == ' ')
        {
            *dst++ = '+';
            src++;
        }
        else
        {
            sprintf(buf, "%02X", *(unsigned char*)src++);
            *dst++ = '%';
            *dst++ = buf[0];
            *dst++ = buf[1];
        }
    }
    *dst = '\0';
}

/*********************
���X��������
*********************/
int psp2chFormResPost(char* host, char* dir, int dat, char* name, char* mail, char* message)
{
    HTTP_HEADERS resHeader;
    int ret, mySocket;
    char *encode, *buffer, *str;

    encode = (char*)malloc(2048*4);
    if (encode == NULL)
    {
        memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
        sprintf(s2ch.mh.message, "memorry error\n");
        pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
        sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
        return -1;
    }
    buffer = (char*)malloc(2048*2);
    if (buffer == NULL)
    {
        free(encode);
        memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
        sprintf(s2ch.mh.message, "memorry error\n");
        pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
        sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
        return -1;
    }
    pgCopy(0, 0);
    sceDisplayWaitVblankStart();
    framebuffer = sceGuSwapBuffers();
    strcpy(encode, "submit=%8F%91%82%AB%8D%9E%82%DE&FROM=");
    psp2chUrlEncode(buffer, name);
    strcat(encode, buffer);
    strcat(encode, "&mail=");
    psp2chUrlEncode(buffer, mail);
    strcat(encode, buffer);
    strcat(encode, "&MESSAGE=");
    psp2chUrlEncode(buffer, message);
    strcat(encode, buffer);
    sprintf(buffer, "&bbs=%s&key=%d&time=1", dir, dat);
    strcat(encode, buffer);
    free(buffer);
    // �����M����2����񂩂�Set-Cookie��PON HAP �擾
    if (cookie[0] == 0)
    {
        mySocket = psp2chPost(host, dir, dat, cookie, encode);
        if (mySocket < 0)
        {
            free(encode);
            memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
            sprintf(s2ch.mh.message, "POST error");
            pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
            sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
            return mySocket;
        }
        ret = psp2chGetStatusLine(mySocket);
        switch(ret)
        {
            case 200: // OK
                break;
            default:
                free(encode);
                memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
                sprintf(s2ch.mh.message, "Status code %d", ret);
                pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
                sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
                psp2chCloseSocket(mySocket);
                return -1;
        }
        psp2chGetHttpHeaders(mySocket, &resHeader);
        // Cookie��hana=mogera���ǉ�(encode��&hana=mogera�ǉ��ł���������)
        strcat(cookie, "; NAME=\"\"; MAIL=\"\"; hana=mogera");
        psp2chCloseSocket(mySocket);
    }
    // Cookie���Z�b�g���Ė{���M
    mySocket = psp2chPost(host, dir, dat, cookie, encode);
    if (mySocket < 0)
    {
        free(encode);
        memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
        sprintf(s2ch.mh.message, "POST error");
        pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
        sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
        return -1;
    }
    ret = psp2chGetStatusLine(mySocket);
    switch(ret)
    {
        case 200: // OK
            break;
        default:
            free(encode);
            memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
            sprintf(s2ch.mh.message, "Status code %d", ret);
            pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
            sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
            psp2chCloseSocket(mySocket);
            return -1;
    }
    psp2chGetHttpHeaders(mySocket, &resHeader);
    ret = recv(mySocket, encode, 2048*4, 0);
    encode[ret] = '\0';
    psp2chCloseSocket(mySocket);
#ifdef DEBUG
    SceUID fd;
    fd = sceIoOpen("log.txt", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
    if (fd >= 0)
    {
        sceIoWrite(fd, encode, strlen(encode));
        sceIoClose(fd);
    }
#endif
    S_2CH_RES_COLOR c;
    c.text = BLACK;
    c.bg = WHITE;
    c.link = BLUE;
    pgFillvram(WHITE, 0, 0, SCR_WIDTH, SCR_HEIGHT);
    s2ch.pgCursorX = 0;
    s2ch.pgCursorY = 0;
    str = strstr(encode, "</html");
    if (str) *str = 0;
    str = strstr(encode, "<html");
    while ((str = pgPrintHtml(str, c, 0, SCR_WIDTH, 0)))
    {
        s2ch.pgCursorX = 0;
        s2ch.pgCursorY += LINE_PITCH;
        if (s2ch.pgCursorY >= 260)
        {
            break;
        }
    }
    free(encode);
    while (s2ch.running)
    {
        pgCopy(0,0);
        pgMenuBar("��ʂ͐؂�ւ��܂���@�~�Ŗ߂��Ă�������");
        sceDisplayWaitVblankStart();
        framebuffer = sceGuSwapBuffers();
        sceCtrlPeekBufferPositive(&s2ch.pad, 1);
        if (s2ch.pad.Buttons != s2ch.oldPad.Buttons)
        {
            s2ch.oldPad = s2ch.pad;
            if(s2ch.pad.Buttons & PSP_CTRL_CROSS)
            {
                break;
            }
        }
    }
    return 1;
}

/*********************
���͉�ʕ\��
OSK�œ���
*********************/
int psp2chForm(char* host, char* dir, int dat, char* subject, char* message)
{
    static char name[64] = {0};
    static char mail[64] = {0};
    SceUID fd;
    int focus, sage;
    char buf[256];
    char  *str, *p;

    focus = 0;
    if (mail[0] == '\0' && name[0] == '\0')
    {
        sprintf(buf, "%s/%s/form.ini", s2ch.cwDir, s2ch.logDir);
        fd = sceIoOpen(buf, PSP_O_RDONLY, 0777);
        if (fd >= 0)
        {
            sceIoRead(fd, buf, 256);
            sceIoClose(fd);
            str = strchr(buf, '\n');
            *str = '\0';
            strcpy(name, buf);
            str++;
            p = strchr(str, '\n');
            *p = '\0';
            strcpy(mail, str);
        }
    }
    if (mail[0])
    {
        sage = 0;
    }
    else
    {
        sage = 1;
    }
    while (s2ch.running)
    {
        if(sceCtrlPeekBufferPositive(&s2ch.pad, 1))
        {
            if (s2ch.pad.Buttons != s2ch.oldPad.Buttons)
            {
                s2ch.oldPad = s2ch.pad;
                if(s2ch.pad.Buttons & PSP_CTRL_UP)
                {
                    focus--;
                    if (focus < 0)
                    {
                        focus = 0;
                    }
                }
                if(s2ch.pad.Buttons & PSP_CTRL_DOWN)
                {
                    focus++;
                    if (focus > 2)
                    {
                        focus = 2;
                    }
                }
                if(s2ch.pad.Buttons & PSP_CTRL_LEFT)
                {
                    if (focus == 1)
                    {
                        sage = sage ? 0 : 1;
                    }
                }
                if(s2ch.pad.Buttons & PSP_CTRL_RIGHT)
                {
                    if (focus == 1)
                    {
                        sage = sage ? 0 : 1;
                    }
                }
                if(s2ch.pad.Buttons & PSP_CTRL_CIRCLE)
                {
                    switch (focus)
                    {
                    case 0:
                        psp2chGets("���O", name, 64, 1);
                        break;
                    case 1:
                        psp2chGets("mail", mail, 64, 1);
                        break;
                    case 2:
                        psp2chGets(NULL, message, 2048, 32);
                        break;
                    }
                }
                else if(s2ch.pad.Buttons & PSP_CTRL_CROSS)
                {
                    break;
                }
                else if(s2ch.pad.Buttons & PSP_CTRL_TRIANGLE)
                {
                    memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
                    s2ch.mh.options = PSP_UTILITY_MSGDIALOG_OPTION_TEXT |
                        PSP_UTILITY_MSGDIALOG_OPTION_YESNO_BUTTONS | PSP_UTILITY_MSGDIALOG_OPTION_DEFAULT_NO;
                    strcpy(s2ch.mh.message, TEXT_6);
                    pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
                    sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
                    if (s2ch.mh.buttonPressed == PSP_UTILITY_MSGDIALOG_RESULT_YES)
                    {
                        psp2chFormResPost(host, dir, dat, name, mail, message);
                    }
                }
                else if(s2ch.pad.Buttons & PSP_CTRL_SQUARE)
                {
                }
            }
            if (sage)
            {
                strcpy(mail, "sage");
            }
            pgFillvram(RGB(0xCC, 0xFF, 0xCC), 0, 0, SCR_WIDTH, SCR_HEIGHT);
            pgFillvram(s2ch.formColor.title_bg, 0, 0, SCR_WIDTH, 15);
            s2ch.pgCursorX = 10;
            s2ch.pgCursorY = 1;
            pgPrint(subject, s2ch.formColor.title, s2ch.formColor.title_bg, SCR_WIDTH);
            s2ch.pgCursorX = 10;
            s2ch.pgCursorY = 30;
            pgPrint("�@���O�F", GRAY, RGB(0xCC, 0xFF, 0xCC), 58);
            s2ch.pgCursorX = 60;
            pgEditBox(GRAY, 58, 28, 400, 44);
            pgPrint(name, BLACK, GRAY, 400);
            s2ch.pgCursorX = 10;
            s2ch.pgCursorY = 60;
            pgPrint("���[���F", GRAY, RGB(0xCC, 0xFF, 0xCC), 58);
            s2ch.pgCursorX = 60;
            pgEditBox(GRAY, 58, 58, 300, 74);
            pgPrint(mail, BLACK, GRAY, 400);
            s2ch.pgCursorX = 310;
            s2ch.pgCursorY = 60;
            if (sage)
            {
                pgPrint("��", GRAY, RGB(0xCC, 0xFF, 0xCC), SCR_WIDTH);
            }
            else
            {
                pgPrint("��", GRAY, RGB(0xCC, 0xFF, 0xCC), SCR_WIDTH);
            }
            pgPrint("sage (�� ���L�[�Őؑ�)", GRAY, RGB(0xCC, 0xFF, 0xCC), SCR_WIDTH);
            s2ch.pgCursorX = 10;
            s2ch.pgCursorY = 90;
            pgEditBox(GRAY, 8, 88, 470, 250);
            str = message;
            while ((str = pgPrint(str, BLACK, GRAY, 470)))
            {
                s2ch.pgCursorX = 10;
                s2ch.pgCursorY += LINE_PITCH;
                if (s2ch.pgCursorY >= 250)
                {
                    break;
                }
            }
            switch (focus)
            {
            case 0:
                s2ch.pgCursorX = 10;
                s2ch.pgCursorY = 30;
                pgPrint("�@���O�F", BLACK, RGB(0xCC, 0xFF, 0xCC), 58);
                s2ch.pgCursorX = 60;
                pgEditBox(WHITE, 58, 28, 400, 44);
                pgPrint(name, BLACK, WHITE, 400);
                break;
            case 1:
                s2ch.pgCursorX = 10;
                s2ch.pgCursorY = 60;
                pgPrint("���[���F", BLACK, RGB(0xCC, 0xFF, 0xCC), 58);
                s2ch.pgCursorX = 60;
                pgEditBox(WHITE, 58, 58, 300, 74);
                pgPrint(mail, BLACK, WHITE, 300);
                s2ch.pgCursorX = 310;
                s2ch.pgCursorY = 60;
                if (sage)
                {
                    pgPrint("��", BLACK, RGB(0xCC, 0xFF, 0xCC), SCR_WIDTH);
                }
                else
                {
                    pgPrint("��", BLACK, RGB(0xCC, 0xFF, 0xCC), SCR_WIDTH);
                }
                pgPrint("sage (�� ���L�[�Őؑ�)", BLACK, RGB(0xCC, 0xFF, 0xCC), SCR_WIDTH);
                break;
            case 2:
                s2ch.pgCursorX = 10;
                s2ch.pgCursorY = 90;
                pgEditBox(WHITE, 8, 88, 470, 250);
                str = message;
                while ((str = pgPrint(str, BLACK, WHITE, 470)))
                {
                    s2ch.pgCursorX = 10;
                    s2ch.pgCursorY += LINE_PITCH;
                    if (s2ch.pgCursorY >= 250)
                    {
                        break;
                    }
                }
                break;
            }
        }
        pgCopy(0, 0);
        pgMenuBar("�@�� : ���́@�@�@�~ : �߂�@�@�@�� : ���M");
        sceDisplayWaitVblankStart();
        framebuffer = sceGuSwapBuffers();
    }
    sprintf(buf, "%s/%s/form.ini", s2ch.cwDir, s2ch.logDir);
    fd = sceIoOpen(buf, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
    if (fd >= 0)
    {
        sprintf(buf, "%s\n%s\n", name, mail);
        sceIoWrite(fd, buf, strlen(buf));
        sceIoClose(fd);
    }
    return 0;
}

