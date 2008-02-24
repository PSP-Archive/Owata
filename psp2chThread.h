#ifndef __PSP2CH_THREAD_H__
#define __PSP2CH_THREAD_H__


typedef struct {
    int id;
    int dat;
    char title[128];
    int res;
    int old;
} S_2CH_THREAD;

int psp2chThread(void);
int psp2chThreadList(int ita);
int psp2chGetSubject(int ita);
void psp2chSort(int sort);
void psp2chThreadSort(void);
int psp2chThreadSearch(void);
void psp2chDrawThread(int scrollX);

#endif
