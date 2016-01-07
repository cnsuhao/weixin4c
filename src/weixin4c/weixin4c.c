#include "weixin4c_public.h"
#include "weixin4c_private.h"

static int cgiinit( struct Weixin4cEnv *penv )
{
	int		nret = 0 ;
	
	chdir( "/tmp" );
	
	SetLogFile( "%s/log/weixin4c.log" , penv->pconf->home );
	SetLogLevel( LOGLEVEL_DEBUG );
	
	if( penv->pconf->funcs.pfuncInitEnvProc )
	{
		nret = penv->pconf->funcs.pfuncInitEnvProc() ;
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "pfuncInitEnvProc failed[%d]" , nret );
			return -2;
		}
		else
		{
			InfoLog( __FILE__ , __LINE__ , "pfuncInitEnvProc ok" );
		}
	}
	
	return 0;
}

static int cgimain( struct Weixin4cEnv *penv )
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
		SetLogFile( "%s/log/verify_server.log" , penv->pconf->home );
		nret = VerifyServer( penv , signature , timestamp , nonce , echostr ) ;
		SetLogFile( "%s/log/weixin4c.log" , penv->pconf->home );
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
					SetLogFile( "%s/log/receive_event.log" , penv->pconf->home );
					nret = ReceiveEvent( penv , post_data , post_data_len , & req ) ;
					SetLogFile( "%s/log/weixin4c.log" , penv->pconf->home );
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
					SetLogFile( "%s/log/receive_text.log" , penv->pconf->home );
					nret = ReceiveText( penv , post_data , post_data_len , & req ) ;
					SetLogFile( "%s/log/weixin4c.log" , penv->pconf->home );
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

static int cgiclean( struct Weixin4cEnv *penv )
{
	int		nret = 0 ;
	
	if( penv->pconf->funcs.pfuncCleanEnvProc )
	{
		nret = penv->pconf->funcs.pfuncCleanEnvProc() ;
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "pfuncCleanEnvProc failed[%d]" , nret );
		}
		else
		{
			InfoLog( __FILE__ , __LINE__ , "pfuncCleanEnvProc ok" );
		}
	}
	
	return 0;
}

int weixin4c( struct Weixin4cConfig *pconf )
{
	struct Weixin4cEnv	env ;
	
	int			nret = 0 ;
	
	memset( & env , 0x00 , sizeof(struct Weixin4cEnv) );
	env.pconf = pconf ;
	
	nret = cgiinit( & env ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "cgiinit failed[%d]" , nret );
		return nret;
	}
	
	while( FCGI_Accept() >= 0 )
	{
		nret = cgimain( & env ) ;
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "cgimain failed[%d]" , nret );
			nret = cgiclean( & env ) ;
			return nret;
		}
break;
	}
	
	nret = cgiclean( & env ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "cgiclean failed[%d]" , nret );
		return nret;
	}
	
	return 0;
}

