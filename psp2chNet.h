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

int psp2chOpenSocket(void);
int psp2chCloseSocket(int mySocket);
int psp2chResolve(const char* host, struct in_addr* addr);
int psp2chGet(const char* host, const char* path, const char* header, S_NET* net);
int psp2chPost(char* host, char* dir, int dat, char* cook, S_NET* net);
int psp2chRequest(int mySocket, const char* host, const char* path, const char* requestText, S_NET* net);
int psp2chResponse(int mySocket, const char* host, const char* path, S_NET* net);
int psp2chGetStatusLine(int mySocket);
int psp2chGetHttpHeaders(int mySocket, S_NET* net, char* cookie);
int psp2chApConnect(void);

#endif