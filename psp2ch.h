/*
* $Id$
*/

#ifndef __PSP2CH_H__
#define __PSP2CH_H__

#include "pspdialogs.h"
#include <pspnet.h>
#include <pspnet_apctl.h>
#include <pspnet_inet.h>
#include <pspnet_resolver.h>
#include <sys/socket.h>
#include <pspctrl.h>

//#define DEBUG
#define RES_BAR_WIDTH (8)
#define RES_SCR_WIDTH (SCR_WIDTH - RES_BAR_WIDTH)
#define RES_BAR_WIDTH_V (6)
#define RES_SCR_WIDTH_V (SCR_HEIGHT - RES_BAR_WIDTH_V)
#define RES_A_X 45
#define RES_A_Y 30
#define RES_A_WIDTH 390
#define RES_A_HEIGHT 195
#define RES_A_LINE (RES_A_HEIGHT / LINE_PITCH)
#define RES_A_X_V 30
#define RES_A_Y_V 45
#define RES_A_WIDTH_V 195
#define RES_A_HEIGHT_V 390
#define RES_A_LINE_V (RES_A_HEIGHT_V / LINE_PITCH)

typedef struct {
    char name[32];
    int itaId;
} S_2CH_CATEGORY;

typedef struct {
    char host[32];
    char dir[32];
    char title[32];
} S_2CH_ITA;

typedef struct {
    char host[32];
    char dir[32];
    char title[32];
    int dat;
    char subject[128];
    int res;
    int update;
} S_2CH_FAVORITE;

typedef struct {
    char cate[32];
    char title[32];
} S_2CH_FAV_ITA;

typedef struct {
    int id;
    int dat;
    char title[128];
    int res;
    int old;
    int ikioi;
} S_2CH_THREAD;

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

typedef struct {
    int text;
    int bg;
    int s_text;
    int s_bg;
} S_2CH_TXT_COLOR;

typedef struct {
    S_2CH_TXT_COLOR cate;
    S_2CH_TXT_COLOR ita;
    int base;
} S_2CH_ITA_COLOR;

typedef struct {
    int ita;
    int title;
    int title_bg;
} S_2CH_FORM_COLOR;

typedef struct {
    int arrow1;
    int arrow2;
} S_2CH_CURSOR_COLOR;

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

typedef struct {
    int x1;
    int x2;
    int line;
    int num;
} S_2CH_NUM_ANCHOR;

typedef struct {
    int x1;
    int x2;
    int line;
    int type;
    int id;
} S_2CH_ANCHOR_LIST;

typedef struct {
    int up, pUp, down, pDown, top, end;
} S_2CH_SCROLL_BUTTONS;

typedef struct {
    int ok, esc, move, reload, shift;
    int addFav, search2ch;
} S_2CH_ITA_BUTTONS;

typedef struct {
    int ok, move, change, del, shift;
    int sort, search2ch, update;
} S_2CH_FAV_BUTTONS;

typedef struct {
    int ok, esc, move, reload, shift;
    int sort, search, search2ch;
} S_2CH_THREAD_BUTTONS;

typedef struct {
    int ok, esc, ita, fav, shift;
    int search2ch;
} S_2CH_SEARCH_BUTTONS;

typedef struct {
    int ok, esc;
    char main[112];
} S_2CH_MENU_WIN;

typedef struct {
    int del, esc;
    char main[112];
} S_2CH_MENU_NG;

typedef struct {
    int osk, esc, ok;
    char main[112];
} S_2CH_INPUT_DIALOG;

typedef struct {
    int form, back, reload, datDel, change, addFav, delFav, cursor, wide;
    int resForm, resFBack, idView, idNG, idBack, resView, resMove, resBack, url, urlBack;
    S_2CH_SCROLL_BUTTONS s;
} S_2CH_RES_BUTTONS;

typedef struct {
    char main[112];
    char sub[112];
} S_2CH_MENU_STR;

typedef struct {
    char main[112];
    char sub1[112];
    char sub2[112];
    char aNum[112];
    char aRes[112];
    char aId[112];
    char aUrl[112];
} S_2CH_RES_MENU_STR;

typedef struct {
    int padReverse;
    int padAccel;
    int padCutoff;
    int favSelect;
    char imageDir[32];
} S_2CH_CONFIG;

