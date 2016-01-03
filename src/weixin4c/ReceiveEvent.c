#include "public.h"
#include "private.h"

int ReceiveEvent( char *post_data , int post_data_len , xml *p_req )
{
	xml	rsp ;
	char	output_buffer[ 4096 * 90 ] ;
	int	n ;
	char	rsp_buffer[ 4096 * 100 ] ;
	int	rsp_buflen ;
	
	int	nret = 0 ;
	
	SetLogFile( HOME"/log/ReceiveEvent.log" );
	SetLogLevel( LOGLEVEL_DEBUG );
	
	InfoLog( __FILE__ , __LINE__ , "req xml[%.*s]" , post_data_len , post_data );
	
	if( strcmp( p_req->Event , "<![CDATA[subscribe]]>" ) == 0 )
	{
		snprintf( output_buffer , sizeof(output_buffer)-1 ,
			"��ӭ����\n"
			"\"������\"�ṩ�ڶ๫����Ϣʵʱ��ѯ\n"
			"Ŀǰ���ṩ:\n"
			"    .com����ʵʱ��ѯע����Ϣ���緢��\"ym google.com\"����\n"
			"   �����๦�ܿ�����...��\n"
			"��������뷢��\"?\"��á�"
			);
	}
	else if( strcmp( p_req->Event , "<![CDATA[unsubscribe]]>" ) == 0 )
	{
	}
	else
	{
		snprintf( output_buffer , sizeof(output_buffer)-1 , "δ֪���¼�����[%s]" , p_req->Event );
	}
	
	InfoLog( __FILE__ , __LINE__ , "�ı�����ǰ[%s]" , output_buffer );
	n = PUBConvCharacterCode( "GB18030" , "UTF-8" , output_buffer , -1 , sizeof(output_buffer) );
	if( n < 0 )
	{
		ErrorLog( __FILE__ , __LINE__ , "PUBConvCharacterCode failed[%d]" , n );
		return -1;
	}
	InfoLog( __FILE__ , __LINE__ , "�ı������[%s]" , output_buffer );
	
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

