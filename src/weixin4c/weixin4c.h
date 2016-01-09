#ifndef _H_WEIXIN4C_
#define _H_WEIXIN4C_

#include "weixin4c_public.h"

#include "IDL_xml.dsc.h"

typedef int funcInitEnvProc();
typedef int funcCleanEnvProc();
funcInitEnvProc InitEnvProc ;
funcCleanEnvProc CleanEnvProc ;

typedef int funcReceiveEventProc( xml *p_req , char *output_buffer , int *p_output_buflen , int output_bufsize );
funcReceiveEventProc ReceiveEventProc ;

typedef int funcReceiveTextProc( xml *p_req , char *output_buffer , int *p_output_buflen , int output_bufsize );
typedef int funcReceiveImageProc( xml *p_req , char *output_buffer , int *p_output_buflen , int output_bufsize );
typedef int funcReceiveVoiceProc( xml *p_req , char *output_buffer , int *p_output_buflen , int output_bufsize );
typedef int funcReceiveVideoProc( xml *p_req , char *output_buffer , int *p_output_buflen , int output_bufsize );
typedef int funcReceiveShortVideoProc( xml *p_req , char *output_buffer , int *p_output_buflen , int output_bufsize );
typedef int funcReceiveLocationProc( xml *p_req , char *output_buffer , int *p_output_buflen , int output_bufsize );
typedef int funcReceiveLinkProc( xml *p_req , char *output_buffer , int *p_output_buflen , int output_bufsize );
funcReceiveTextProc ReceiveTextProc ;
funcReceiveImageProc ReceiveImageProc ;
funcReceiveVoiceProc ReceiveVoiceProc ;
funcReceiveVideoProc ReceiveVideoProc ;
funcReceiveShortVideoProc ReceiveShortVideoProc ;
funcReceiveLocationProc ReceiveLocationProc ;
funcReceiveLinkProc ReceiveLinkProc ;

struct Weixin4cProcFuncs
{
	funcInitEnvProc			*pfuncInitEnvProc ;
	funcCleanEnvProc		*pfuncCleanEnvProc ;
	
	funcReceiveEventProc		*pfuncReceiveEventProc ;
	
	funcReceiveTextProc		*pfuncReceiveTextProc ;
	funcReceiveImageProc		*pfuncReceiveImageProc ;
	funcReceiveVoiceProc		*pfuncReceiveVoiceProc ;
	funcReceiveVideoProc		*pfuncReceiveVideoProc ;
	funcReceiveShortVideoProc	*pfuncReceiveShortVideoProc ;
	funcReceiveLocationProc		*pfuncReceiveLocationProc ;
	funcReceiveLinkProc		*pfuncReceiveLinkProc ;
} ;

#define WEIXIN4C_RUNMODE_DEBUG		1
#define WEIXIN4C_RUNMODE_PRODUCT	0

struct Weixin4cConfig
{
	int				run_mode ;
	char				*home ;
	char				*project_name ;
	
	struct Weixin4cProcFuncs	funcs ;
} ;

int weixin4c( struct Weixin4cConfig *pconf );

#endif

