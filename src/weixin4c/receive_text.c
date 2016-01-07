#include "weixin4c_public.h"
#include "weixin4c_private.h"

int ReceiveText( struct Weixin4cEnv *penv , char *post_data , int post_data_len , xml *p_req )
{
	xml	rsp ;
	char	output_buffer[ 4096 * 100 ] ;
	int	output_buflen ;
	char	rsp_buffer[ 4096 * 110 ] ;
	int	rsp_buflen ;
	
	int	nret = 0 ;
	
	InfoLog( __FILE__ , __LINE__ , "req xml[%.*s]" , post_data_len , post_data );
	
	if( memcmp( p_req->Content , "<![CDATA[" , 9 ) == 0 )
	{
		int	len ;
		len = strlen(p_req->Content) ;
		p_req->Content[len-3] = '\0' ;
		memmove( p_req->Content , p_req->Content+9 , len-3-9+1 );
	}
	
	memset( output_buffer , 0x00 , sizeof(output_buffer) );
	output_buflen = 0 ;
	nret = penv->pconf->funcs.pfuncReceiveTextProc( p_req->Content , strlen(p_req->Content) , sizeof(p_req->Content) , output_buffer , & output_buflen , sizeof(output_buffer) ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "pfuncReceiveTextProc failed[%d]" , nret );
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "pfuncReceiveTextProc ok" );
	}
	
	memset( & rsp , 0x00 , sizeof(xml) );
	strcpy( rsp.ToUserName , p_req->FromUserName );
	strcpy( rsp.FromUserName , p_req->ToUserName );
	snprintf( rsp.CreateTime , sizeof(rsp.CreateTime) , "%d" , (int)time(NULL) );
	strcpy( rsp.MsgType , p_req->MsgType );
	snprintf( rsp.Content , sizeof(rsp.Content)-1 , "<![CDATA[%s]]>" , output_buffer );
	strcpy( rsp.MsgId , p_req->MsgId );
	
	memset( rsp_buffer , 0x00 , sizeof(rsp_buffer) );
	rsp_buflen = sizeof(rsp_buffer) - 1 ;
	nret = DSCSERIALIZE_XML_xml( & rsp , "GB18030" , rsp_buffer , & rsp_buflen ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "DSCSERIALIZE_XML_xml failed[%d]" , nret );
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "DSCSERIALIZE_XML_xml ok" );
		InfoLog( __FILE__ , __LINE__ , "rsp xml[%.*s]" , rsp_buflen-41 , rsp_buffer+41 );
		printf( "%.*s" , rsp_buflen-41 , rsp_buffer+41 );
	}
	
	return 0;
}

