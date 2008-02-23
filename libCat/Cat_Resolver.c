// Cat_Resolver.c
// 名前解決

#include <malloc.h>
#include <pspthreadman.h>
#include <string.h>
#include <stdlib.h>
#include "Cat_Resolver.h"
#include <arpa/inet.h>

#define RESOLVER_WORK_BUFFER_SIZE (2*1024)

/* URLキャッシュ数 */
#define MAX_URL_CACHE (256)

#define RESOLVER_MALLOC(x) malloc(x)
#define RESOLVER_FREE(x) free(x)

#ifdef DEBUG_TRACE
#define TRACE(x) pspDebugScreenPrintf x
#else
#define TRACE(x)
#endif

typedef struct {
	struct in_addr address;
	char* pszURL;
} AddrSet;

static AddrSet AddressSet[MAX_URL_CACHE];
static int m_nRIndex = 0;
static SceUID m_semaResolver = -1;

// リゾルバ
int resolve_host( struct in_addr* sin_addr, const char* hostname );

// リゾルバの初期化
int
Cat_ResolverInitEngine( void )
{
	int rc;

	Cat_ResolverTermEngine();

	memset( AddressSet, 0, sizeof(AddressSet) );
	m_nRIndex = 0;
	rc = m_semaResolver = sceKernelCreateSema( "semaResolve", 0, 1, 1, 0 );
	if(rc >= 0) {
		rc = sceNetResolverInit();
		if(rc < 0) {
			sceKernelDeleteSema( m_semaResolver );
			m_semaResolver = -1;
		}
	}

	return rc;
}

// リゾルバの終了処理
void
Cat_ResolverTermEngine( void )
{
	if(m_semaResolver >= 0) {
		int i;
		sceKernelWaitSemaCB( m_semaResolver, 1, 0 );

		for(i = 0; i < MAX_URL_CACHE; i++) {
			if(AddressSet[i].pszURL) {
				RESOLVER_FREE( AddressSet[i].pszURL );
			}
		}

		memset( AddressSet, 0, sizeof(AddressSet) );
		m_nRIndex = 0;

		sceKernelDeleteSema( m_semaResolver );
		m_semaResolver = -1;
	}

	sceNetResolverTerm();
}

// 名前解決
int
Cat_ResolverURL( const char* pszURL, struct in_addr* pDest )
{
	int i;
	int rc;

	if(pDest == 0 || pszURL == 0 || m_semaResolver < 0) {
		return -1;
	}

	if(sceKernelWaitSemaCB( m_semaResolver, 1, 0 ) < 0) {
		return -1;
	}

	rc = -1;
	for(i = 0; i < MAX_URL_CACHE; i++) {
		if(AddressSet[i].pszURL && (strcmp( AddressSet[i].pszURL, pszURL ) == 0)) {
			/* 発見した */
			memcpy( pDest, &AddressSet[i].address, sizeof(struct in_addr) );
			rc = 0;
			break;
		}
	}
	if(i >= MAX_URL_CACHE) {
		/* 見つからなかったら、適当に */
		AddrSet* pReprace = &AddressSet[m_nRIndex];
		m_nRIndex = (m_nRIndex + 1) % MAX_URL_CACHE;

		if(pReprace->pszURL) {
			RESOLVER_FREE( pReprace->pszURL );
		}
		pReprace->pszURL = (char*)RESOLVER_MALLOC( strlen( pszURL ) + 1 );
		if(pReprace->pszURL) {
			strcpy( pReprace->pszURL, pszURL );
		}
		memset( &pReprace->address, 0, sizeof(struct in_addr) );

		rc = resolve_host( &pReprace->address, pszURL );
		if(rc >= 0) {
			memcpy( pDest, &pReprace->address, sizeof(struct in_addr) );
			rc = 0;
		} else {
			if(pReprace->pszURL) {
				RESOLVER_FREE( pReprace->pszURL );
			}
			pReprace->pszURL = 0;
			memset( &pReprace->address, 0, sizeof(struct in_addr) );
		}
	}

	sceKernelSignalSema( m_semaResolver, 1 );
	return rc;
}

// リゾルバ
int
resolve_host( struct in_addr* sin_addr, const char* hostname )
{
	int rid = -1;
	char* buf;
	int rc;

	/* 引数チェック */
	if(sin_addr == 0 || hostname == 0) {
		return -1;
	}

	if(('0' <= hostname[0]) && (hostname[0] <= '9')) {
		/* xxx.xxx.xxx.xxx形式かな */
		if(inet_aton( hostname, sin_addr ) != 0) {
			/* 正常に変換できた */
			return 0;
		}
	}

	buf = (char*)RESOLVER_MALLOC( RESOLVER_WORK_BUFFER_SIZE );

	do {
		/* Create a resolver */
		rc = sceNetResolverCreate( &rid, buf, RESOLVER_WORK_BUFFER_SIZE );
		if(rc < 0) {
			TRACE(( "Error creating resolver:error code %08X[%s]\n", rc, hostname ));
			break;
		}

		/* Resolve a name to an ip address */
		rc = sceNetResolverStartNtoA( rid, hostname, sin_addr, 5 * 1000 * 1000, 5 );
		if(rc < 0) {
			TRACE(( "Error sceNetResolverStartNtoA:error code %08X[%s]\n", rc, hostname ));
			break;
		}
	} while(0);

	if(rid >= 0) {
		sceNetResolverDelete( rid );
	}

	if(buf) {
		RESOLVER_FREE( buf );
	}

    return rc;
}
