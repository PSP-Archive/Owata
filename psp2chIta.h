/*
* $Id$
*/

#ifndef __PSP2CH_ITA_H__
#define __PSP2CH_ITA_H__

#include "psp2ch.h"

void psp2chItaSetMenuString(void);
int psp2chIta(void);
int psp2chItaList(void);
int psp2chGetMenu(void);
void psp2chDrawCategory(int start, int select, S_2CH_ITA_COLOR color);
void psp2chDrawIta(int start, int select, S_2CH_ITA_COLOR color);
int psp2chItenCheck(char* host, char* ita);

#endif
