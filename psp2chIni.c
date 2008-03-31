/*
* $Id$
*/

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "psp2ch.h"
#include "pg.h"

extern S_2CH s2ch; // psp2ch.c
extern const char* colorFile; // psp2chMenu.c

#define setHex(A, B, C) \
    p = strstr(buf, (A));\
    if (p) {\
        p = strstr(p, "0x");\
        if(p){\
            p+=2;\
            sscanf(p, "%X", &(B));\
        }\
        else{(B) = (C);}\
    }\
    else{(B) = (C);}

#define setInt(A, B, C) \
    p = strstr(buf, (A));\
    if (p) {\
        p = strchr(p, '=');\
        if(p){\
            p++;\
            sscanf(p, "%d", &(B));\
        }\
        else{(B) = (C);}\
    }\
    else{(B) = (C);}

#define setString(A, B, C, D) \
    p = strstr(buf2, (A));\
    if (p) {\
        p = strchr(p, '"');\
        if(p){\
            p++;\
            for (i = 0; *p != '"'; i++) {\
                if (i >= (D)) break;\
                (B)[i] = *p++;\
            }\
            (B)[i] = '\0';\
        }\
        else{strcpy((B), (C));}\
    }\
    else{strcpy((B), (C));}\

/***********************************
config.iniを読み込む
************************************/
void psp2chIniLoadConfig(void)
{
    SceUID fd;
    SceIoStat st;
    char file[256];
    char *buf, *buf2;
    char *p;
    int i, j;

    sprintf(file, "%s/config.ini", s2ch.cwDir);
    i = sceIoGetstat(file, &st);
    if (i >= 0)
    {
        buf = (char*)malloc(st.st_size + 1);
        if (buf)
        {
            fd = sceIoOpen(file, PSP_O_RDONLY, 0777);
            if (fd >= 0)
            {
                sceIoRead(fd, buf, st.st_size);
                sceIoClose(fd);
                buf[st.st_size] = '\0';
                buf2 = buf;
                setInt("CFG_PAD_REVERSE", s2ch.cfg.padReverse, 1);
                setInt("CFG_PAD_ACCEL", s2ch.cfg.padAccel, 1);
                setInt("CFG_PAD_CUTOFF", s2ch.cfg.padCutoff, 35);
                setInt("CFG_FAV_SELECT", s2ch.cfg.favSelect, 0);
                setString("CFG_IMAGE_DIR", s2ch.cfg.imageDir, "", 32);
                s2ch.font.count = 0;
                while ((p = strstr(buf2, "CFG_FONT_SET")))
                {
                    s2ch.font.count++;
                    buf2 = p + 12;
                }
                if (s2ch.font.count == 0)
                {
                    s2ch.font.count = 1;
                }
                s2ch.font.set = (char**)malloc(sizeof(char*) * s2ch.font.count);
                buf2 = buf;
                if (s2ch.font.set == NULL)
                {
                    strcpy(s2ch.font.fileA, "monafontA.bin");
                    strcpy(s2ch.font.fileJ, "monafontJ.bin");
                    s2ch.font.height = 12;
                    s2ch.font.pitch = 13;
                    s2ch.font.count = 1;
                }
                else
                {
                    for (j = 0; j < s2ch.font.count; j++)
                    {
                        setString("CFG_FONT_SET", file, "モナー	monafontA.bin	monafontJ.bin	12	13", 256);
                        s2ch.font.set[j] = (char*)malloc(sizeof(char) * strlen(file));
                        strcpy(s2ch.font.set[j], file);
                        buf2 = p;
                    }
                }

                free(buf);
                return;
            }
            else
            {
                free(buf);
            }
        }
    }
    s2ch.cfg.padReverse = 1; // パッド横軸の移動方向(1:対象が移動,-1:視点が移動)
    s2ch.cfg.padAccel = 1; // 0:2段階速, 1:8段階速
    s2ch.cfg.padCutoff = 35; // アナログパッドのニュートラルあそび値
    s2ch.cfg.favSelect = 0; // お気に入りのデフォルト表示 0:スレ, 1:板
    strcpy(s2ch.cfg.imageDir, ""); // PICTUREフォルダに作成するフォルダ名
    strcpy(s2ch.font.fileA, "monafontA.bin"); // シングルバイト(ASCII, 半角カナ)フォント
    strcpy(s2ch.font.fileJ, "monafontJ.bin"); // マルチバイトフォント
    s2ch.font.height = 12;
    s2ch.font.pitch = 13;
    s2ch.font.count = 1;
    s2ch.font.set = NULL;
}

