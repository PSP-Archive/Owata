/*
* $Id$
*/

#include "psp2ch.h"
#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include <time.h>
#include <arpa/inet.h>
#include <pspwlan.h>
#include "psp2chNet.h"
#include "pg.h"
#include "utf8.h"
#include "libCat/Cat_Network.h"
#include "libCat/Cat_Resolver.h"

extern S_2CH s2ch;
extern char* ver;

const char* userAgent = "Monazilla/1.00 (Compatible; PSP; ja) owata(^o^)";

/*****************************
アクセスポイントへ接続してソケット作成
成功時にはソケット(>=0)を返す
失敗時には<0を返す
psp2chRequest()とpsp2chPost()から呼ばれます。
*****************************/
int psp2chOpenSocket(void)
{
    int mySocket, ret;

    ret = psp2chApConnect();
    sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
    if (ret < 0 && ret != 0x80110405)
    {
        return ret;
    }
    mySocket = socket(PF_INET, SOCK_STREAM, 0 );
#ifdef DEBUG
    pspDebugScreenPrintf("socket = %d\n", mySocket);
#endif
    return mySocket;
}

/*****************************
ソケットを閉じる
*****************************/
int psp2chCloseSocket(int mySocket)
{
    shutdown(mySocket, SHUT_RDWR);
    return close(mySocket);
}

/*****************************
名前解決　ホスト名をIPアドレスに変換
Sfiya猫さんのパッチ使用
*****************************/
int psp2chResolve(const char* host, struct in_addr* addr)
{
    SceUID fd;
    char buf[256];
    char hosts[2048];
    char *p;

    sprintf(buf, "  %s のアドレスを解決しています", host);
    pgMenuBar(buf);
    sceDisplayWaitVblankStart();
    framebuffer = sceGuSwapBuffers();
    sprintf(buf, "%s/hosts", s2ch.cwDir);
    fd = sceIoOpen(buf, PSP_O_RDONLY, 0777);
    if (fd >= 0)
    {
        sceIoRead(fd, hosts, sizeof(hosts));
        sceIoClose(fd);
        p = strstr(hosts, host);
        if (p)
        {
            *p = '\0';
            p = strrchr(hosts, '\n');
            if (p)
            {
                sscanf(p, " %s", buf);
                if (inet_aton(buf, addr))
                {
                    return 0;
                }
            }
        }
    }
    if(Cat_ResolverURL( host, addr ) < 0 )
    {
        pgMenuBar("  Cat_ResolverURL error");
        sceDisplayWaitVblankStart();
        framebuffer = sceGuSwapBuffers();
        return -1;
    }
    return 0;
}

/*****************************
HTTP で GETリクエスト
ソケットを作成
psp2chRequest();でリクエストヘッダ送信
本文受信
ソケットを閉じる
成功時には0を返す
*****************************/
int psp2chGet(const char* host, const char* path, const char* header, S_NET* net)
{
    int mySocket;
    char requestText[512];
    char *p;

    mySocket = psp2chOpenSocket();
    if (mySocket < 0)
    {
        return mySocket;
    }
    p = strrchr(path, '#');
    if (p)
    {
        *p = '\0';
    }
    sprintf(requestText,
        "GET /%s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "User-Agent: %s/%s\r\n"
        "%s"
        "Connection: close\r\n\r\n"
        , path, host, userAgent, ver, header
    );
    memset(net, 0, sizeof(S_NET));
    // リクエスト送信
    if (psp2chRequest(mySocket, host, path, requestText, net) < 0)
    {
        return -1;
    }
    // Status code 受信
    if ((net->status = psp2chGetStatusLine(mySocket)) <= 0)
    {
        return -1;
    }
    // Response Header 受信
    psp2chGetHttpHeaders(mySocket, net, NULL);
    // Response Body 受信
    if (psp2chResponse(mySocket, host, path, net) < 0)
    {
        return -1;
    }
    // Close
    psp2chCloseSocket(mySocket);
    return 0;
}

/*****************************
HTTP で POST 送信します。
ソケット作成
psp2chRequest();でリクエストヘッダ送信
ボディを送信
ソケットを閉じる
成功時には0を返す
*****************************/
int psp2chPost(char* host, char* dir, int dat, char* cook, S_NET* net)
{
    int mySocket;
    char requestText[512];
    char *path = "test/bbs.cgi";

    mySocket = psp2chOpenSocket();
    if (mySocket < 0)
    {
        return mySocket;
    }
    sprintf(requestText,
        "POST /%s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "User-Agent: %s/%s\r\n"
        "Referer: http://%s/test/read.cgi/%s/%d/l50\r\n"
        "Cookie: %s\r\n"
        "Content-Type: application/x-www-form-urlencoded\r\n"
        "Content-Length: %d\r\n\r\n"
        , path, host, userAgent, ver, host, dir, dat, cook, strlen(net->body)
    );
    // リクエスト送信
    if (psp2chRequest(mySocket, host, path, requestText, net) < 0)
    {
        return -1;
    }
    // 本文送信
    send(mySocket, net->body, strlen(net->body), 0 );
    // Status code 受信
    if ((net->status = psp2chGetStatusLine(mySocket)) <= 0)
    {
        return -1;
    }
    // Response Header 受信
    psp2chGetHttpHeaders(mySocket, net, cook);
    // Response Body 受信
    if (psp2chResponse(mySocket, host, path, net) < 0)
    {
        return -1;
    }
    // Close
    psp2chCloseSocket(mySocket);
    return 0;
}

