#include "weixin4c.h"

int cgiinit( struct Environment *penv )
{
	int		nret = 0 ;
	
	chdir( "/tmp" );
	
	SetLogFile( "%s/log/weixin4c.log" , getenv("HOME") );
	SetLogLevel( LOGLEVEL_DEBUG );
	
	penv->so_handler = _dlopen() ;
	if( penv->so_handler == NULL )
	{
		ErrorLog( __FILE__ , __LINE__ , "dlopen failed , dlerror[%s]" , dlerror() );
		return -1;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "dlopen ok" );
	}
	
	penv->funcs.pfuncInitEnvProc = dlsym( penv->so_handler , "InitEnvProc" ) ;
	penv->funcs.pfuncCleanEnvProc = dlsym( penv->so_handler , "CleanEnvProc" ) ;
	
	penv->funcs.pfuncReceiveSubscribeEventProc = dlsym( penv->so_handler , "ReceiveSubscribeEventProc" ) ;
	penv->funcs.pfuncReceiveUnsubscribeEventProc = dlsym( penv->so_handler , "ReceiveUnsubscribeEventProc" ) ;
	penv->funcs.pfuncReceiveTextMsgProc = dlsym( penv->so_handler , "ReceiveTextMsgProc" ) ;
	
	if( penv->funcs.pfuncInitEnvProc )
	{
		nret = penv->funcs.pfuncInitEnvProc() ;
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "pfuncInitEnvProc failed[%d]" , nret );
			dlclose( penv->so_handler );
			return -2;
		}
		else
		{
			InfoLog( __FILE__ , __LINE__ , "pfuncInitEnvProc ok" );
		}
	}
	
	return 0;
}

int cgimain( struct Environment *penv )
{
	char		*signature = NULL ;
	char		*timestamp = NULL ;
	char		*nonce = NULL ;
	char		*echostr = NULL ;
	
	char		*post_data = NULL ;
	int		post_data_len ;
	xml		req ;
	
	int		nret = 0 ;
	
	PUBSendContentTypeHtml();
	
	PUBDupUrlParam( "signature" , & signature );
	PUBDupUrlParam( "timestamp" , & timestamp );
	PUBDupUrlParam( "nonce" , & nonce );
	PUBDupUrlParam( "echostr" , & echostr );
	
	if( signature && timestamp && nonce && echostr )
	{
		SetLogFile( "%s/log/VerifyServer.log" , getenv("HOME") );
		nret = VerifyServer( penv , signature , timestamp , nonce , echostr ) ;
		SetLogFile( "%s/log/weixin4c.log" , getenv("HOME") );
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
		}
		else
		{
			InfoLog( __FILE__ , __LINE__ , "PUBReadPostBuffer ok" );
			
			post_data = PUBGetPostBufferPtr() ;
			post_data_len = PUBGetPostBufferLength() ;
			
			memset( & req , 0x00 , sizeof(xml) );
			nret = DSCDESERIALIZE_XML_xml( NULL , post_data , & post_data_len , & req ) ;
			if( nret )
			{
				ErrorLog( __FILE__ , __LINE__ , "DSCDESERIALIZE_XML_xml_MsgType failed[%d] , xml[%s]" , nret , post_data );
			}
			else
			{
				InfoLog( __FILE__ , __LINE__ , "DSCDESERIALIZE_XML_xml_MsgType ok , xml[%s]" , post_data );
				
				InfoLog( __FILE__ , __LINE__ , "req.MsgType[%s]" , req.MsgType );
				if( strcmp( req.MsgType , "<![CDATA[event]]>" ) == 0 )
				{
					SetLogFile( "%s/log/ReceiveEvent.log" , getenv("HOME") );
					nret = ReceiveEvent( penv , post_data , post_data_len , & req ) ;
					SetLogFile( "%s/log/weixin4c.log" , getenv("HOME") );
					if( nret )
					{
						ErrorLog( __FILE__ , __LINE__ , "ReceiveEvent failed[%d]" , nret );
					}
					else
					{
						InfoLog( __FILE__ , __LINE__ , "ReceiveEvent ok" );
					}
				}
				else if( strcmp( req.MsgType , "<![CDATA[text]]>" ) == 0 )
				{
					SetLogFile( "%s/log/ReceiveText.log" , getenv("HOME") );
					nret = ReceiveText( penv , post_data , post_data_len , & req ) ;
					SetLogFile( "%s/log/weixin4c.log" , getenv("HOME") );
					if( nret )
					{
						ErrorLog( __FILE__ , __LINE__ , "ReceiveText failed[%d]" , nret );
					}
					else
					{
						InfoLog( __FILE__ , __LINE__ , "ReceiveText ok" );
					}
				}
				else
				{
					ErrorLog( __FILE__ , __LINE__ , "xml.MsgType[%s] invalid" , req.MsgType );
				}
			}
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

int cgiclean( struct Environment *penv )
{
	int		nret = 0 ;
	
	if( penv->funcs.pfuncCleanEnvProc )
	{
		nret = penv->funcs.pfuncCleanEnvProc() ;
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "pfuncCleanEnvProc failed[%d]" , nret );
		}
		else
		{
			InfoLog( __FILE__ , __LINE__ , "pfuncCleanEnvProc ok" );
		}
	}
	
	if( penv->so_handler )
	{
		InfoLog( __FILE__ , __LINE__ , "dlclose ok" );
		dlclose( penv->so_handler );
		penv->so_handler = NULL ;
	}
	
	return 0;
}

