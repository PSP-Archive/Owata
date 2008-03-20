
#include <stdio.h>
#include <malloc.h>
#include "psp2ch.h"
#include "pg.h"
#include "psp2chTinyBrowser.h"

extern S_2CH s2ch; // psp2ch.c
extern int preLine; // psp2chRes.c
extern unsigned int pixels[BUF_WIDTH*BUF_HEIGHT]; // pg.c
extern unsigned int winPixels[BUF_WIDTH*BUF_HEIGHT]; // pg.c
extern unsigned int* printBuf; // pg.c

int psp2chTinyBrowser(char* path)
{
    SceUID fd;
    SceIoStat st;
    int ret;
    char* txt;

    ret = sceIoGetstat(path, &st);
    if (ret < 0)
    {
        memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
        sprintf(s2ch.mh.message, "Getstat error\n%s", path);
        pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
        sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
        return -1;
    }
    txt = (char*)malloc(st.st_size + 1);
    if (txt == NULL)
    {
        memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
        strcpy(s2ch.mh.message, "memorry error\txt");
        pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
        sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
        return -1;
    }
    fd = sceIoOpen(path, PSP_O_RDONLY, 0777);
    if (fd < 0)
    {
        free(txt);
        memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
        sprintf(s2ch.mh.message, "Open error\n%s", path);
        pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
        sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
        return -1;
    }
    else
    {
        sceIoRead(fd, txt, st.st_size);
        sceIoClose(fd);
    }
    txt[st.st_size] = '\0';
    //pspDebugScreenInit();
    //pspDebugScreenPrintf("Here");
    //pgWaitVn(200);
    if (psp2chRenderHtml(txt) < 0)
    {
        return -1;
    }
    /*
    fd = sceIoOpen("log.txt", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
    if (fd >= 0)
    {
        sceIoWrite(fd, txt, strlen(txt));
        sceIoClose(fd);
    }
    */
    psp2chViewHtml(txt, path);
    free(txt);
    return 0;
}

