/*
* $Id$
*/

#include <stdio.h>
#include <malloc.h>
#include "psp2ch.h"
#include "pg.h"
#include "psp2chTinyBrowser.h"
#include "cp932.h"

extern S_2CH s2ch; // psp2ch.c
extern int preLine; // psp2chRes.c
extern unsigned int pixels[BUF_WIDTH*BUF_HEIGHT]; // pg.c
extern unsigned int winPixels[BUF_WIDTH*BUF_HEIGHT]; // pg.c
extern unsigned int* printBuf; // pg.c

/**************
HTML表示
***************/
int psp2chTinyBrowser(char* path)
{
    SceUID fd;
    SceIoStat st;
    int ret, change;
    char *txt, *bck;
    S_2CH_SCREEN html;
    S_SCROLLBAR bar;
    int startX, startY, scrX, scrY, lineEnd, barW, count;
    char menu[128];
    char* p;
    char** line;
    char* codeStr[] = {"SJIS", "EUC", "UTF-8"};
    int code = 0;

    ret = sceIoGetstat(path, &st);
    if (ret < 0)
    {
        psp2chErrorDialog("Getstat error\n%s", path);
        return -1;
    }
    txt = (char*)malloc(st.st_size + 1);
    if (txt == NULL)
    {
        psp2chErrorDialog("memorry error\txt");
        return -1;
    }
    bck = (char*)malloc(st.st_size + 1);
    if (bck == NULL)
    {
        psp2chErrorDialog("memorry error\bck");
        return -1;
    }
    fd = sceIoOpen(path, PSP_O_RDONLY, 0777);
    if (fd < 0)
    {
        free(bck);
        free(txt);
        psp2chErrorDialog("Open error\n%s", path);
        return -1;
    }
    else
    {
        sceIoRead(fd, bck, st.st_size);
        sceIoClose(fd);
    }
    bck[st.st_size] = '\0';
    if (s2ch.tateFlag)
    {
        startX = RES_A_X_V;
        scrX = RES_A_WIDTH_V;
        startY = RES_A_Y_V;
        scrY = RES_A_HEIGHT_V;
        lineEnd = RES_A_LINE_V;
        barW = RES_BAR_WIDTH_V;
        bar.view = RES_A_HEIGHT_V;
        bar.x = startX + scrX;
        bar.y = RES_A_Y_V;
        bar.w = RES_BAR_WIDTH_V;
        bar.h = RES_A_HEIGHT_V;
    }
    else
    {
        startX = RES_A_X;
        scrX = RES_A_WIDTH;
        startY = RES_A_Y;
        scrY = RES_A_HEIGHT;
        lineEnd = RES_A_LINE;
        barW = RES_BAR_WIDTH;
        bar.view = RES_A_HEIGHT;
        bar.x = startX + scrX;
        bar.y = RES_A_Y;
        bar.w = RES_BAR_WIDTH;
        bar.h = RES_A_HEIGHT;
    }
    psp2chRenderHtml(txt, bck, code);
/*
    fd = sceIoOpen("log.txt", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
    if (fd >= 0)
    {
        sceIoWrite(fd, txt, strlen(txt));
        sceIoClose(fd);
    }
*/
    s2ch.pgCursorX = 0;
    p = txt;
    count = 0;
    while ((p = psp2chPrintText(p, s2ch.resAColor, scrX, code, 0)))
    {
        s2ch.pgCursorX = 0;
        count++;
    }
    count++;
    line = (char**)malloc(sizeof(char*) * count);
    if (line == NULL)
    {
        psp2chErrorDialog("memorry error\nline");
        return -1;
    }
    p = txt;
    count = 0;
    line[count] = p;
    while ((p = psp2chPrintText(p, s2ch.resAColor, scrX, code, 0)))
    {
        s2ch.pgCursorX = 0;
        count++;
        line[count] = p;
    }
    count++;
    if (count < lineEnd)
    {
        count = lineEnd;
    }
    html.count = count;
    bar.total = count * LINE_PITCH;
    bar.start = 0;
    html.start = 0;
    html.select = 0;
    printBuf = winPixels;
    while (s2ch.running)
    {
        if(sceCtrlPeekBufferPositive(&s2ch.pad, 1))
        {
            if (s2ch.tateFlag)
            {
                psp2chCursorSet(&html, lineEnd, s2ch.btnResV.change, &change);
            }
            else
            {
                psp2chCursorSet(&html, lineEnd, s2ch.btnResH.change, &change);
            }
            if (html.select > html.count - lineEnd)
            {
                html.select = html.count - lineEnd;
            }
            html.start = html.select;
            if (s2ch.pad.Buttons != s2ch.oldPad.Buttons)
            {
                s2ch.oldPad = s2ch.pad;
                if(s2ch.pad.Buttons & PSP_CTRL_CIRCLE)
                {
                    code++;
                    if (code > 2)
                    {
                        code = 0;
                    }
                    psp2chRenderHtml(txt, bck, code);
                    s2ch.pgCursorX = 0;
                    p = txt;
                    count = 0;
                    while ((p = psp2chPrintText(p, s2ch.resAColor, scrX, code, 0)))
                    {
                        s2ch.pgCursorX = 0;
                        count++;
                    }
                    count++;
                    free(line);
                    line = (char**)malloc(sizeof(char*) * count);
                    if (line == NULL)
                    {
                        psp2chErrorDialog("memorry error\nline");
                        free(bck);
                        free(txt);
                        printBuf = pixels;
                        preLine = -2;
                        return -1;
                    }
                    p = txt;
                    count = 0;
                    line[count] = p;
                    while ((p = psp2chPrintText(p, s2ch.resAColor, scrX, code, 0)))
                    {
                        s2ch.pgCursorX = 0;
                        count++;
                        line[count] = p;
                    }
                    count++;
                    if (count < lineEnd)
                    {
                        count = lineEnd;
                    }
                    html.count = count;
                    bar.total = count * LINE_PITCH;
					sprintf(menu, "　○ : コード(%s)　　× : 戻る　　□ : 削除", codeStr[code]);
					pgPrintMenuBar(menu);
                }
                if(s2ch.pad.Buttons & PSP_CTRL_CROSS)
                {
                    break;
                }
                if(s2ch.pad.Buttons & PSP_CTRL_SQUARE)
                {
                    sceIoRemove(path);
                    break;
                }
				change = 1;
            }
			if (change)
			{
	            psp2chDrawHtml(line[html.start], html, code);
			}
            pgCopyWindow(html.start * LINE_PITCH, startX, startY, scrX, scrY);
            pgWindowFrame(startX, startY, startX + scrX + barW, startY + scrY);
            bar.start = html.start * LINE_PITCH;
            pgScrollbar(bar, s2ch.resABarColor);
            pgCopyMenuBar();
            sceDisplayWaitVblankStart();
            framebuffer = sceGuSwapBuffers();
        }
    }
    free(line);
    free(bck);
    free(txt);
    printBuf = pixels;
    preLine = -2;
    return 0;
}

