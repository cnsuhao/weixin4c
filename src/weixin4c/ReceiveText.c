#include "public.h"
#include "private.h"

int ReceiveText( char *post_data , int post_data_len , xml *p_req )
{
	xml	rsp ;
	int	n ;
	char	*p = NULL ;
	char	*command = NULL ;
	char	*params = NULL ;
	char	output_buffer[ 4096 * 90 ] ;
	int	output_buflen ;
	char	rsp_buffer[ 4096 * 100 ] ;
	int	rsp_buflen ;
	
	int	nret = 0 ;
	
	SetLogFile( HOME"/log/ReceiveText.log" );
	SetLogLevel( LOGLEVEL_DEBUG );
	
	InfoLog( __FILE__ , __LINE__ , "req xml[%.*s]" , post_data_len , post_data );
	
	InfoLog( __FILE__ , __LINE__ , "文本解码前[%s]" , p_req->Content );
	n = PUBConvCharacterCode( "UTF-8" , "GB18030" , p_req->Content , -1 , sizeof(p_req->Content) );
	if( n < 0 )
	{
		ErrorLog( __FILE__ , __LINE__ , "PUBConvCharacterCode failed[%d]" , n );
		return -1;
	}
	InfoLog( __FILE__ , __LINE__ , "文本解码后[%s]" , p_req->Content );
	
	if( memcmp( p_req->Content , "<![CDATA[" , 9 ) == 0 )
	{
		int	len ;
		len = strlen(p_req->Content) ;
		p_req->Content[len-3] = '\0' ;
		memmove( p_req->Content , p_req->Content+9 , len-3-9+1 );
	}
	
	p = strchr( p_req->Content , ' ' ) ;
	if( p == NULL )
	{
		command = p_req->Content ;
		params = "" ;
	}
	else
	{
		(*p) = '\0' ;
		command = p_req->Content ;
		params = p + 1 ;
	}
	
	if( strcmp( command , "?" ) == 0 || strcmp( command , "？" ) == 0 )
	{
		memset( output_buffer , 0x00 , sizeof(output_buffer) );
		output_buflen = snprintf( output_buffer , sizeof(output_buffer)-1 ,
			"\"钛搜索\"提供众多公共信息实时查询\n"
			"1.实时查询域名注册信息 :（目前支持.com .net .org .info .edu .biz .cn）\n"
			"  命令语法:\"[ym|yuming|域名] [域名]\"\n"
			"  示例:\"ym google.com\"\n"
			"  示例:\"yuming google.com\"\n"
			"  示例:\"域名 google.com\"\n"
			"  示例:\"ym google\"\n"
			"（更多生活类查询功能开发中...）\n"
			"具体帮助请发送\"?\"获得。"
			) ;
	}
	else if( strcmp( command , "ym" ) == 0 || strcmp( command , "yuming" ) == 0 || strcmp( command , "域名" ) == 0 )
	{
		memset( output_buffer , 0x00 , sizeof(output_buffer) );
		output_buflen = 0 ;
		nret = ReceiveText_QueryDomain( params , output_buffer , & output_buflen , sizeof(output_buffer) ) ;
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "QueryDomain failed[%d]" , nret );
		}
		else
		{
			InfoLog( __FILE__ , __LINE__ , "QueryDomain ok" );
		}
	}
	else
	{
		ErrorLog( __FILE__ , __LINE__ , "command[%s] invalid" , command );
		output_buflen = snprintf( output_buffer , sizeof(output_buffer)-1 , "未知命令[%s]" , command ) ;
	}
	
	InfoLog( __FILE__ , __LINE__ , "文本编码前[%s]" , output_buffer );
	n = PUBConvCharacterCode( "GB18030" , "UTF-8" , output_buffer , output_buflen , sizeof(output_buffer) );
	if( n < 0 )
	{
		ErrorLog( __FILE__ , __LINE__ , "PUBConvCharacterCode failed[%d]" , n );
		return -1;
	}
	InfoLog( __FILE__ , __LINE__ , "文本编码后[%s]" , output_buffer );
	
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