/*****************************
アドレス解決してリクエストヘッダを送信します
成功時には0を返します
*****************************/
int psp2chRequest(int mySocket, const char* host, const char* path, const char* requestText, S_NET* net)
{
    int ret;
    char buf[512];
    struct sockaddr_in sain;
    struct in_addr addr;

    ret = psp2chResolve(host, &addr);
    if (ret < 0) {
        return ret;
    }
    sprintf(buf, "  %s (%s)", host, inet_ntoa(addr));
    pgMenuBar(buf);
    sceDisplayWaitVblankStart();
    framebuffer = sceGuSwapBuffers();
    // Tell the socket to connect to the IP address we found, on port 80 (HTTP)
    sain.sin_family = AF_INET;
    sain.sin_port = htons(80);
    sain.sin_addr.s_addr = addr.s_addr;
    sprintf(buf, "  http://%s/%s に接続しています", host, path);
    pgMenuBar(buf);
    sceDisplayWaitVblankStart();
    framebuffer = sceGuSwapBuffers();
    ret = connect(mySocket,(struct sockaddr *)&sain, sizeof(sain) );
    if (ret < 0) {
        memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
        strcpy(s2ch.mh.message, "Connect errror");
        pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
        sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
        return ret;
    }
    pgMenuBar("接続しました");
    sceDisplayWaitVblankStart();
    framebuffer = sceGuSwapBuffers();
    // send our request
    send(mySocket, requestText, strlen(requestText), 0 );
    return 0;
}

/*****************************
Response body を受信
*****************************/
int psp2chResponse(int mySocket, const char* host, const char* path, S_NET* net)
{
    char buf[512];
    int ret, size;

    sprintf(buf, "http://%s/%s からデータを転送しています...", host, path);
    pgMenuBar(buf);
    sceDisplayWaitVblankStart();
    framebuffer = sceGuSwapBuffers();
    size = 0;
    net->body = NULL;
    while ((ret = recv(mySocket, buf, sizeof(buf), 0)) > 0)
    {
        net->body = (char*)realloc(net->body, size + ret);
        if (net->body == NULL)
        {
            memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
            sprintf(s2ch.mh.message, "memory error\nnet.body");
            pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
            sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
            psp2chCloseSocket(mySocket);
            return -1;
        }
        memcpy(net->body + size, buf, ret);
        size += ret;
    }
    net->body = (char*)realloc(net->body, size + 1);
    if (net->body == NULL)
    {
        memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
        sprintf(s2ch.mh.message, "memory error\nnet.body");
        pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
        sceCtrlPeekBufferPositive(&s2ch.oldPad, 1);
        psp2chCloseSocket(mySocket);
        return -1;
    }
    net->body[size] = '\0';
    sprintf(buf, "完了");
    pgMenuBar(buf);
    sceDisplayWaitVblankStart();
    framebuffer = sceGuSwapBuffers();
    return 0;
}

/*****************************
ソケットから最初の1行（ステータスライン）を読み込みます。
ステータスコードをintに変換して返す
HTTP/1.1 200 OK
なら200を返す
*****************************/
int psp2chGetStatusLine(int mySocket)
{
    char in;
    char incomingBuffer[256];
    int i = 0;
    int verMajor, verMinor, code;
    char phrase[256];

     do
     {
        if (recv( mySocket, &in, 1, 0 ) == 0)
        {
            return -1;
        }
        incomingBuffer[i] = in;
        if (++i > 255)
        {
            return -1;
        }
    } while (in != '\n');
    incomingBuffer[i] = '\0';
    pgMenuBar(incomingBuffer);
    sceDisplayWaitVblankStart();
    framebuffer = sceGuSwapBuffers();
    sscanf(incomingBuffer, "HTTP/%d.%d %d %s\r\n", &verMajor, &verMinor, &code, phrase);
    return code;
}

