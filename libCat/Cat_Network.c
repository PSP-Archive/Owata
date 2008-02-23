// Cat_Network.c
// ネットワークの初期化と接続処理など

#include <pspkerneltypes.h>
#include <pspthreadman.h>
#include <pspkerror.h>
#include <psputils.h>
#include <psputility.h>
#include <pspnet.h>
#include <pspnet_inet.h>
#include <pspnet_apctl.h>
#include <pspnet_resolver.h>
#include <pspwlan.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "Cat_Network.h"
#include "Cat_Resolver.h"

#ifdef DEBUG_TRACE
#define TRACE(x) pspDebugScreenPrintf x
#else
#define TRACE(x)
#endif

#define MALLOC(x) malloc(x)
#define FREE(x) free(x)
#define STRDUP(x) strdup(x)

static int gfInitialized = 0;

// ネットワークの初期化
int
Cat_NetworkInit()
{
    int rc;

    if(gfInitialized == 0) {

        rc = sceNetInit( 256*1024, 0x12, 0, 0x12, 0 );
        if(rc < 0) {
            rc = sceNetInit( 256*1024, 0x12, 0x1000, 0x12, 0x1000 );
        }
        if(rc < 0) {
            return rc;
        }

        rc = sceNetInetInit();
        if(rc < 0) {
            sceNetTerm();
            return rc;
        }

        rc = sceNetApctlInit( 0x8000, 0x13 );
        if(rc < 0) {
            sceNetInetTerm();
            sceNetTerm();
            return rc;
        }
        gfInitialized = 1;
    }

    return 0;
}

// ネットワークの切断と終了処理
void
Cat_NetworkTerm()
{
    if(gfInitialized) {
        sceNetApctlDisconnect();
        while(Cat_NetworkIsConnect()) {
            sceKernelDelayThreadCB( 100 * 1000 );
        }
        sceNetApctlTerm();
        sceNetInetTerm();
        sceNetTerm();

        gfInitialized = 0;
    }
}

// ボタンのスワップ情報を取得
// 0: OK=CIRCLE
// 1: OK=CROSS
static int
GetButtonSwap()
{
    int buttonswap;
    int rc;

    rc = sceUtilityGetSystemParamInt( PSP_SYSTEMPARAM_ID_INT_UNKNOWN,  &buttonswap ); // X/O button swap
    if(rc == 0) {
        /* ボタンの入れ替え情報が取得できたら、それに従う */
        if(buttonswap == 0) {
            rc = 0;
        } else {
            rc = 1;
        }
    } else {
        /* 言語設定で判定する */
        /* 多分1.0の場合 */
        int lang;
        rc = sceUtilityGetSystemParamInt( PSP_SYSTEMPARAM_ID_INT_LANGUAGE,  &lang );
        if(rc) {
            /* 言語情報も取得できなかったら英語にしとく */
            lang = PSP_SYSTEMPARAM_LANGUAGE_ENGLISH;
        }
        switch(lang) {
            case PSP_SYSTEMPARAM_LANGUAGE_JAPANESE:
            case PSP_SYSTEMPARAM_LANGUAGE_KOREAN:
            case PSP_SYSTEMPARAM_LANGUAGE_CHINESE_TRADITIONAL:
            case PSP_SYSTEMPARAM_LANGUAGE_CHINESE_SIMPLIFIED:
                rc = 0;
                break;
            default:
                rc = 1;
                break;
        }
    }
    return rc;
}

// ダイアログ共通部分の初期化
static void
ConfigureDialog( pspUtilityMsgDialogParams* dialog, size_t dialog_size )
{
    memset( dialog, 0, dialog_size );

    dialog->base.size = dialog_size;
    sceUtilityGetSystemParamInt( PSP_SYSTEMPARAM_ID_INT_LANGUAGE, &dialog->base.language );  // Prompt language
    dialog->base.buttonSwap = GetButtonSwap(); // X/O button swap

    dialog->base.graphicsThread = 0x11;
    dialog->base.accessThread   = 0x13;
    dialog->base.fontThread     = 0x12;
    dialog->base.soundThread    = 0x10;
}

