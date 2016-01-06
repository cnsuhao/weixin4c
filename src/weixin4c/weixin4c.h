#ifndef _H_PRIVATE_
#define _H_PRIVATE_

#include "weixin4c_public.h"
#include "weixin4c_proc.h"

#include "openssl/sha.h"

#include "IDL_xml.dsc.h"

struct Environment
{
	void		*so_handler ;
	struct ProcFuncs
	{
		funcInitEnvProc			*pfuncInitEnvProc ;
		funcCleanEnvProc		*pfuncCleanEnvProc ;
		
		funcReceiveSubscribeEventProc	*pfuncReceiveSubscribeEventProc ;
		funcReceiveUnsubscribeEventProc	*pfuncReceiveUnsubscribeEventProc ;
		funcReceiveTextMsgProc		*pfuncReceiveTextMsgProc ;
	} funcs ;
} ;

int cgiinit( struct Environment *penv );
int cgimain( struct Environment *penv );
int cgiclean( struct Environment *penv );

int VerifyServer( struct Environment *penv , char *signature , char *timestamp , char *nonce , char *echostr );
int ReceiveEvent( struct Environment *penv , char *post_data , int post_data_len , xml *p_req );
int ReceiveText( struct Environment *penv , char *post_data , int post_data_len , xml *p_req );

#endif

