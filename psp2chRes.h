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
int psp2chResCursorMove(int* totalLine, int* lineEnd);;
int psp2chResSetLine(S_SCROLLBAR* bar);
void psp2chResPadMove(int* cursorX, int* cursorY, int limitX, int limitY);
int psp2chResList(char* host, char* dir, char* title, int dat);
int psp2chGetDat(char* host, char* dir, char* title, int dat);
void psp2chSaveIdx(char* title, int dat);
void psp2chDrawRes(int line);
void psp2chResAnchor(int anchor);
int psp2chUrlAnchor(int anchor, char* title, int dat, int offset);
void psp2chIdAnchor(int anchor);
int psp2chCountRes(int res, int width);
void psp2chUrlEncode(char* dst, char* src);
int psp2chForm(char* host, char* dir, int dat, char* subject, char* message);

#endif
