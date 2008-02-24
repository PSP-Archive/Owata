/*
* $Id$
*/

#ifndef __PSP2CH_ITA_H__
#define __PSP2CH_ITA_H__

#include "psp2ch.h"

typedef struct {
    char name[32];
    int itaId;
} S_2CH_CATEGORY;

typedef struct {
    char host[32];
    char dir[32];
    char title[32];
} S_2CH_ITA;

int psp2chIta(void);
int psp2chItaList(void);
int psp2chGetMenu(void);
void psp2chDrawCategory(int start, int select, S_2CH_ITA_COLOR color);
void psp2chDrawIta(int start, int select, S_2CH_ITA_COLOR color);

#endif
