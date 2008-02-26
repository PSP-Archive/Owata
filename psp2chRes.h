/*
* $Id$
*/

#ifndef __PSP2CH_RES_H__
#define __PSP2CH_RES_H__

typedef struct {
    int x;
    int y;
    int w;
    int h;
    int total;
    int view;
    int start;
} S_SCROLLBAR;

typedef struct {
    int num;
    char* name;
    char* mail;
    char* date;
    char* id;
    char* text;
    char* title;
    char* be;
    int line;
    int ng;
} S_2CH_RES;

typedef struct {
    int x1;
    int x2;
    int line;
    int res[1001];
    int resCount;
} S_2CH_RES_ANCHOR;

typedef struct {
    int x1;
    int x2;
    int line;
    char host[64];
    char path[256];
} S_2CH_URL_ANCHOR;

typedef struct {
    int x1;
    int x2;
    int line;
    char id[12];
} S_2CH_ID_ANCHOR;

int psp2chFavoriteRes(int ret);
int psp2chThreadRes(int ret);
int psp2chJumpRes(int ret);
int psp2chSearchRes(int ret);
int psp2chRes(char* host, char* dir, char* title, int dat, int ret);
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
