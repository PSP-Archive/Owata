/*
* $Id$
*/

#ifndef __PSP2CH_FAVORITE_H__
#define __PSP2CH_FAVORITE_H__

void psp2chFavSetMenuString(void);
int psp2chFavorite(void);
int psp2chGetResCount(char* title, int dat);
int psp2chLoadFavorite(void);
int psp2chLoadFavoriteIta(void);
int psp2chAddFavorite(char* host, char* dir, char* title, int dat);
int psp2chAddFavoriteIta(char* cate, char* title);
int psp2chDelFavorite(char* title, int dat);
int psp2chDelFavoriteIta(int index);
void psp2chDrawFavorite(int scrollX);
void psp2chDrawFavoriteIta(void);

#endif
