#include "weixin4c.h"

int ReceiveEvent( struct Environment *penv , char *post_data , int post_data_len , xml *p_req )
{
	xml	rsp ;
	char	output_buffer[ 4096 * 100 ] ;
	int	output_buflen ;
	char	rsp_buffer[ 4096 * 110 ] ;
	int	rsp_buflen ;
	
	int	nret = 0 ;
	
	InfoLog( __FILE__ , __LINE__ , "req xml[%.*s]" , post_data_len , post_data );
	
	memset( output_buffer , 0x00 , sizeof(output_buffer) );
	output_buflen = 0 ;
	
	if( strcmp( p_req->Event , "<![CDATA[subscribe]]>" ) == 0 )
	{
		if( penv->funcs.pfuncReceiveSubscribeEventProc )
		{
			nret = penv->funcs.pfuncReceiveSubscribeEventProc( output_buffer , & output_buflen , sizeof(output_buffer) ) ;
			if( nret )
			{
				ErrorLog( __FILE__ , __LINE__ , "pfuncReceiveSubscribeEventProc failed , errno[%d]" , errno );
			}
		}
	}
	else if( strcmp( p_req->Event , "<![CDATA[unsubscribe]]>" ) == 0 )
	{
		if( penv->funcs.pfuncReceiveUnsubscribeEventProc )
		{
			nret = penv->funcs.pfuncReceiveUnsubscribeEventProc( output_buffer , & output_buflen , sizeof(output_buffer) ) ;
			if( nret )
			{
				ErrorLog( __FILE__ , __LINE__ , "pfuncReceiveUnsubscribeEventProc failed , errno[%d]" , errno );
			}
		}
	}
	else
	{
		output_buflen += snprintf( output_buffer+output_buflen , sizeof(output_buffer)-1 - output_buflen , "未知的事件类型[%s]" , p_req->Event );
	}
	
	memset( & rsp , 0x00 , sizeof(xml) );
	strcpy( rsp.ToUserName , p_req->FromUserName );
	strcpy( rsp.FromUserName , p_req->ToUserName );
	snprintf( rsp.CreateTime , sizeof(rsp.CreateTime) , "%d" , (int)time(NULL) );
	strcpy( rsp.MsgType , "<![CDATA[text]]>" );
	snprintf( rsp.Content , sizeof(rsp.Content)-1 , "<![CDATA[%s]]>" , output_buffer );
	
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

