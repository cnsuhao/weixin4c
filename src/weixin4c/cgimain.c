#include "public.h"
#include "private.h"

int cgiinit()
{
	chdir( "/tmp" );
	
	SetLogFile( "/home/calvin/log/VerifyServer.log" );
	SetLogLevel( LOGLEVEL_DEBUG );
	
	return 0;
}

int cgimain()
{
	char		*signature = NULL ;
	char		*timestamp = NULL ;
	char		*nonce = NULL ;
	char		*echostr = NULL ;
	
	char		*post_data = NULL ;
	int		post_data_len ;
	xml_MsgType	xm ;
	
	int		nret = 0 ;
	
	PUBSendContentTypeHtml();
	
	PUBDupUrlParam( "signature" , & signature );
	PUBDupUrlParam( "timestamp" , & timestamp );
	PUBDupUrlParam( "nonce" , & nonce );
	PUBDupUrlParam( "echostr" , & echostr );
	
	if( signature && timestamp && nonce && echostr )
	{
		nret = VerifyServer( signature , timestamp , nonce , echostr ) ;
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "VerifyServer failed[%d]" , nret );
		}
		else
		{
			InfoLog( __FILE__ , __LINE__ , "VerifyServer ok" );
		}
	}
	else
	{
		nret = PUBReadPostBuffer() ;
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "PUBReadPostBuffer failed[%d]" , nret );
			return nret;
		}
		else
		{
			InfoLog( __FILE__ , __LINE__ , "PUBReadPostBuffer ok" );
		}
		
		post_data = PUBGetPostBufferPtr() ;
		post_data_len = PUBGetPostBufferLength() ;
		
		memset( & xm , 0x00 , sizeof(xml_MsgType) );
		nret = DSCDESERIALIZE_XML_xml_MsgType( NULL , post_data , & post_data_len , & xm ) ;
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "DSCDESERIALIZE_XML_xml_MsgType failed[%d] , xml[%s]" , nret , post_data );
		}
		else
		{
			InfoLog( __FILE__ , __LINE__ , "DSCDESERIALIZE_XML_xml_MsgType ok , xml[%s]" , post_data );
		}
		
		
		
	}
	
	if( signature )
		free( signature );
	if( timestamp )
		free( timestamp );
	if( nonce )
		free( nonce );
	if( echostr )
		free( echostr );
	
	return nret;
}

int cgiclean()
{
	return 0;
}

