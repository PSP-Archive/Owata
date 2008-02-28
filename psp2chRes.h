/*
* $Id$
*/

#ifndef __PSP2CH_RES_H__
#define __PSP2CH_RES_H__

void psp2chResSetMenuString(void);
int psp2chFavoriteRes(int ret);
int psp2chThreadRes(int ret);
int psp2chJumpRes(int ret);
int psp2chSearchRes(int ret);
int psp2chRes(char* host, char* dir, char* title, int dat, int ret);
void psp2chResResetAnchors(void);
int psp2chResCursorMove(int* totalLine, int* lineEnd, int* cursorY, int limitY);
int psp2chResSetLine(S_SCROLLBAR* bar);
void psp2chResPadMove(int* cursorX, int* cursorY, int limitX, int limitY);
void psp2chResCheckNG(void);
int psp2chResList(char* host, char* dir, char* title, int dat);
int psp2chGetDat(char* host, char* dir, char* title, int dat);
void psp2chSaveIdx(char* title, int dat);
int psp2chDrawResHeader(int re, int* skip, int line, int lineEnd, int startX, int endX,S_2CH_RES_COLOR c,S_2CH_HEADER_COLOR hc, int* drawLine);
int psp2chDrawResText(int res, int* skip, int line, int lineEnd, int startX, int endX, S_2CH_RES_COLOR c, int* drawLine);
void psp2chDrawRes(int line);
int psp2chCountRes(int res, int width);

#endif
