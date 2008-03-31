/*
* $Id$
*/

#ifndef __PSP2CH_THREAD_H__
#define __PSP2CH_THREAD_H__

void psp2chThreadSetMenuString(void);
int psp2chThread(int retSel);
void psp2chThreadGetTitle(char* dir, int dat, char* title, int len);
int psp2chThreadList(int ita);
int psp2chGetSubject(int ita);
void psp2chSort(int sort);
void psp2chThreadSort(void);
int psp2chThreadSearch(void);
void psp2chDrawThread(void);

#endif