/**************
HTML変換
***************/
int psp2chRenderHtml(char* txt)
{
    unsigned char *buf, *p, *q;

    buf = (unsigned char*)malloc(strlen(txt));
    if (buf == NULL)
    {
        memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
        strcpy(s2ch.mh.message, "memorry error\buf");
        pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
        sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
        return -1;
    }
    p = (unsigned char*)txt;
    q = (unsigned char*)buf;
    while (*p)
    {
        if (*p == '<')
        {
            psp2chParseTag(&p, &q);
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
    *q = '\0';
    strcpy(txt, (char*)buf);
    free(buf);
    return 0;
}

/**************
タグ解析
***************/
int psp2chParseTag(unsigned char** src, unsigned char** dst)
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
        *q++ = 0x81; // ・
        *q++ = 0x45;
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

/**************
HTML表示
***************/
void psp2chViewHtml(char* txt, char* path)
{
    int count, lineFlag, drawLine;
    S_2CH_SCREEN html;
    S_SCROLLBAR bar;
    int startX, startY, scrX, scrY, lineEnd, barW;
    char* p;

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
    count = 0;
    s2ch.pgCursorX = 0;
    p = txt;
    while ((p = psp2chCountText(p, scrX)))
    {
        s2ch.pgCursorX = 0;
        count++;
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
    drawLine = html.select;
    while (s2ch.running)
    {
        if(sceCtrlPeekBufferPositive(&s2ch.pad, 1))
        {
            if (s2ch.tateFlag)
            {
                psp2chCursorSet(&html, lineEnd, s2ch.btnResV.change);
            }
            else
            {
                psp2chCursorSet(&html, lineEnd, s2ch.btnResH.change);
            }
            if (html.select > html.count - lineEnd)
            {
                html.select = html.count - lineEnd;
            }
            html.start = html.select;
            if(html.select == drawLine)
            {
                lineFlag = 0;
            }
            else if(html.select == drawLine + 1)
            {
                lineFlag = 1;
            }
            else if(html.select == drawLine - 1)
            {
                lineFlag = 2;
            }
            else
            {
                lineFlag = 3;
            }
            drawLine = html.select;
            if (s2ch.pad.Buttons != s2ch.oldPad.Buttons)
            {
                s2ch.oldPad = s2ch.pad;
                if(s2ch.pad.Buttons & PSP_CTRL_CROSS)
                {
                    break;
                }
                if(s2ch.pad.Buttons & PSP_CTRL_SQUARE)
                {
                    sceIoRemove(path);
                    break;
                }
            }
            psp2chDrawHtml(txt, html, lineFlag);
            pgCopyWindow(html.start * LINE_PITCH, startX, startY, scrX, scrY);
            pgWindowFrame(startX, startY, startX + scrX + barW, startY + scrY);
            bar.start = html.start * LINE_PITCH;
            pgScrollbar(bar, s2ch.resABarColor);
            pgMenuBar("　× : 戻る　　　□ : 削除");
            sceDisplayWaitVblankStart();
            framebuffer = sceGuSwapBuffers();
        }
    }
    printBuf = pixels;
    preLine = -2;
    return;
}

/*****************************
HTML描画
lineFlag:0=変更なし; 1=下に1行追加; 2=上に1行追加; その他:全画面再描画
*****************************/
void psp2chDrawHtml(char* txt, S_2CH_SCREEN html, int lineFlag)
{
    int skip;
    int line = 0, drawLine;
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
    drawLine = html.select;
    if (lineFlag == 0 && drawLine)
    {
        return;
    }
    else if (lineFlag == 1)
    {
        drawLine += lineEnd-1;
        skip = drawLine;
        s2ch.pgCursorY = (LINE_PITCH*drawLine+startY)&0x01FF;
        while (skip-- > 0 && txt)
        {
            s2ch.pgCursorX = 0;
            txt = psp2chCountText(txt, scrW);
        }
        s2ch.pgCursorX = startX;
        pgFillvram(s2ch.resAColor.bg, startX, s2ch.pgCursorY, scrW, LINE_PITCH);
        psp2chPrintText(txt, s2ch.resAColor, scrW+startX);
    }
    else if (lineFlag == 2)
    {
        skip = drawLine;
        s2ch.pgCursorY = (LINE_PITCH*drawLine+startY)&0x01FF;
        while (skip-- > 0 && txt)
        {
            s2ch.pgCursorX = 0;
            txt = psp2chCountText(txt, scrW);
        }
        s2ch.pgCursorX = startX;
        pgFillvram(s2ch.resAColor.bg, startX, s2ch.pgCursorY, scrW, LINE_PITCH);
        psp2chPrintText(txt, s2ch.resAColor, scrW+startX);
    }
    else
    {
        skip = drawLine;
        s2ch.pgCursorY = (LINE_PITCH*drawLine+startY)&0x01FF;
        while (skip-- > 0 && txt)
        {
            s2ch.pgCursorX = 0;
            txt = psp2chCountText(txt, scrW);
        }
        line = 0;
        s2ch.pgCursorX = startX;
        pgFillvram(s2ch.resAColor.bg, startX, s2ch.pgCursorY, scrW, LINE_PITCH * lineEnd);
        while (line++ <= lineEnd && txt)
        {
            txt = psp2chPrintText(txt, s2ch.resAColor, scrW+startX);
            s2ch.pgCursorX = startX;
            s2ch.pgCursorY += LINE_PITCH;
        }
    }
}

/*****************************
strを画面幅widthで表示したときの行数を数えるのに使う関数
表示は行わない
*****************************/
char* psp2chCountText(char *str, int width)
{
    unsigned char ch = 0,bef = 0;
    int ret = 0;

    if (str == NULL)
    {
        return NULL;
    }
    while(*str) {
        ch = (unsigned char)*str;
        if (bef!=0) {
            ret = pgCountCharW(bef, ch, width);
            if (ret) {
                return --str;
            }
            bef=0;
        } else {
            if (((ch>=0x80) && (ch<0xa0)) || (ch>=0xe0)) {
                bef = ch;
            } else {
                if (ch == '&') {
                    ret = pgCountSpecialChars((char**)(&str), width);
                }
                else if (ch == '\n') {
                    str++;
                    return str;
                }
                else {
                    ret = pgCountCharA(ch, width);
                }
                if (ret) {
                    return str;
                }
            }
        }
        str++;
    }
    return NULL;
}

/*****************************
strを画面幅widthで表示
*****************************/
char* psp2chPrintText(char *str, S_2CH_RES_COLOR c, int width)
{
    unsigned char ch = 0,bef = 0;
    int ret = 0;
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
            ret = pgPutCharW(bef, ch, tcolor, c.bg, width);
            if (ret)
            {
                return --str;
            }
            bef=0;
        }
        else
        {
            if (((ch>=0x80) && (ch<0xa0)) || (ch>=0xe0))
            {
                bef = ch;
            }
            else
            {
                if (ch == '&')
                {
                    ret = pgSpecialChars((char**)(&str), tcolor, c.bg, width);
                }
                else if (ch == '\n') {
                    str++;
                    return str;
                }
                else
                {
                    ret = pgPutCharA(ch, tcolor, c.bg, width);
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
