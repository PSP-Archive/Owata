// Cat_Resolver.h
// ���O����

#ifndef INCL_Cat_Resolver_h
#define INCL_Cat_Resolver_h

#if defined(__cplusplus)
extern "C" {
#endif

#include <pspkerneltypes.h>
#include <pspnet_resolver.h>

// ���]���o�̏�����
extern int Cat_ResolverInitEngine( void );

// ���]���o�̏I������
extern void Cat_ResolverTermEngine( void );

// ���O����
extern int Cat_ResolverURL( const char* pszURL, struct in_addr* pDest );

#if defined(__cplusplus)
};
#endif

#endif
