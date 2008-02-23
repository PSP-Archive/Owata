#ifndef __PSP2CH_FORM_H__
#define __PSP2CH_FORM_H__


void psp2chUrlEncode(char* dst, char* src);
int psp2chForm(char* host, char* dir, int dat, char* subject, char* message);

#endif
