#include "public.h"

#include "curl/curl.h"

#include "IDL_access_token.dsc.h"

struct StringBuffer
{
	int		buf_size ;
	int		str_len ;
	char		*base ;
} ;

static size_t CurlProc( char *buffer , size_t size , size_t nmemb , void *p )
{
	struct StringBuffer	*buf = (struct StringBuffer *)p ;
	
	if( (int)(size*nmemb) > buf->buf_size-1 - buf->str_len )
	{
		char	*tmp = NULL ;
		int	new_buf_size = buf->buf_size + 4096 ;
		tmp = realloc( buf->base , new_buf_size ) ;
		if( tmp == NULL )
		{
			ErrorLog( __FILE__ , __LINE__ , "realloc failed , errno[%d]" , errno );
			return -1;
		}
		buf->base = tmp ;
		buf->buf_size = new_buf_size ;
		memset( buf->base + buf->str_len , 0x00 , buf->buf_size-1 - buf->str_len );
	}
	
	memcpy( buf->base + buf->str_len , buffer , size*nmemb );
	buf->str_len += size*nmemb ;
	return size*nmemb;
}

static int AccessToken( int loop_flag )
{
	char			*AppID = NULL ;
	char			*AppSecret = NULL ;
	
	CURL			*curl = "" ;
	CURLcode		res ;
	char			url[ 1024 + 1 ] ;
	struct StringBuffer	buf ;
	access_token		at ;
	
	int			nret = 0 ;
	
	curl_global_init( CURL_GLOBAL_ALL );
	
	memset( & buf , 0x00 , sizeof(struct StringBuffer) );
	
	memset( & at , 0x00 , sizeof(access_token) );
	
	do
	{
		InfoLog( __FILE__ , __LINE__ , "sleep [%d]seconds ..." , at.expires_in / 2 );
		sleep( at.expires_in / 2 );
		InfoLog( __FILE__ , __LINE__ , "sleep [%d]seconds done" , at.expires_in / 2 );
		
		PUBReadEntireFileSafely( ETC_PATHFILENAME_AppID , "r" , & AppID , NULL );
		PUBStringNoEnter( AppID );
		InfoLog( __FILE__ , __LINE__ , "AppID[%s]" , AppID );
		
		PUBReadEntireFileSafely( ETC_PATHFILENAME_AppSecret , "r" , & AppSecret , NULL);
		PUBStringNoEnter( AppSecret );
		InfoLog( __FILE__ , __LINE__ , "AppSecret[%s]" , AppSecret );
		
		curl = curl_easy_init() ;
		if( curl == NULL )
		{
			ErrorLog( __FILE__ , __LINE__ , "curl_easy_init failed" );
			break;
		}
		
		memset( url , 0x00 , sizeof(url) );
		snprintf( url , sizeof(url)-1 , "https://api.weixin.qq.com/cgi-bin/token?grant_type=client_credential&appid=%s&secret=%s" , AppID , AppSecret );
		curl_easy_setopt( curl , CURLOPT_URL , url );
		
		curl_easy_setopt( curl , CURLOPT_WRITEFUNCTION , & CurlProc );
		if( buf.buf_size )
		{
			memset( buf.base , 0x00 , buf.buf_size );
			buf.str_len = 0 ;
		}
		curl_easy_setopt( curl , CURLOPT_WRITEDATA , (void*) & buf );
		
		InfoLog( __FILE__ , __LINE__ , "curl_easy_perform[%s] ..." , url );
		res = curl_easy_perform( curl );
		InfoLog( __FILE__ , __LINE__ , "curl_easy_perform[%s] return[%d]" , url , res );
		curl_easy_cleanup( curl );
		if( res == CURLE_OK )
		{
			memset( & at , 0x00 , sizeof(access_token) );
			nret = DSCDESERIALIZE_JSON_access_token( NULL , buf.base , & (buf.str_len) , & at ) ;
			if( nret )
			{
				ErrorLog( __FILE__ , __LINE__ , "DSCDESERIALIZE_JSON_access_token failed[%d] , json[%.*s]" , nret , buf.str_len , buf.base );
				break;
			}
			else
			{
				InfoLog( __FILE__ , __LINE__ , "DSCDESERIALIZE_JSON_access_token ok , json[%.*s]" , buf.str_len , buf.base );
			}
			
			PUBWriteEntireFile( ETC_PATHFILENAME_AccessToken , "w" , at.access_token , -1 );
			InfoLog( __FILE__ , __LINE__ , "write[%s] to file[%s]" , at.access_token , ETC_PATHFILENAME_AccessToken );
		}
		
		free( AppID ); AppID = NULL ;
		free( AppSecret ); AppSecret = NULL ;
	}
	while( loop_flag );
	
	curl_global_cleanup();
	
	if( AppID )
	{
		free( AppID ); AppID = NULL ;
	}
	if( AppSecret )
	{
		free( AppSecret );  AppSecret = NULL ;
	}
	
	if( buf.base )
	{
		free( buf.base ); buf.base = NULL ;
		buf.buf_size = 0 ;
		buf.str_len = 0 ;
	}
	
	return 0;
}

static void usage()
{
	printf( "USAGE : access_token [ 0 | 1 ]\n" );
	printf( "                    0 : fetch once\n" );
	printf( "                    1 : fetch for-ever\n" );
	return;
}

int main( int argc , char *argv[] )
{
	SetLogFile( HOME"/log/access_token.log" );
	SetLogLevel( LOGLEVEL_DEBUG );
	
	if( argc == 1 + 1 )
	{
		return -AccessToken( atoi(argv[1]) );
	}
	else
	{
		usage();
		exit(9);
	}
}