// エラーダイアログの表示開始
static int
ShowMessageDialogError( int nErrorCode )
{
    static pspUtilityMsgDialogParams dialog;

    ConfigureDialog(&dialog, sizeof(dialog));
    dialog.mode       = 0;
    dialog.errorValue = nErrorCode;

    return sceUtilityMsgDialogInitStart( &dialog );
}

// ネットワーク接続処理
// 返り値 < 0 : エラー
// 返り値 = 0 : 接続した
// 返り値 > 0 : キャンセルされた
int
Cat_NetworkConnect( void (*draw_callback)(void*), void (*screen_update_callback)(void*), void* pvUserData )
{
    static char pBuffer[sizeof(pspUtilityNetconfData) + 64];
    pspUtilityNetconfData* pConf;
    int nState;
    int rc;
    int fFinish;

    rc = Cat_NetworkInit();
    if(rc < 0) {
        return rc;
    }

    pConf = (pspUtilityNetconfData*)pBuffer;
    memset( pConf, 0, sizeof(pspUtilityNetconfData) + 64 );
    pConf->base.size = sizeof(pspUtilityNetconfData) + 64;

    sceUtilityGetSystemParamInt( PSP_SYSTEMPARAM_ID_INT_LANGUAGE, &pConf->base.language ); // Prompt language
    pConf->base.buttonSwap     = GetButtonSwap(); // X/O button swap
    pConf->action              = PSP_NETCONF_ACTION_CONNECTAP;
    pConf->base.graphicsThread = 0x11;
    pConf->base.accessThread   = 0x13;
    pConf->base.fontThread     = 0x12;
    pConf->base.soundThread    = 0x10;

    rc = sceUtilityNetconfInitStart( pConf );
    while((unsigned int)rc == 0x80110004U && pConf->base.size >= 4) {
        pConf->base.size = pConf->base.size - 4;
        rc = sceUtilityNetconfInitStart( pConf );
    }
    if(rc < 0) {
        return rc;
    }

    nState = 1;
    fFinish = 0;
    while(!fFinish) {
        switch(nState) {
            case 0:
                rc = sceUtilityNetconfInitStart( pConf );
                if(rc < 0) {
                    nState = 10;
                }
                nState = 1;
                break;
            case 1:
                if(sceUtilityNetconfGetStatus() == PSP_UTILITY_DIALOG_QUIT) {
                    sceUtilityNetconfShutdownStart();
                    nState = 2;
                }
                break;
            case 2:
                if(sceUtilityNetconfGetStatus() == PSP_UTILITY_DIALOG_NONE) {
                    /* 接続完了 */
                    rc = pConf->base.result;
                    if(rc < 0) {
                        /* エラーが発生した エラーメッセージでも出しとこう */
                        nState = 10;
                    } else {
                        if(rc > 0) {
                            /* キャンセルされたので */
                            Cat_NetworkTerm();
                        } else {
                            Cat_ResolverInitEngine();
                        }
                        fFinish = 1;
                    }
                }
                break;

            case 10:
                rc = ShowMessageDialogError( rc );
                if(rc < 0) {
                    /* エラーが発生した */
                    fFinish = 1;
                    Cat_NetworkTerm();
                } else {
                    nState = 11;
                }
                break;
            case 11:
                if(sceUtilityMsgDialogGetStatus() == PSP_UTILITY_DIALOG_QUIT) {
                    sceUtilityMsgDialogShutdownStart();
                    nState = 12;
                }
                break;
            case 12:
                if(sceUtilityMsgDialogGetStatus() == PSP_UTILITY_DIALOG_NONE) {
                    nState = 0;
                }
                break;
        }

        if(draw_callback) {
            draw_callback( pvUserData );
        }

        if(sceUtilityNetconfGetStatus() == PSP_UTILITY_DIALOG_VISIBLE) {
            sceUtilityNetconfUpdate( 2 );
        }
        if(sceUtilityMsgDialogGetStatus() == PSP_UTILITY_DIALOG_VISIBLE) {
            sceUtilityMsgDialogUpdate( 2 );
        }

        if(screen_update_callback) {
            screen_update_callback( pvUserData );
        }
    }
    return rc;
}

// ネットワーク接続状態を取得
int
Cat_NetworkIsConnect( void )
{
    int nState;
    if(sceNetApctlGetState( &nState ) < 0) {
        return 0;
    }
    return (nState == 4) ? 1 : 0;
}
