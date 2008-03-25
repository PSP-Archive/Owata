/*
* $Id$
*/

#ifndef __PSP2CH_TINYBROWSER_H__
#define __PSP2CH_TINYBROWSER_H__

int psp2chTinyBrowser(char* path);
int psp2chRenderHtml(char* txt, char* bck, int code);
int psp2chParseTag(unsigned char** src, unsigned char** dst, int code);
void psp2chDrawHtml(char* txt, S_2CH_SCREEN html, int code);
char* psp2chPrintText(char *str, S_2CH_RES_COLOR c, int width, int code, int view);

#endif