/*****************************
ソケットからレスポンスヘッダを読み込みます。
S_NET構造体に
Content-Length
Content-Type
Last-Modified
ETag
の内容がコピーされます。
cookie[]にSet-Cookieの内容が追加されます。
Content-Lengthがintで返されます
Content-Lengthを返さない場合もあるので注意（CGI等）
*****************************/
int psp2chGetHttpHeaders(int mySocket, S_NET* net, char* cookie)
{
    int i = 0;
    char in;
    char incomingBuffer[256];
    int contentLength = 0;
    char *p;

    while(recv(mySocket, &in, 1, 0) > 0)  // if recv returns 0, the socket has been closed.
    {
        if (in == '\r')
        {
            continue;
        }
        incomingBuffer[i] = in;
        i++;
        if (in == '\n')
        {
            incomingBuffer[i] = 0;
            if (incomingBuffer[0] == '\n')  // End of headers
            {
                break;
            }
            if (strstr(incomingBuffer, "Content-Length:"))
            {
                sscanf(incomingBuffer, "Content-Length: %d\n", &contentLength);
                net->head.Content_Length = contentLength;
            }
            else if (strstr(incomingBuffer, "Content-Type:"))
            {
                strcpy(net->head.Content_Type, &incomingBuffer[14]);
            }
            else if (strstr(incomingBuffer, "Last-Modified:"))
            {
                strcpy(net->head.Last_Modified, &incomingBuffer[15]);
            }
            else if (strstr(incomingBuffer, "ETag:"))
            {
                strcpy(net->head.ETag, &incomingBuffer[6]);
            }
            else if (cookie && strstr(incomingBuffer, "Set-Cookie:"))
            {
                p = strchr(incomingBuffer, ';');
                *p = '\0';
                if (cookie[0])
                {
                    strcat(cookie, "; ");
                }
                strcat(cookie, &incomingBuffer[12]);
            }
#ifdef DEBUG
            pspDebugScreenPrintf("%s", incomingBuffer);
#endif
            i = 0;
        }
    }
    return contentLength;
}

static void draw_callback( void* pvUserData )
{
    // 描画処理
    // ダイアログ表示のさいの背景を描画する

    minimalRender();
}

static void screen_update_callback( void* pvUserData ) {
    // 更新処理
    // フレームバッファのスワップなど

    sceGuSync( 0, 0 );
    sceDisplayWaitVblankStartCB();
    framebuffer = sceGuSwapBuffers();
}

/*****************************
指定された設定番号のアクセスポイントに接続します
psp2chApConnect()から呼ばれます。
*****************************/
int connect_to_apctl(int config)
{
    int err;
    int stateLast = -1;
    int state;
    char buf[32];
    clock_t time;

    err = sceNetApctlConnect(config);
    if (err != 0)
    {
        memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
        strcpy(s2ch.mh.message, "sceNetApctlConnect error");
        pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
        return 0;
    }

    pgMenuBar("  Connecting...");
    sceDisplayWaitVblankStart();
    framebuffer = sceGuSwapBuffers();
    time = clock();
    while (1)
    {
        err = sceNetApctlGetState(&state);
        if (err != 0)
        {
            memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
            sprintf(s2ch.mh.message, "sceNetApctlGetState error\n0x%X", err);
            pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
            break;
        }
        if (state == 2)
        {
            strcpy(buf, "　アクセスポイントに接続中です。");
            stateLast = state;
        }
        if (state == 3)
        {
            strcpy(buf, "　IPアドレスを取得中です。");
            stateLast = state;
        }
        if (state == 4)
        {
            break;  // connected with static IP
        }
        pgMenuBar(buf);
        sceDisplayWaitVblankStart();
        framebuffer = sceGuSwapBuffers();
        if (clock() > time + 1000000 * 8)
        {
            break;
        }
    }
    if (state != 4)
    {
        pgMenuBar("  接続に失敗しました。");
    }
    else
    {
        pgMenuBar("  接続しました。");
        Cat_ResolverInitEngine();
    }
    sceDisplayWaitVblankStart();
    framebuffer = sceGuSwapBuffers();
    return 0;
}

/*****************************
PSP内のアクセスポイント設定を検索します
設定が2個以上あれば選択ダイアログを表示します。
設定が1個のみの場合はその値で接続します。
設定がない場合は-1が返されます。
*****************************/
int psp2chApConnect(void)
{
    struct
    {
        int index;
        char name[128];
    } ap[2];
    int count = 0;
    int i;
    char buf[32];

    memset(&s2ch.mh,0,sizeof(MESSAGE_HELPER));
    s2ch.mh.options = PSP_UTILITY_MSGDIALOG_OPTION_TEXT;
    if (sceWlanGetSwitchState() == 0)
    {
        strcpy(s2ch.mh.message, TEXT_1);
        pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
        return -1;
    }
    else if (sceWlanDevIsPowerOn() == 0)
    {
        strcpy(s2ch.mh.message, TEXT_2);
        pspShowMessageDialog(&s2ch.mh, DIALOG_LANGUAGE_AUTO);
        return -1;
    }
    if (sceNetApctlGetInfo(8, buf) == 0)
    {
        // IP取得済み
        return 0;
    }
    pgMenuBar("接続の設定を検索しています...");
    pgWaitVn(2);
    framebuffer = sceGuSwapBuffers();
    for (i = 1; i < 100; i++) // skip the 0th connection
    {
        if (sceUtilityCheckNetParam(i) != 0) continue;  // more
        sceUtilityGetNetParam(i, 0, (netData*) ap[count].name);
        ap[count].index = i;
        count++;
        if (count > 1) break;  // no more room
    }
    if (count == 1)
    {
        return connect_to_apctl(ap[0].index);
    }
    else if (count > 1)
    {
        // 返り値 < 0 : エラー
        // 返り値 = 0 : 接続した
        // 返り値 > 0 : キャンセルされた
        return Cat_NetworkConnect( draw_callback, screen_update_callback, 0 );
        // 全部0なのかな？？？
    }
    return -1;
}