/***********************************
外部カラーセット
設定ファイルを読み込んで各カラーをセット
************************************/
void psp2chIniSetColor(const char* file)
{
    SceUID fd;
    SceIoStat st;
    char path[256];
    char *buf;
    char *p;
    int ret;

    if (file == NULL)
    {
        file = colorFile;
    }
    sprintf(path, "%s/%s/%s", s2ch.cwDir, s2ch.colorDir, file);
    ret = sceIoGetstat(path, &st);
    if (ret >= 0)
    {
        buf = (char*)malloc(st.st_size + 1);
        if (buf)
        {
            fd = sceIoOpen(path, PSP_O_RDONLY, 0777);
            if (fd >= 0)
            {
                sceIoRead(fd, buf, st.st_size);
                sceIoClose(fd);
                buf[st.st_size] = '\0';
                setHex("RES_NUMBER", s2ch.resHeaderColor.num, RED);
                setHex("RES_NAME_HEAD", s2ch.resHeaderColor.name1, BLACK);
                setHex("RES_NAME_BODY", s2ch.resHeaderColor.name2, RGB(0x00, 0xCC, 0x00));
                setHex("RES_MAIL", s2ch.resHeaderColor.mail, RGB(0x99, 0x99, 0x99));
                setHex("RES_DATE", s2ch.resHeaderColor.date, BLACK);
                setHex("RES_ID_HEAD_1", s2ch.resHeaderColor.id1, BLUE);
                setHex("RES_ID_HEAD_2", s2ch.resHeaderColor.id2, RED);
                setHex("RES_ID_BODY", s2ch.resHeaderColor.id3, BLUE);
                setHex("RES_TEXT", s2ch.resColor.text, BLACK);
                setHex("RES_BG", s2ch.resColor.bg, RGB(0xE0, 0xE0, 0xE0));
                setHex("RES_LINK", s2ch.resColor.link, BLUE);
                setHex("RES_BAR_SLIDER", s2ch.resBarColor.slider, YELLOW);
                setHex("RES_BAR_BG", s2ch.resBarColor.bg, RGB(0x66, 0x66, 0xFF));

                setHex("RES_A_NUMBER", s2ch.resAHeaderColor.num, RED);
                setHex("RES_A_NAME_HEAD", s2ch.resAHeaderColor.name1, BLACK);
                setHex("RES_A_NAME_BODY", s2ch.resAHeaderColor.name2, RGB(0x00, 0xCC, 0x00));
                setHex("RES_A_MAIL", s2ch.resAHeaderColor.mail, RGB(0x99, 0x99, 0x99));
                setHex("RES_A_DATE", s2ch.resAHeaderColor.date, BLACK);
                setHex("RES_A_ID_HEAD_1", s2ch.resAHeaderColor.id1, BLUE);
                setHex("RES_A_ID_HEAD_2", s2ch.resAHeaderColor.id2, RED);
                setHex("RES_A_ID_BODY", s2ch.resAHeaderColor.id3, BLUE);
                setHex("RES_A_TEXT", s2ch.resAColor.text, BLACK);
                setHex("RES_A_BG", s2ch.resAColor.bg, RGB(0xFF, 0xFF, 0xCC));
                setHex("RES_A_LINK", s2ch.resAColor.link, BLUE);
                setHex("RES_A_BAR_SLIDER", s2ch.resABarColor.slider, RGB(0x00, 0xFF, 0xCC));
                setHex("RES_A_BAR_BG", s2ch.resABarColor.bg, RGB(0xCC, 0xFF, 0xFF));

                setHex("MENU_TEXT", s2ch.menuColor.text, WHITE);
                setHex("MENU_BG", s2ch.menuColor.bg, BLACK);
                setHex("MENU_BATTERY_1", s2ch.menuColor.bat1, GREEN);
                setHex("MENU_BATTERY_2", s2ch.menuColor.bat2, YELLOW);
                setHex("MENU_BATTERY_3", s2ch.menuColor.bat3, RED);

                setHex("THREAD_NUMBER", s2ch.threadColor.num, RED);
                setHex("THREAD_CATEGORY", s2ch.threadColor.category, RED);
                setHex("THREAD_TEXT_1", s2ch.threadColor.text1, BLUE);
                setHex("THREAD_TEXT_2", s2ch.threadColor.text2, RED);
                setHex("THREAD_BG", s2ch.threadColor.bg, RGB(0xCC, 0xFF, 0xCC));
                setHex("THREAD_COUNT_1", s2ch.threadColor.count1, BLACK);
                setHex("THREAD_COUNT_2", s2ch.threadColor.count2, RED);
                setHex("THREAD_SELECT_NUMBER", s2ch.threadColor.s_num, RED);
                setHex("THREAD_SELECT_CATEGORY", s2ch.threadColor.s_category, RGB(0x99, 0x00, 0x00));
                setHex("THREAD_SELECT_TEXT_1", s2ch.threadColor.s_text1, RGB(0x00, 0x00, 0x99));
                setHex("THREAD_SELECT_TEXT_2", s2ch.threadColor.s_text2, RGB(0x99, 0x00, 0x00));
                setHex("THREAD_SELECT_BG", s2ch.threadColor.s_bg, GRAY);
                setHex("THREAD_SELECT_COUNT_1", s2ch.threadColor.s_count1, BLACK);
                setHex("THREAD_SELECT_COUNT_2", s2ch.threadColor.s_count2, RED);

                setHex("CATE_ON_TEXT", s2ch.cateOnColor.cate.text, RGB(0xCC, 0x33, 0x00));
                setHex("CATE_ON_BG", s2ch.cateOnColor.cate.bg, WHITE);
                setHex("CATE_ON_S_TEXT", s2ch.cateOnColor.cate.s_text, WHITE);
                setHex("CATE_ON_S_BG", s2ch.cateOnColor.cate.s_bg, RGB(0xCC, 0x33, 0x00));
                setHex("ITA_OFF_TEXT", s2ch.cateOnColor.ita.text, RGB(0x66, 0x66, 0xFF));
                setHex("ITA_OFF_BG", s2ch.cateOnColor.ita.bg, GRAY);
                setHex("ITA_OFF_S_TEXT", s2ch.cateOnColor.ita.s_text, GRAY);
                setHex("ITA_OFF_S_BG", s2ch.cateOnColor.ita.s_bg, RGB(0x66, 0x66, 0xFF));
                setHex("CATE_ON_BASE", s2ch.cateOnColor.base, WHITE);

                setHex("CATE_OFF_TEXT", s2ch.cateOffColor.cate.text, RGB(0x88, 0x99, 0x66));
                setHex("CATE_OFF_BG", s2ch.cateOffColor.cate.bg, GRAY);
                setHex("CATE_OFF_S_TEXT", s2ch.cateOffColor.cate.s_text, GRAY);
                setHex("CATE_OFF_S_BG", s2ch.cateOffColor.cate.s_bg, RGB(0x88, 0x99, 0x66));
                setHex("ITA_ON_TEXT", s2ch.cateOffColor.ita.text, BLUE);
                setHex("ITA_ON_BG", s2ch.cateOffColor.ita.bg, WHITE);
                setHex("ITA_ON_S_TEXT", s2ch.cateOffColor.ita.s_text, WHITE);
                setHex("ITA_ON_S_BG", s2ch.cateOffColor.ita.s_bg, BLUE);
                setHex("CATE_OFF_BASE", s2ch.cateOffColor.base, WHITE);

                setHex("FORM_ITA_TEXT", s2ch.formColor.ita, GRAY);
                setHex("FORM_TITLE_TEXT", s2ch.formColor.title, WHITE);
                setHex("FORM_TITLE_BG", s2ch.formColor.title_bg, RED);

                setHex("MENU_WIN_TEXT", s2ch.menuWinColor.text, GRAY);
                setHex("MENU_WIN_BG", s2ch.menuWinColor.bg, BLACK);
                setHex("MENU_WIN_S_TEXT", s2ch.menuWinColor.s_text, WHITE);
                setHex("MENU_WIN_S_BG", s2ch.menuWinColor.s_bg, BLUE);
                free(buf);
                return;
            }
            else
            {
                free(buf);
            }
        }
    }
    // iniファイルがない時のデフォルト
    // レス本文
    s2ch.resHeaderColor.num = RED;
    s2ch.resHeaderColor.name1 = BLACK;
    s2ch.resHeaderColor.name2 = RGB(0x00, 0xCC, 0x00);
    s2ch.resHeaderColor.mail = RGB(0x99, 0x99, 0x99);
    s2ch.resHeaderColor.date = BLACK;
    s2ch.resHeaderColor.id1 = BLUE;
    s2ch.resHeaderColor.id2 = RED;
    s2ch.resHeaderColor.id3 = BLUE;
    s2ch.resColor.text = BLACK;
    s2ch.resColor.bg = RGB(0xE0, 0xE0, 0xE0);
    s2ch.resColor.link = BLUE;
    s2ch.resBarColor.slider = YELLOW;
    s2ch.resBarColor.bg = RGB(0x66, 0x66, 0xFF);
    // レスアンカー　
    s2ch.resAHeaderColor.num = RED;
    s2ch.resAHeaderColor.name1 = BLACK;
    s2ch.resAHeaderColor.name2 = RGB(0x00, 0xCC, 0x00);
    s2ch.resAHeaderColor.mail = RGB(0x99, 0x99, 0x99);
    s2ch.resAHeaderColor.date = BLACK;
    s2ch.resAHeaderColor.id1 = BLUE;
    s2ch.resAHeaderColor.id2 = RED;
    s2ch.resAHeaderColor.id3 = BLUE;
    s2ch.resAColor.text = BLACK;
    s2ch.resAColor.bg = RGB(0xFF, 0xFF, 0xCC);
    s2ch.resAColor.link = BLUE;
    s2ch.resABarColor.slider = RGB(0x00, 0xFF, 0xCC);
    s2ch.resABarColor.bg = RGB(0xCC, 0xFF, 0xFF);
    // メニューバー　
    s2ch.menuColor.text = WHITE;
    s2ch.menuColor.bg = BLACK;
    s2ch.menuColor.bat1 = GREEN;
    s2ch.menuColor.bat2 = YELLOW;
    s2ch.menuColor.bat3 = RED;
    // スレッド一覧
    s2ch.threadColor.num = RED;
    s2ch.threadColor.category = RED;
    s2ch.threadColor.text1 = BLUE;
    s2ch.threadColor.text2 = RED;
    s2ch.threadColor.bg = RGB(0xCC, 0xFF,0xCC);
    s2ch.threadColor.count1 = BLACK;
    s2ch.threadColor.count2 = RED;
    s2ch.threadColor.s_num = RED;
    s2ch.threadColor.s_category = RGB(0x99, 0x00, 0x00);
    s2ch.threadColor.s_text1 = RGB(0x00, 0x00, 0x99);
    s2ch.threadColor.s_text2 = RGB(0x99, 0x00, 0x00);
    s2ch.threadColor.s_bg = GRAY;
    s2ch.threadColor.s_count1 = BLACK;
    s2ch.threadColor.s_count2 = RED;
    // カテゴリー・板一覧
    s2ch.cateOnColor.cate.text = RGB(0xCC, 0x33, 0x00);
    s2ch.cateOnColor.cate.bg = WHITE;
    s2ch.cateOnColor.cate.s_text = WHITE;
    s2ch.cateOnColor.cate.s_bg = RGB(0xCC, 0x33, 0x00);
    s2ch.cateOnColor.ita.text = RGB(0x66, 0x66, 0xFF);
    s2ch.cateOnColor.ita.bg = GRAY;
    s2ch.cateOnColor.ita.s_text = GRAY;
    s2ch.cateOnColor.ita.s_bg = RGB(0x66, 0x66, 0xFF);
    s2ch.cateOnColor.base = WHITE;
    s2ch.cateOffColor.cate.text = RGB(0x88, 0x99, 0x66);
    s2ch.cateOffColor.cate.bg = GRAY;
    s2ch.cateOffColor.cate.s_text = GRAY;
    s2ch.cateOffColor.cate.s_bg = RGB(0x88, 0x99, 0x66);
    s2ch.cateOffColor.ita.text = BLUE;
    s2ch.cateOffColor.ita.bg = WHITE;
    s2ch.cateOffColor.ita.s_text = WHITE;
    s2ch.cateOffColor.ita.s_bg = BLUE;
    s2ch.cateOffColor.base = WHITE;
    // 送信フォーム
    s2ch.formColor.ita = GRAY;
    s2ch.formColor.title = WHITE;
    s2ch.formColor.title_bg = RED;
    // メニューウィンドウ
    s2ch.menuWinColor.text = GRAY;
    s2ch.menuWinColor.bg = BLACK;
    s2ch.menuWinColor.s_text = WHITE;
    s2ch.menuWinColor.s_bg = BLUE;
}