/**************
HTML変換
bckのタグを処理して
txtに結果を保存
code 文字コード(0:sjis, 1:euc-jp, 2:utf-8)
***************/
int psp2chRenderHtml(char* txt, char* bck, int code)
{
    unsigned char *p, *q;

    p = (unsigned char*)bck;
    q = (unsigned char*)txt;
    while (*p)
    {
        if (*p == '<')
        {
            psp2chParseTag(&p, &q, code);
        }
        else if (*p <= ' ')
        {
            while (*p && *p <= ' ')
            {
                p++;
            }
            *q++ = ' ';
        }
        else
        {
            *q++ = *p++;
        }
    }
    *q++ = '\n';
    *q = '\0';
    return 0;
}

/**************
タグ解析
***************/
int psp2chParseTag(unsigned char** src, unsigned char** dst, int code)
{
    unsigned char *p, *q;
    char element[64];
    int i;

    p = *src;
    q = *dst;
    p++;
    // スペース除去
    while (*p && *p <= ' ')
    {
        p++;
    }
    // 英文字と/,!以外はリターン
    if ((*p < 'A' &&  *p != '/' && *p != '!') || (*p > 'Z' && *p < 'a') || *p > 'z')
    {
        while (*p && *p != '>')
        {
            p++;
        }
        if (*p)
        {
            p++;
        }
        *src = p;
        *dst = q;
        return -1;
    }
    // 要素を読み込む
    i = 0;
    while ((*p >= 'A' && *p <= 'Z') || (*p >= 'a' && *p <= 'z') || (*p >= '0' && *p <= '9') || *p == '/' || *p == '!' || *p == '-')
    {
        if (*p >= 'A' && *p <= 'Z')
        {
            *p += 'a' - 'A';
        }
        element[i] = *p++;
        i++;
        if (i >= 64)
        {
            break;
        }
    }
    element[i] = '\0';
    // タグ処理
    if (strcmp(element, "br") == 0)
    {
        *q++ = '\n';
    }
    else if (strcmp(element, "hr") == 0)
    {
        *q++ = '\n';
        *q++ = '+';
        *q++ = '+';
        *q++ = '\n';
    }
    else if (strcmp(element, "tr") == 0)
    {
        *q++ = '\n';
        *q++ = ' ';
        *q++ = ' ';
    }
    else if (strcmp(element, "th") == 0)
    {
        *q++ = ' ';
        *q++ = '|';
        *q++ = ' ';
    }
    else if (strcmp(element, "td") == 0)
    {
        *q++ = ' ';
        *q++ = '|';
        *q++ = ' ';
    }
    else if (strcmp(element, "/tr") == 0)
    {
        *q++ = ' ';
        *q++ = '|';
    }
    else if (strcmp(element, "li") == 0)
    {
        *q++ = '\n';
        *q++ = ' ';
        switch (code)
        {
        case 0:
            *q++ = 0x81; // ・
            *q++ = 0x45;
            break;
        case 1:
            *q++ = 0xA1;
            *q++ = 0xA6;
            break;
        case 2:
            *q++ = 0xE3;
            *q++ = 0x83;
            *q++ = 0xBB;
            break;
        }
    }
    else if (strcmp(element, "/ul") == 0)
    {
        *q++ = '\n';
    }
    else if (strcmp(element, "dt") == 0)
    {
        *q++ = '\n';
        *q++ = '*';
    }
    else if (strcmp(element, "dd") == 0)
    {
        *q++ = '\n';
        *q++ = ' ';
        *q++ = '-';
        *q++ = ' ';
    }
    else if (strcmp(element, "/dl") == 0)
    {
        *q++ = '\n';
    }
    else if (strcmp(element, "p") == 0)
    {
        *q++ = '\n';
    }
    else if (strcmp(element, "/p") == 0)
    {
        *q++ = '\n';
    }
    else if (strcmp(element, "/div") == 0)
    {
        *q++ = '\n';
    }
    else if (strcmp(element, "h1") == 0)
    {
        *q++ = '\n';
        *q++ = '[';
        *q++ = '[';
    }
    else if (strcmp(element, "/h1") == 0)
    {
        *q++ = ']';
        *q++ = ']';
        *q++ = '\n';
    }
    else if (strcmp(element, "h2") == 0)
    {
        *q++ = '\n';
        *q++ = '[';
    }
    else if (strcmp(element, "/h2") == 0)
    {
        *q++ = ']';
        *q++ = '\n';
    }
    else if (strcmp(element, "h3") == 0)
    {
        *q++ = '\n';
        *q++ = '{';
    }
    else if (strcmp(element, "/h3") == 0)
    {
        *q++ = '}';
        *q++ = '\n';
    }
    else if (strcmp(element, "h4") == 0)
    {
        *q++ = '\n';
        *q++ = '(';
    }
    else if (strcmp(element, "/h4") == 0)
    {
        *q++ = ')';
        *q++ = '\n';
    }
    else if (strcmp(element, "img") == 0)
    {
        *q++ = '[';
        *q++ = 'I';
        *q++ = 'M';
        *q++ = 'G';
        *q++ = ']';
    }
    else if (strstr(element, "!--") == element)
    {
        p = (unsigned char*)strstr((char*)p, "-->");
        if (p == NULL)
        {
            p = *src;
        }
    }
    else if (strcmp(element, "script") == 0)
    {
        p = (unsigned char*)strstr((char*)p, "/script");
        if (p == NULL)
        {
            p = *src;
        }
    }
    // 終了
    while (*p && *p != '>')
    {
        p++;
    }
    if (*p)
    {
        p++;
    }
    *src = p;
    *dst = q;
    return 0;
}

