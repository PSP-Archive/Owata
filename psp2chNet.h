/*
* $Id$
*/

#ifndef __PSP2CH_NET_H__
#define __PSP2CH_NET_H__

typedef struct {
    int Content_Length;
    char Content_Type[32];
    char Last_Modified[32];
    char ETag[32];
} HTTP_HEADERS;

typedef struct {
    int status;
    HTTP_HEADERS head;
    char* body;
    int length;
} S_NET;

int psp2chNetInit(void);
void psp2chNetTerm(void);
int psp2chOpenSocket(void);
int psp2chCloseSocket(void);
int psp2chResolve(const char* host, struct in_addr* addr);
int psp2chGet(const char* host, const char* path, const char* header, char* cook, S_NET* net);
int psp2chPost(char* host, char* dir, int dat, char* cook, S_NET* net);
int psp2chRequest(const char* host, const char* path, const char* requestText, S_NET* net);
int psp2chResponse(const char* host, const char* path, S_NET* net);
int psp2chGetStatusLine(void);
int psp2chGetHttpHeaders(S_NET* net, char* cookie);
int psp2chGetHttpBody(S_NET* net);
int psp2chApConnect(void);

#endif