/********************
ボタンの割り当て
button.iniを読み込んでボタンの設定
********************/
void psp2chIniSetButtons(void)
{
    SceUID fd;
    SceIoStat st;
    char file[256];
    char *buf;
    char *p;
    int ret;

    sprintf(file, "%s/button.ini", s2ch.cwDir);
    ret = sceIoGetstat(file, &st);
    if (ret >= 0)
    {
        buf = (char*)malloc(st.st_size + 1);
        if (buf)
        {
            fd = sceIoOpen(file, PSP_O_RDONLY, 0777);
            if (fd >= 0)
            {
                sceIoRead(fd, buf, st.st_size);
                sceIoClose(fd);
                buf[st.st_size] = '\0';
                setInt("RES_FORM_H", s2ch.btnResH.form, 8192);
                setInt("RES_BACK_H", s2ch.btnResH.back, 16384);
                setInt("RES_RELOAD_H", s2ch.btnResH.reload, 4096);
                setInt("RES_DATDEL_H", s2ch.btnResH.datDel, 32768);
                setInt("RES_SHIFT_H", s2ch.btnResH.change, 512);
                setInt("RES_FORM_V", s2ch.btnResV.form, 8192);
                setInt("RES_BACK_V", s2ch.btnResV.back, 16384);
                setInt("RES_RELOAD_V", s2ch.btnResV.reload, 4096);
                setInt("RES_DATDEL_V", s2ch.btnResV.datDel, 32768);
                setInt("RES_SHIFT_V", s2ch.btnResV.change, 512);

                setInt("RES_TOP_H", s2ch.btnResH.s.top, 16);
                setInt("RES_END_H", s2ch.btnResH.s.end, 64);
                setInt("RES_ADDFAV_H", s2ch.btnResH.addFav, 8192);
                setInt("RES_DELFAV_H", s2ch.btnResH.delFav, 32768);
                setInt("RES_TOP_V", s2ch.btnResV.s.top, 32);
                setInt("RES_END_V", s2ch.btnResV.s.end, 128);
                setInt("RES_ADDFAV_V", s2ch.btnResV.addFav, 8192);
                setInt("RES_DELFAV_V", s2ch.btnResV.delFav, 32768);

                setInt("RES_NUMRES_H", s2ch.btnResH.resForm, 8192);
                setInt("RES_NUMBACK_H", s2ch.btnResH.resFBack, 16384);
                setInt("RES_NUMRES_V", s2ch.btnResV.resForm, 8192);
                setInt("RES_NUMBACK_V", s2ch.btnResV.resFBack, 16384);

                setInt("RES_IDVIEW_H", s2ch.btnResH.idView, 8192);
                setInt("RES_IDNG_H", s2ch.btnResH.idNG, 32768);
                setInt("RES_IDBACK_H", s2ch.btnResH.idBack, 16384);
                setInt("RES_IDVIEW_V", s2ch.btnResV.idView, 8192);
                setInt("RES_IDNG_V", s2ch.btnResV.idNG, 32768);
                setInt("RES_IDBACK_V", s2ch.btnResV.idBack, 16384);

                setInt("RES_RESVIEW_H", s2ch.btnResH.resView, 8192);
                setInt("RES_RESMOVE_H", s2ch.btnResH.resMove, 4096);
                setInt("RES_RESBACK_H", s2ch.btnResH.resBack, 16384);
                setInt("RES_RESVIEW_V", s2ch.btnResV.resView, 8192);
                setInt("RES_RESMOVE_V", s2ch.btnResV.resMove, 4096);
                setInt("RES_RESBACK_V", s2ch.btnResV.resBack, 16384);

                setInt("RES_URL_H", s2ch.btnResH.url, 8192);
                setInt("RES_URLBACK_H", s2ch.btnResH.urlBack, 16384);
                setInt("RES_URL_V", s2ch.btnResV.url, 8192);
                setInt("RES_URLBACK_V", s2ch.btnResV.urlBack, 16384);

                setInt("RES_UP_H", s2ch.btnResH.s.up, 16);
                setInt("RES_PAGEUP_H", s2ch.btnResH.s.pUp, 128);
                setInt("RES_DOWN_H", s2ch.btnResH.s.down, 64);
                setInt("RES_PAGEDOWN_H", s2ch.btnResH.s.pDown, 32);
                setInt("RES_UP_V", s2ch.btnResV.s.up, 32);
                setInt("RES_PAGEUP_V", s2ch.btnResV.s.pUp, 16);
                setInt("RES_DOWN_V", s2ch.btnResV.s.down, 128);
                setInt("RES_PAGEDOWN_V", s2ch.btnResV.s.pDown, 64);

                setInt("SCROLL_UP_H", s2ch.listH.up, 16);
                setInt("PAGE_UP_H", s2ch.listH.pUp, 128);
                setInt("SCROLL_DOWN_H", s2ch.listH.down, 64);
                setInt("PAGE_DOWN_H", s2ch.listH.pDown, 32);
                setInt("LIST_TOP_H", s2ch.listH.top, 16);
                setInt("LIST_END_H", s2ch.listH.end, 64);
                setInt("SCROLL_UP_V", s2ch.listV.up, 32);
                setInt("PAGE_UP_V", s2ch.listV.pUp, 16);
                setInt("SCROLL_DOWN_V", s2ch.listV.down, 128);
                setInt("PAGE_DOWN_V", s2ch.listV.pDown, 64);
                setInt("LIST_TOP_V", s2ch.listV.top, 32);
                setInt("LIST_END_V", s2ch.listV.end, 128);

                setInt("ITA_OK_H", s2ch.itaH.ok, 8192);
                setInt("ITA_ESC_H", s2ch.itaH.esc, 16384);
                setInt("ITA_MOVEFAV_H", s2ch.itaH.move, 32768);
                setInt("ITA_RELOAD_H", s2ch.itaH.reload, 4096);
                setInt("ITA_SHIFT_H", s2ch.itaH.shift, 512);
                setInt("ITA_OK_V", s2ch.itaV.ok, 256);
                setInt("ITA_ESC_V", s2ch.itaV.esc, 16384);
                setInt("ITA_MOVEFAV_V", s2ch.itaV.move, 32768);
                setInt("ITA_RELOAD_V", s2ch.itaV.reload, 4096);
                setInt("ITA_SHIFT_V", s2ch.itaV.shift, 512);

                setInt("ITA_ADDFAV_H", s2ch.itaH.addFav, 4096);
                setInt("ITA_2CHSEARCH_H", s2ch.itaH.search2ch, 32768);
                setInt("ITA_ADDFAV_V", s2ch.itaV.addFav, 4096);
                setInt("ITA_2CHSEARCH_V", s2ch.itaV.search2ch, 32768);

                setInt("THREAD_OK_H", s2ch.thH.ok, 8192);
                setInt("THREAD_ESC_H", s2ch.thH.esc, 16384);
                setInt("THREAD_MOVEFAV_H", s2ch.thH.move, 32768);
                setInt("THREAD_RELOAD_H", s2ch.thH.reload, 4096);
                setInt("THREAD_SHIFT_H", s2ch.thH.shift, 512);
                setInt("THREAD_OK_V", s2ch.thV.ok, 256);
                setInt("THREAD_ESC_V", s2ch.thV.esc, 16384);
                setInt("THREAD_MOVEFAV_V", s2ch.thV.move, 32768);
                setInt("THREAD_RELOAD_V", s2ch.thV.reload, 4096);
                setInt("THREAD_SHIFT_V", s2ch.thV.shift, 512);

                setInt("THREAD_SORT_H", s2ch.thH.sort, 8192);
                setInt("THREAD_SEARCH_H", s2ch.thH.search, 4096);
                setInt("THREAD_2CHSEARCH_H", s2ch.thH.search2ch, 32768);
                setInt("THREAD_SORT_V", s2ch.thV.sort, 8192);
                setInt("THREAD_SEARCH_V", s2ch.thV.search, 4096);
                setInt("THREAD_2CHSEARCH_V", s2ch.thV.search2ch, 32768);

                setInt("FAV_OK_H", s2ch.favH.ok, 8192);
                setInt("FAV_MOVEITS_H", s2ch.favH.move, 16384);
                setInt("FAV_CHANGE_H", s2ch.favH.change, 4096);
                setInt("FAV_DEL_H", s2ch.favH.del, 32768);
                setInt("FAV_SHIFT_H", s2ch.favH.shift, 512);
                setInt("FAV_OK_V", s2ch.favV.ok, 256);
                setInt("FAV_MOVEITS_V", s2ch.favV.move, 16384);
                setInt("FAV_CHANGE_V", s2ch.favV.change, 4096);
                setInt("FAV_DEL_V", s2ch.favV.del, 32768);
                setInt("FAV_SHIFT_V", s2ch.favV.shift, 512);

                setInt("FAV_SORT_H", s2ch.favH.sort, 8192);
                setInt("FAV_2CHSEARCH_H", s2ch.favH.search2ch, 32768);
                setInt("FAV_UPDATE_H", s2ch.favH.update, 4096);
                setInt("FAV_SORT_V", s2ch.favV.sort, 8192);
                setInt("FAV_2CHSEARCH_V", s2ch.favV.search2ch, 32768);
                setInt("FAV_UPDATE_V", s2ch.favV.update, 4096);

                setInt("SEARCH_OK_H", s2ch.findH.ok, 8192);
                setInt("SEARCH_ESC_H", s2ch.findH.esc, 16384);
                setInt("SEARCH_ITA_H", s2ch.findH.ita, 32768);
                setInt("SEARCH_FAV_H", s2ch.findH.fav, 4096);
                setInt("SEARCH_SHIFT_H", s2ch.findH.shift, 512);
                setInt("SEARCH_OK_V", s2ch.findV.ok, 256);
                setInt("SEARCH_ESC_V", s2ch.findV.esc, 16384);
                setInt("SEARCH_ITA_V", s2ch.findV.ita, 32768);
                setInt("SEARCH_FAV_V", s2ch.findV.fav, 4096);
                setInt("SEARCH_SHIFT_V", s2ch.findV.shift, 512);

                setInt("SEARCH_2CHSEARCH_H", s2ch.findH.search2ch, 32768);
                setInt("SEARCH_2CHSEARCH_V", s2ch.findV.search2ch, 32768);

                setInt("MENUWIN_OK_H", s2ch.menuWinH.ok, 8192);
                setInt("MENUWIN_ESC_H", s2ch.menuWinH.esc, 16384);
                setInt("MENUWIN_OK_V", s2ch.menuWinV.ok, 256);
                setInt("MENUWIN_ESC_V", s2ch.menuWinV.esc, 16384);

                setInt("MENUNG_DEL_H", s2ch.menuNGH.del, 8192);
                setInt("MENUNG_ESC_H", s2ch.menuNGH.esc, 16384);
                setInt("MENUNG_DEL_V", s2ch.menuNGV.del, 256);
                setInt("MENUNG_ESC_V", s2ch.menuNGV.esc, 16384);

                free(buf);
                return;
            }
            else
            {
                free(buf);
            }
        }
    }
    s2ch.btnResH.form     = PSP_CTRL_CIRCLE;
    s2ch.btnResH.back     = PSP_CTRL_CROSS;
    s2ch.btnResH.reload   = PSP_CTRL_TRIANGLE;
    s2ch.btnResH.datDel   = PSP_CTRL_SQUARE;
    s2ch.btnResH.change   = PSP_CTRL_RTRIGGER;

    s2ch.btnResH.s.top      = PSP_CTRL_UP;
    s2ch.btnResH.s.end      = PSP_CTRL_DOWN;
    s2ch.btnResH.addFav   = PSP_CTRL_CIRCLE;
    s2ch.btnResH.delFav   = PSP_CTRL_SQUARE;

    s2ch.btnResH.resForm  = PSP_CTRL_CIRCLE;
    s2ch.btnResH.resFBack  = PSP_CTRL_CROSS;

    s2ch.btnResH.idView   = PSP_CTRL_CIRCLE;
    s2ch.btnResH.idNG     = PSP_CTRL_SQUARE;
    s2ch.btnResH.idBack  = PSP_CTRL_CROSS;

    s2ch.btnResH.resView  = PSP_CTRL_CIRCLE;
    s2ch.btnResH.resMove  = PSP_CTRL_TRIANGLE;
    s2ch.btnResH.resBack  = PSP_CTRL_CROSS;

    s2ch.btnResH.url      = PSP_CTRL_CIRCLE;
    s2ch.btnResH.urlBack  = PSP_CTRL_CROSS;

    s2ch.btnResH.s.up       = PSP_CTRL_UP;
    s2ch.btnResH.s.pUp      = PSP_CTRL_LEFT;
    s2ch.btnResH.s.down     = PSP_CTRL_DOWN;
    s2ch.btnResH.s.pDown    = PSP_CTRL_RIGHT;

    s2ch.btnResV.form     = PSP_CTRL_CIRCLE;
    s2ch.btnResV.back     = PSP_CTRL_CROSS;
    s2ch.btnResV.reload   = PSP_CTRL_TRIANGLE;
    s2ch.btnResV.datDel   = PSP_CTRL_SQUARE;
    s2ch.btnResV.change   = PSP_CTRL_RTRIGGER;

    s2ch.btnResV.s.top      = PSP_CTRL_RIGHT;
    s2ch.btnResV.s.end      = PSP_CTRL_LEFT;
    s2ch.btnResV.addFav   = PSP_CTRL_CIRCLE;
    s2ch.btnResV.delFav   = PSP_CTRL_SQUARE;

    s2ch.btnResV.resForm  = PSP_CTRL_CIRCLE;
    s2ch.btnResV.resFBack  = PSP_CTRL_CROSS;

    s2ch.btnResV.idView   = PSP_CTRL_CIRCLE;
    s2ch.btnResV.idNG     = PSP_CTRL_SQUARE;
    s2ch.btnResV.idBack  = PSP_CTRL_CROSS;

    s2ch.btnResV.resView  = PSP_CTRL_CIRCLE;
    s2ch.btnResV.resMove  = PSP_CTRL_TRIANGLE;
    s2ch.btnResV.resBack  = PSP_CTRL_CROSS;

    s2ch.btnResV.url      = PSP_CTRL_CIRCLE;
    s2ch.btnResV.urlBack  = PSP_CTRL_CROSS;

    s2ch.btnResV.s.up       = PSP_CTRL_RIGHT;
    s2ch.btnResV.s.pUp      = PSP_CTRL_UP;
    s2ch.btnResV.s.down     = PSP_CTRL_LEFT;
    s2ch.btnResV.s.pDown    = PSP_CTRL_DOWN;

    s2ch.listH.up     = PSP_CTRL_UP;
    s2ch.listH.pUp    = PSP_CTRL_LEFT;
    s2ch.listH.down   = PSP_CTRL_DOWN;
    s2ch.listH.pDown  = PSP_CTRL_RIGHT;
    s2ch.listH.top    = PSP_CTRL_UP;
    s2ch.listH.end    = PSP_CTRL_DOWN;
    s2ch.listV.up     = PSP_CTRL_RIGHT;
    s2ch.listV.pUp    = PSP_CTRL_UP;
    s2ch.listV.down   = PSP_CTRL_LEFT;
    s2ch.listV.pDown  = PSP_CTRL_DOWN;
    s2ch.listV.top    = PSP_CTRL_RIGHT;
    s2ch.listV.end    = PSP_CTRL_LEFT;

    s2ch.itaH.ok = PSP_CTRL_CIRCLE;
    s2ch.itaH.esc = PSP_CTRL_CROSS;
    s2ch.itaH.move = PSP_CTRL_SQUARE;
    s2ch.itaH.reload = PSP_CTRL_TRIANGLE;
    s2ch.itaH.shift = PSP_CTRL_RTRIGGER;
    s2ch.itaV.ok = PSP_CTRL_LTRIGGER;
    s2ch.itaV.esc = PSP_CTRL_CROSS;
    s2ch.itaV.move = PSP_CTRL_SQUARE;
    s2ch.itaV.reload = PSP_CTRL_TRIANGLE;
    s2ch.itaV.shift = PSP_CTRL_RTRIGGER;

    s2ch.itaH.addFav = PSP_CTRL_TRIANGLE;
    s2ch.itaH.search2ch = PSP_CTRL_SQUARE;
    s2ch.itaV.addFav = PSP_CTRL_TRIANGLE;
    s2ch.itaV.search2ch = PSP_CTRL_SQUARE;

    s2ch.thH.ok = PSP_CTRL_CIRCLE;
    s2ch.thH.esc = PSP_CTRL_CROSS;
    s2ch.thH.move = PSP_CTRL_SQUARE;
    s2ch.thH.reload = PSP_CTRL_TRIANGLE;
    s2ch.thH.shift = PSP_CTRL_RTRIGGER;
    s2ch.thV.ok = PSP_CTRL_LTRIGGER;
    s2ch.thV.esc = PSP_CTRL_CROSS;
    s2ch.thV.move = PSP_CTRL_SQUARE;
    s2ch.thV.reload = PSP_CTRL_TRIANGLE;
    s2ch.thV.shift = PSP_CTRL_RTRIGGER;

    s2ch.thH.sort = PSP_CTRL_CIRCLE;
    s2ch.thH.search = PSP_CTRL_TRIANGLE;
    s2ch.thH.search2ch = PSP_CTRL_SQUARE;
    s2ch.thV.sort = PSP_CTRL_CIRCLE;
    s2ch.thV.search = PSP_CTRL_TRIANGLE;
    s2ch.thV.search2ch = PSP_CTRL_SQUARE;

    s2ch.favH.ok = PSP_CTRL_CIRCLE;
    s2ch.favH.move = PSP_CTRL_CROSS;
    s2ch.favH.change = PSP_CTRL_TRIANGLE;
    s2ch.favH.del = PSP_CTRL_SQUARE;
    s2ch.favH.shift = PSP_CTRL_RTRIGGER;
    s2ch.favV.ok = PSP_CTRL_LTRIGGER;
    s2ch.favV.move = PSP_CTRL_CROSS;
    s2ch.favV.change = PSP_CTRL_TRIANGLE;
    s2ch.favV.del = PSP_CTRL_SQUARE;
    s2ch.favV.shift = PSP_CTRL_RTRIGGER;

    s2ch.favH.sort = PSP_CTRL_CIRCLE;
    s2ch.favH.search2ch = PSP_CTRL_SQUARE;
    s2ch.favH.update = PSP_CTRL_TRIANGLE;
    s2ch.favV.sort = PSP_CTRL_CIRCLE;
    s2ch.favV.search2ch = PSP_CTRL_SQUARE;
    s2ch.favV.update = PSP_CTRL_TRIANGLE;

    s2ch.findH.ok = PSP_CTRL_CIRCLE;
    s2ch.findH.esc = PSP_CTRL_CROSS;
    s2ch.findH.ita = PSP_CTRL_SQUARE;
    s2ch.findH.fav = PSP_CTRL_TRIANGLE;
    s2ch.findH.shift = PSP_CTRL_RTRIGGER;
    s2ch.findV.ok = PSP_CTRL_CIRCLE;
    s2ch.findV.esc = PSP_CTRL_CROSS;
    s2ch.findV.ita = PSP_CTRL_SQUARE;
    s2ch.findV.fav = PSP_CTRL_TRIANGLE;
    s2ch.findV.shift = PSP_CTRL_RTRIGGER;

    s2ch.findH.search2ch = PSP_CTRL_SQUARE;
    s2ch.findV.search2ch = PSP_CTRL_SQUARE;

    s2ch.menuWinH.ok = PSP_CTRL_CIRCLE;
    s2ch.menuWinH.esc = PSP_CTRL_CROSS;
    s2ch.menuWinV.ok = PSP_CTRL_RTRIGGER;
    s2ch.menuWinV.esc = PSP_CTRL_CROSS;

    s2ch.menuNGH.del = PSP_CTRL_CIRCLE;
    s2ch.menuNGH.esc = PSP_CTRL_CROSS;
    s2ch.menuNGV.del = PSP_CTRL_RTRIGGER;
    s2ch.menuNGV.esc = PSP_CTRL_CROSS;
}
