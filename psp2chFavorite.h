#ifndef __PSP2CH_FAVORITE_H__
#define __PSP2CH_FAVORITE_H__


typedef struct {
    char host[32];
    char dir[32];
    char title[32];
    int dat;
    char subject[128];
} S_2CH_FAVORITE;

int psp2chFavorite(void);
int psp2chLoadFavorite(void);
int psp2chAddFavorite(char* host, char* dir, char* title, int dat);
int psp2chDelFavorite(char* title, int dat);
void psp2chDrawFavorite(int scrollX);

#endif