/*****************************
HTML描画
*****************************/
void psp2chDrawHtml(char* txt, S_2CH_SCREEN html, int code)
{
    int line = 0;
    int startX, startY, scrW, lineEnd;

    if (s2ch.tateFlag)
    {
        startX = RES_A_X_V;
        scrW = RES_A_WIDTH_V;
        startY = RES_A_Y_V+1;
        lineEnd = RES_A_LINE_V;
    }
    else
    {
        startX = RES_A_X;
        scrW = RES_A_WIDTH;
        startY = RES_A_Y;
        lineEnd = RES_A_LINE;
    }
    s2ch.pgCursorY = (LINE_PITCH * html.start + startY) & 0x01FF;
    line = 0;
    s2ch.pgCursorX = startX;
    pgFillvram(s2ch.resAColor.bg, startX, s2ch.pgCursorY, scrW, LINE_PITCH * lineEnd, 2);
    while (line++ <= lineEnd && txt)
    {
        txt = psp2chPrintText(txt, s2ch.resAColor, scrW+startX, code, 1);
        s2ch.pgCursorX = startX;
        s2ch.pgCursorY += LINE_PITCH;
    }
}

/*****************************
strを画面幅widthで表示
view:0 表示なし
*****************************/
char* psp2chPrintText(char *str, S_2CH_RES_COLOR c, int width, int code, int view)
{
    unsigned char ch = 0,bef = 0, u1, u2;
    int ret = 0, sjis;
    int tcolor;

    if (str == NULL)
    {
        return NULL;
    }
    tcolor = c.text;
    while(*str)
    {
        ch = (unsigned char)*str;
        if (bef!=0)
        {
            if (view)
            {
                ret = pgPutCharW2(bef, ch, tcolor, c.bg, width, code);
            }
            else
            {
                ret = pgCountCharW2(bef, ch, width, code);
            }
            if (ret)
            {
                return --str;
            }
            bef=0;
        }
        else
        {
            if ((code == 0) && (((ch>=0x80) && (ch<0xa0)) || (ch>=0xe0)))
            {
                bef = ch;
            }
            else if ((code == 1) && (ch>=0x80))
            {
                bef = ch;
            }
            else if ((code == 2) && (ch>=0x80))
            {
                u1 = 0;
                u2 = 0;
                // UCS-2に
                if ((ch & 0xE0) == 0xC0)
                {
                    ch &= 0x1F;
                    u1 = ch >> 2;
                    u2 = ch << 6;
                    str++;
                    if ((ch = *str) == 0)
                    {
                        return NULL;
                    }
                    u2 |= (ch & 0x3F);
                    bef = 1;
                }
                else if ((ch & 0xF0) == 0xE0)
                {
                    ch &= 0x0F;
                    u1 = ch << 4;
                    str++;
                    if ((ch = *str) == 0)
                    {
                        return NULL;
                    }
                    ch &= 0x3F;
                    u1 |= ch >> 2;
                    u2 = ch << 6;
                    str++;
                    if ((ch = *str) == 0)
                    {
                        return NULL;
                    }
                    u2 |= (ch & 0x3F);
                    bef = 2;
                }
                // 半角カナ
                if (u1 == 0xFF && (u2 > 0x60 && u2 < 0xA0))
                {
                    if (view)
                    {
                        ret = pgPutCharA(u2 + 0x40, tcolor, c.bg, width);
                    }
                    else
                    {
                        ret = pgCountCharA(u2 + 0x40, width);
                    }
                }
                // sjisに
                else
                {
                    sjis = (u1 << 8) | u2;
                    sjis = conv_wchar_sjiswin(sjis);
                    if (sjis)
                    {
                        u2 = sjis >> 8;
                        u1 = sjis & 0xFF;
                        if (view)
                        {
                            ret = pgPutCharW(u1, u2, tcolor, c.bg, width);
                        }
                        else
                        {
                            ret = pgCountCharW(u1, u2, width);
                        }
                    }
                    else
                    {
                        if (view)
                        {
                            ret = pgPutCharA('?', tcolor, c.bg, width);
                        }
                        else
                        {
                            ret = pgCountCharA('?', width);
                        }
                    }
                }
                if (ret)
                {
                    str -= bef;
                    return str;
                }
                bef = 0;
            }
            else
            {
                if (ch == '&')
                {
                    if(view)
                    {
                        ret = pgSpecialChars((char**)(&str), tcolor, c.bg, width);
                    }
                    else
                    {
                        ret = pgCountSpecialChars((char**)(&str), width);
                    }
                }
                else if (ch == '\n') {
                    str++;
                    return str;
                }
                else
                {
                    if (view)
                    {
                        ret = pgPutCharA(ch, tcolor, c.bg, width);
                    }
                    else
                    {
                        ret = pgCountCharA(ch, width);
                    }
                }
                if (ret)
                {
                    return str;
                }
            }
        }
        str++;
    }
    return NULL;
}
