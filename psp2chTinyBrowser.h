/*
* $Id$
*/

#ifndef __PSP2CH_TINYBROWSER_H__
#define __PSP2CH_TINYBROWSER_H__

int psp2chTinyBrowser(char* path);
int psp2chRenderHtml(char* txt);
int psp2chParseTag(unsigned char** src, unsigned char** dst);
void psp2chViewHtml(char* txt, char* path);
void psp2chDrawHtml(char* txt, S_2CH_SCREEN html, int lineFlag);
char* psp2chCountText(char *str, int width);
char* psp2chPrintText(char *str, S_2CH_RES_COLOR c, int width);

#endif
