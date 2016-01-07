#ifndef _H_WEIXIN4C_PROC_
#define _H_WEIXIN4C_PROC_

#include "weixin4c_public.h"

typedef int funcInitEnvProc();
typedef int funcCleanEnvProc();
funcInitEnvProc InitEnvProc ;
funcCleanEnvProc CleanEnvProc ;

typedef int funcReceiveSubscribeEventProc( char *output_buffer , int *p_output_buflen , int output_bufsize );
typedef int funcReceiveUnsubscribeEventProc( char *output_buffer , int *p_output_buflen , int output_bufsize );
typedef int funcReceiveTextProc( char *input_buffer , int input_buflen , int input_bufsize , char *output_buffer , int *p_output_buflen , int output_bufsize );
funcReceiveSubscribeEventProc ReceiveSubscribeEventProc ;
funcReceiveUnsubscribeEventProc ReceiveUnsubscribeEventProc ;
funcReceiveTextProc ReceiveTextProc ;

struct Weixin4cProcFuncs
{
	funcInitEnvProc			*pfuncInitEnvProc ;
	funcCleanEnvProc		*pfuncCleanEnvProc ;
	
	funcReceiveSubscribeEventProc	*pfuncReceiveSubscribeEventProc ;
	funcReceiveUnsubscribeEventProc	*pfuncReceiveUnsubscribeEventProc ;
	funcReceiveTextProc		*pfuncReceiveTextProc ;
} ;

struct Weixin4cConfig
{
	char				*home ;
	
	struct Weixin4cProcFuncs	funcs ;
} ;

int weixin4c( struct Weixin4cConfig *pconf );

#endif

