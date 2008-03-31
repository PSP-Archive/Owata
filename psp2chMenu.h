/*
* $Id$
*/

#ifndef __PSP2CH_MENU_H__
#define __PSP2CH_MENU_H__

void psp2chMenuSetMenuString(void);
int psp2chMenu(void);
void psp2chMenuNG(void);
char* psp2chGetNGBuf(const char* file, char* buf);
int psp2chNGDel(const char* file);
int psp2chNGAdd(const char* file, char* val);
void psp2chMenuFontSet(int select);
void psp2chMenuFont(void);
int psp2chMenuColor(void);
void psp2chDrawMenu(char** menuList, S_2CH_SCREEN menu, int x, int y, int width, int height);

#endif
