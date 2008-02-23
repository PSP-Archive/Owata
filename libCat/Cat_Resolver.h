// Cat_Resolver.h
// 名前解決

#ifndef INCL_Cat_Resolver_h
#define INCL_Cat_Resolver_h

#if defined(__cplusplus)
extern "C" {
#endif

#include <pspkerneltypes.h>
#include <pspnet_resolver.h>

// リゾルバの初期化
extern int Cat_ResolverInitEngine( void );

// リゾルバの終了処理
extern void Cat_ResolverTermEngine( void );

// 名前解決
extern int Cat_ResolverURL( const char* pszURL, struct in_addr* pDest );

#if defined(__cplusplus)
};
#endif

#endif
