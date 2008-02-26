/*
* $Id$
*/

#ifndef __PSP2CH_MENU_H__
#define __PSP2CH_MENU_H__

int psp2chMenu(int pixelsX, int pixelsY);
void psp2chMenuNG(int pixelsX, int pixelsY);
char* psp2chGetNGBuf(const char* file, char* buf);
int psp2chNGDel(const char* file, int pixelsX, int pixelsY);
int psp2chNGAdd(const char* file, char* val);
void psp2chDrawMenu(char** menuList, S_2CH_SCREEN menu, int x, int y, int width, int height);

#endif