typedef struct {
    int height;
    int pitch;
    int lineH;
    int lineV;
    char fileA[32];
    char fileJ[32];
    char name[32];
    char** set;
    int count;
    int select;
} S_2CH_FONT;

typedef struct {
    S_2CH_CATEGORY* categoryList;
    S_2CH_ITA* itaList;
    S_2CH_FAVORITE* favList;
    S_2CH_FAVORITE* findList;
    S_2CH_FAV_ITA* favItaList;
    S_2CH_THREAD* threadList;
    S_2CH_RES* resList;
    S_2CH_SCREEN category;
    S_2CH_SCREEN ita;
    S_2CH_SCREEN fav;
    S_2CH_SCREEN find;
    S_2CH_SCREEN favIta;
    S_2CH_SCREEN thread;
    S_2CH_SCREEN res;
    S_2CH_URL_ANCHOR urlAnchor[50];
    S_2CH_RES_ANCHOR resAnchor[50];
    S_2CH_ID_ANCHOR idAnchor[40];
    S_2CH_NUM_ANCHOR numAnchor[40];
    S_2CH_ANCHOR_LIST anchorList[100];
    int urlAnchorCount;
    int resAnchorCount;
    int idAnchorCount;
    int numAnchorCount;
    S_2CH_HEADER_COLOR resHeaderColor;
    S_2CH_RES_COLOR resColor;
    S_2CH_BAR_COLOR resBarColor;
    S_2CH_HEADER_COLOR resAHeaderColor;
    S_2CH_RES_COLOR resAColor;
    S_2CH_BAR_COLOR resABarColor;
    S_2CH_MENU_COLOR menuColor;
    S_2CH_THREAD_COLOR threadColor;
    S_2CH_ITA_COLOR cateOnColor;
    S_2CH_ITA_COLOR cateOffColor;
    S_2CH_FORM_COLOR formColor;
    S_2CH_TXT_COLOR menuWinColor;
    S_2CH_CURSOR_COLOR cursorColor;
    S_2CH_RES_BUTTONS btnResH;
    S_2CH_RES_BUTTONS btnResV;
    S_2CH_RES_MENU_STR menuResH;
    S_2CH_RES_MENU_STR menuResV;
    S_2CH_SCROLL_BUTTONS listH;
    S_2CH_SCROLL_BUTTONS listV;
    S_2CH_ITA_BUTTONS itaH;
    S_2CH_ITA_BUTTONS itaV;
    S_2CH_THREAD_BUTTONS thH;
    S_2CH_THREAD_BUTTONS thV;
    S_2CH_FAV_BUTTONS favH;
    S_2CH_FAV_BUTTONS favV;
    S_2CH_SEARCH_BUTTONS findH;
    S_2CH_SEARCH_BUTTONS findV;
    S_2CH_MENU_STR menuCateH;
    S_2CH_MENU_STR menuCateV;
    S_2CH_MENU_STR menuItaH;
    S_2CH_MENU_STR menuItaV;
    S_2CH_MENU_STR menuFavH;
    S_2CH_MENU_STR menuFavV;
    S_2CH_MENU_STR menuFavItaH;
    S_2CH_MENU_STR menuFavItaV;
    S_2CH_MENU_STR menuThreadH;
    S_2CH_MENU_STR menuThreadV;
    S_2CH_MENU_STR menuSearchH;
    S_2CH_MENU_STR menuSearchV;
    S_2CH_MENU_WIN menuWinH;
    S_2CH_MENU_WIN menuWinV;
    S_2CH_MENU_NG menuNGH;
    S_2CH_MENU_NG menuNGV;
    S_2CH_CONFIG cfg;
    S_2CH_FONT font;
    int running;
    int sel;
    int tateFlag;
    char cwDir[256];
    char* logDir;
    char* fontDir;
    char* colorDir;
    int pgCursorX, pgCursorY;
    int viewX, viewY;
    SceCtrlData pad;
    SceCtrlData oldPad;
    MESSAGE_HELPER mh;
} S_2CH;

int psp2ch(void);
void psp2chStart(void);
int psp2chOwata(void);
int psp2chCursorSet(S_2CH_SCREEN* line,  int lineEnd, int shift, int* change);
int psp2chPadSet(int scrollX);
int psp2chInit(void);
int psp2chTerm(void);
void psp2chGets(char* title, char* text, int num, int lines);
int psp2chInputDialog(const unsigned short* text1, char* text2);
void psp2chErrorDialog(const char* fmt, ...);

#endif
