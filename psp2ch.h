/*
* $Id$
*/

#ifndef __PSP2CH_H__
#define __PSP2CH_H__

#include <pspnet.h>
#include <pspnet_apctl.h>
#include <pspnet_inet.h>
#include <pspnet_resolver.h>
#include <sys/socket.h>

//#define DEBUG
#define RES_BAR_WIDTH (8)
#define RES_SCR_WIDTH (SCR_WIDTH - RES_BAR_WIDTH)
#define RES_BAR_WIDTH_V (6)
#define RES_SCR_WIDTH_V (SCR_HEIGHT - RES_BAR_WIDTH_V)
#define THREAD_ID 26
#define THREAD_RES 430
#define RES_A_X 45
#define RES_A_Y 30
#define RES_A_WIDTH 390
#define RES_A_HEIGHT 195
#define RES_A_LINE 15
#define RES_A_X_V 30
#define RES_A_Y_V 45
#define RES_A_WIDTH_V 195
#define RES_A_HEIGHT_V 390
#define RES_A_LINE_V 30

#define PAD_CUTOFF 30

typedef struct {
    int Content_Length;
    char Content_Type[32];
    char Last_Modified[32];
    char ETag[32];
} HTTP_HEADERS;

typedef struct {
    int count;
    int start;
    int select;
} S_2CH_SCREEN;

typedef struct {
    int num;
    int name1;
    int name2;
    int mail;
    int date;
    int id1;
    int id2;
    int id3;
} S_2CH_HEADER_COLOR;

typedef struct {
    int text;
    int bg;
    int link;
    int alink;
} S_2CH_RES_COLOR;

typedef struct {
    int slider;
    int bg;
} S_2CH_BAR_COLOR;

typedef struct {
    int text;
    int bg;
    int bat1;
    int bat2;
    int bat3;
} S_2CH_MENU_COLOR;

typedef struct {
    int num;
    int category;
    int text1;
    int text2;
    int bg;
    int count1;
    int count2;
    int s_num;
    int s_category;
    int s_text1;
    int s_text2;
    int s_bg;
    int s_count1;
    int s_count2;
} S_2CH_THREAD_COLOR;

struct sList {
    int text;
    int bg;
    int s_text;
    int s_bg;
};

typedef struct {
    struct sList cate;
    struct sList ita;
    int base;
} S_2CH_ITA_COLOR;

typedef struct {
    int title;
    int title_bg;
} S_2CH_FORM_COLOR;

int psp2ch(void);
void psp2chStart(void);
int psp2chOwata(void);
int psp2chCursorSet(S_2CH_SCREEN* line,  int lineEnd);
int psp2chPadSet(int scrollX);
int psp2chInit(void);
int psp2chTerm(void);
int psp2chCloseSocket(int mySocket);
int psp2chRequest(const char* host, const char* path, const char* header);
int psp2chPost(char* host, char* dir, int dat, char* cook, char* body);
int psp2chGetStatusLine(int mySocket);
int psp2chGetHttpHeaders(int mySocket, HTTP_HEADERS* header);
int psp2chApConnect(void);
void psp2chGets(char* title, char* text, int num, int lines);

#endif
