/*
* $Id$
*/

#include <stdio.h>
#include <malloc.h>
#include "pg.h"
#include "psp2ch.h"
#include "psp2chNet.h"
#include "utf8.h"

#define FORM_BG_COLOR 0x8000
#define BOX_BG_COLOR 0xA000
#define BOX_TEXT_BG_COLOR 0xFDDD

extern S_2CH s2ch; // psp2ch.c
extern unsigned short pixels[BUF_WIDTH*BUF_HEIGHT*2];
extern unsigned short winPixels[BUF_WIDTH*BUF_HEIGHT*2];
extern unsigned short* printBuf;

/*********************
src文字列をURLエンコードしてdst文字列に格納　
dstの容量に注意(srcの3倍あればOK)
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
レス書き込み
*********************/
int psp2chFormResPost(char* host, char* dir, int dat, char* name, char* mail, char* message, int tmp)
{
    S_NET net;
    int ret, x, y;
    char *encode, *buffer, *str;
    char cookie[128] = {0};

	x = s2ch.viewX;
	y = s2ch.viewY;
	s2ch.viewX = 0;
	s2ch.viewY = 0;
    encode = (char*)malloc(2048*4);
    if (encode == NULL)
    {
        psp2chErrorDialog("memorry error\n");
        return -1;
    }
    buffer = (char*)malloc(2048*2);
    if (buffer == NULL)
    {
        free(encode);
        psp2chErrorDialog("memorry error\n");
        return -1;
    }
	if (psp2chApConnect() > 0)
	{
		return -1;
	}
    // 送信しますかダイアログで画面消えてるので再描画
	printBuf = pixels;
	s2ch.tateFlag = tmp;
	pgCopy(s2ch.viewX, s2ch.viewY);
	printBuf = winPixels;
	s2ch.tateFlag = 0;
    pgCopy(0, 0);
	pgCopyMenuBar();
    sceDisplayWaitVblankStart();
    framebuffer = sceGuSwapBuffers();
	printBuf = pixels;
	s2ch.tateFlag = tmp;
	pgCopy(s2ch.viewX, s2ch.viewY);
	printBuf = winPixels;
	s2ch.tateFlag = 0;
    pgCopy(0, 0);
	pgCopyMenuBar();
    sceDisplayWaitVblankStart();
    framebuffer = sceGuSwapBuffers();
    // URLエンコードしてformデータ作成
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
    // 仮送信して2ちゃんからSet-CookieのPON HAP 取得
    memset(&net, 0, sizeof(S_NET));
    net.body = encode;
    ret = psp2chPost(host, dir, dat, cookie, &net);
	s2ch.viewX = x;
	s2ch.viewY = y;
    if (ret < 0)
    {
        free(encode);
        psp2chErrorDialog("POST error");
        return ret;
    }
    net.body = encode;
    switch(net.status)
    {
        case 200: // OK
            break;
        default:
            free(encode);
            psp2chErrorDialog("Status code %d", ret);
            return -1;
    }
    // Cookieにhana=mogeraも追加(encodeに&hana=mogera追加でもいいけど)
    strcat(cookie, "; NAME=\"\"; MAIL=\"\"; hana=mogera");
    // Cookieをセットして本送信
    ret = psp2chPost(host, dir, dat, cookie, &net);
    free(encode);
    if (ret < 0)
    {
        psp2chErrorDialog("POST error");
        return -1;
    }
    switch(net.status)
    {
        case 200: // OK
            break;
        default:
            psp2chErrorDialog("Status code %d", ret);
            return -1;
    }
#ifdef DEBUG
    SceUID fd;
    fd = sceIoOpen("log.txt", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
    if (fd >= 0)
    {
        sceIoWrite(fd, net.body, net.length);
        sceIoClose(fd);
    }
#endif
    S_2CH_RES_COLOR c;
    c.text = WHITE;
    c.bg = FORM_BG_COLOR;
    c.link = BLUE;
    pgFillvram(FORM_BG_COLOR, 0, 0, SCR_WIDTH, SCR_HEIGHT, 2);
    s2ch.pgCursorX = 0;
    s2ch.pgCursorY = 0;
    str = strstr(net.body, "</html");
    if (str) *str = 0;
    str = strstr(net.body, "<html");
    while ((str = pgPrintHtml(str, &c, 0, SCR_WIDTH, 0)))
    {
        s2ch.pgCursorX = 0;
        s2ch.pgCursorY += LINE_PITCH;
        if (s2ch.pgCursorY >= 260)
        {
            break;
        }
    }
    pgPrintMenuBar("画面は切り替わりません　○で入力画面に　×でレス表\示に戻ります");
    while (s2ch.running)
    {
		printBuf = pixels;
		s2ch.tateFlag = tmp;
		pgCopy(s2ch.viewX, s2ch.viewY);
		printBuf = winPixels;
		s2ch.tateFlag = 0;
        pgCopy(0,0);
		pgCopyMenuBar();
        sceDisplayWaitVblankStart();
        framebuffer = sceGuSwapBuffers();
        sceCtrlPeekBufferPositive(&s2ch.pad, 1);
        if (s2ch.pad.Buttons != s2ch.oldPad.Buttons)
        {
            s2ch.oldPad = s2ch.pad;
            if(s2ch.pad.Buttons & PSP_CTRL_CIRCLE)
            {
                break;
            }
            if(s2ch.pad.Buttons & PSP_CTRL_CROSS)
            {
                return 2;
            }
        }
    }
    return 1;
}

/*********************
入力画面表示
OSKで入力
*********************/
int psp2chForm(char* host, char* dir, int dat, char* subject, char* message)
{
    static char name[64] = {0};
    static char mail[64] = {0};
    SceUID fd;
    int focus, prefocus, sage, ret = 0;
    char buf[256];
    char  *str, *p, *menuStr = "　○ : 入力　　　× : 戻る　　　△ : 送信";
    int changeFlag = 0;
	int tmp;

	tmp = s2ch.tateFlag;
	s2ch.tateFlag = 0;
	printBuf = winPixels;
    focus = 0;
	prefocus = -1;
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
    pgPrintMenuBar(menuStr);
	pgFillvram(FORM_BG_COLOR, 0, 0, SCR_WIDTH, SCR_HEIGHT, 2);
	pgFillvram(s2ch.formColor.title_bg, 0, 0, SCR_WIDTH, 15, 2);
	s2ch.pgCursorX = 10;
	s2ch.pgCursorY = 1;
	pgPrint(subject, s2ch.formColor.title, s2ch.formColor.title_bg, SCR_WIDTH);
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
						prefocus = -1;
                    }
                }
                if(s2ch.pad.Buttons & PSP_CTRL_RIGHT)
                {
                    if (focus == 1)
                    {
                        sage = sage ? 0 : 1;
						prefocus = -1;
                    }
                }
                if(s2ch.pad.Buttons & PSP_CTRL_CIRCLE)
                {
                    switch (focus)
                    {
                    case 0:
                        psp2chGets("名前", name, 32, 1);
                        changeFlag = 1;
                        break;
                    case 1:
                        psp2chGets("mail", mail, 32, 1);
                        changeFlag = 1;
                        break;
                    case 2:
                        psp2chGets(NULL, message, 1024, 32);
                        break;
                    }
					prefocus = -1;
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
                        ret = psp2chFormResPost(host, dir, dat, name, mail, message, tmp);
                        if (ret == 2)
                        {
                            break;
                        }
                    }
					pgPrintMenuBar(menuStr);
					pgFillvram(FORM_BG_COLOR, 0, 0, SCR_WIDTH, SCR_HEIGHT, 2);
					pgFillvram(s2ch.formColor.title_bg, 0, 0, SCR_WIDTH, 15, 2);
					s2ch.pgCursorX = 10;
					s2ch.pgCursorY = 1;
					pgPrint(subject, s2ch.formColor.title, s2ch.formColor.title_bg, SCR_WIDTH);
					prefocus = -1;
                }
                else if(s2ch.pad.Buttons & PSP_CTRL_SQUARE)
                {
                }
				if (sage)
				{
					strcpy(mail, "sage");
				}
				if (focus != prefocus)
				{
					prefocus = focus;
					s2ch.pgCursorX = 10;
					s2ch.pgCursorY = 30;
					pgPrint("　名前：", GRAY, FORM_BG_COLOR, 58);
					s2ch.pgCursorX = 60;
					pgEditBox(BOX_BG_COLOR, 58, 28, 400, 44);
					pgPrint(name, WHITE, BOX_BG_COLOR, 400);
					s2ch.pgCursorX = 10;
					s2ch.pgCursorY = 60;
					pgPrint("メール：", GRAY, FORM_BG_COLOR, 58);
					s2ch.pgCursorX = 60;
					pgEditBox(BOX_BG_COLOR, 58, 58, 300, 74);
					pgPrint(mail, WHITE, BOX_BG_COLOR, 400);
					s2ch.pgCursorX = 310;
					s2ch.pgCursorY = 60;
					if (sage)
					{
						pgPrint("●", GRAY, FORM_BG_COLOR, SCR_WIDTH);
					}
					else
					{
						pgPrint("○", GRAY, FORM_BG_COLOR, SCR_WIDTH);
					}
					pgPrint("sage (← →キーで切替)", GRAY, FORM_BG_COLOR, SCR_WIDTH);
					s2ch.pgCursorX = 10;
					s2ch.pgCursorY = 90;
					pgEditBox(BOX_BG_COLOR, 8, 88, 470, 250);
					str = message;
					while ((str = pgPrint(str, WHITE, BOX_BG_COLOR, 470)))
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
						pgPrint("　名前：", WHITE, FORM_BG_COLOR, 58);
						s2ch.pgCursorX = 60;
						pgEditBox(BOX_TEXT_BG_COLOR, 58, 28, 400, 44);
						pgPrint(name, BLACK, BOX_TEXT_BG_COLOR, 400);
						break;
					case 1:
						s2ch.pgCursorX = 10;
						s2ch.pgCursorY = 60;
						pgPrint("メール：", WHITE, FORM_BG_COLOR, 58);
						s2ch.pgCursorX = 60;
						pgEditBox(BOX_TEXT_BG_COLOR, 58, 58, 300, 74);
						pgPrint(mail, BLACK, BOX_TEXT_BG_COLOR, 300);
						s2ch.pgCursorX = 310;
						s2ch.pgCursorY = 60;
						if (sage)
						{
							pgPrint("●", WHITE, FORM_BG_COLOR, SCR_WIDTH);
						}
						else
						{
							pgPrint("○", WHITE, FORM_BG_COLOR, SCR_WIDTH);
						}
						pgPrint("sage (← →キーで切替)", WHITE, FORM_BG_COLOR, SCR_WIDTH);
						break;
					case 2:
						s2ch.pgCursorX = 10;
						s2ch.pgCursorY = 90;
						pgEditBox(BOX_TEXT_BG_COLOR, 8, 88, 470, 250);
						str = message;
						while ((str = pgPrint(str, BLACK, BOX_TEXT_BG_COLOR, 470)))
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
					pgWaitVn(10);
				}
            }
        }
		printBuf = pixels;
		s2ch.tateFlag = tmp;
		pgCopy(s2ch.viewX, s2ch.viewY);
		printBuf = winPixels;
		s2ch.tateFlag = 0;
        pgCopy(0, 0);
		pgCopyMenuBar();
        sceDisplayWaitVblankStart();
        framebuffer = sceGuSwapBuffers();
    }
    if (changeFlag)
    {
        sprintf(buf, "%s/%s/form.ini", s2ch.cwDir, s2ch.logDir);
        fd = sceIoOpen(buf, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
        if (fd >= 0)
        {
            sprintf(buf, "%s\n%s\n", name, mail);
            sceIoWrite(fd, buf, strlen(buf));
            sceIoClose(fd);
        }
    }
	printBuf = pixels;
	s2ch.tateFlag = tmp;
    return ret;
}

