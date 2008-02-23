// Cat_Network.h
// ネットワークの初期化と接続処理など

/*
void draw_callback(void* pvUserData ) {
	// 描画処理
	// ダイアログ表示のさいの背景を描画する
	sceGuStart() ～ sceGuFinish()
}

void screen_update_callback(void* pvUserData ) {
	// 更新処理
	// フレームバッファのスワップなど
	sceGuSync( 0, 0 ); sceDisplayWaitVblankStartCB(); sceGuSwapBuffers(); など
}


	// ネットワークライブラリの初期化
	Cat_NetworkInit();
	...
	...

	// 接続する
	pvUserData = 0; // コールバック関数での第一引数となる
	rc = Cat_NetworkConnect( draw_callback, screen_update_callback, pvUserData );
	if(rc < 0) {
		error
	} else if(rc > 0) {
		cancel
	} else {
		connect
	}

	...
	...
	// ネットワークライブラリの終了処理
	Cat_NetworkTerm();

注意
 ・ネットワークモジュールをあらかじめ読み込んでおく必要がある
 ・1.0/1.50はkernelModeなスレッドで読み込んでおくこと

*/

#ifndef INCL_Cat_Network_h
#define INCL_Cat_Network_h

#ifdef __cplusplus
extern "C" {
#endif

#include <pspkerneltypes.h>

// 接続関連

// ネットワークの初期化
extern int Cat_NetworkInit();

// ネットワークの終了処理
extern void Cat_NetworkTerm();

// ネットワーク接続処理
// 返り値 < 0 : エラー
// 返り値 = 0 : 接続した
// 返り値 > 0 : キャンセルされた
extern int Cat_NetworkConnect( void (*draw_callback)(void*), void (*screen_update_callback)(void*), void* pvUserData );

// ネットワーク接続状態を取得
extern int Cat_NetworkIsConnect( void );

#ifdef __cplusplus
}
#endif

#endif
