#include "public.h"
#include "private.h"

char *strptime(const char *s, const char *format, struct tm *tm);

#define WHOIS_SERVER_TIMEOUT	20

struct RootNameInfo
{
	char	*root_name ;
	char	*ip ;
	int	port ;
	char	*encoding ;
	char	*date_format ;
	struct
	{
		char	*REGISTRAR ;
		char	*EMAIL ;
		char	*SPONSORING_REGISTRAR ;
		char	*SPONSORING_REGISTRAR_IANA_ID ;
		char	*WHOIS_SERVER ;
		char	*REFERRAL_URL ;
		char	*CREATION_DATE ;
		char	*UPDATED_DATE ;
		char	*EXPIRATION_DATE ;
	} matching_text ;
} g_rootname_info [] =
	{
		{ ".com" , "199.7.57.74" , 43 , NULL , "%d-%b-%Y" , {
			"Registrar:" ,
			NULL ,
			NULL ,
			"Sponsoring Registrar IANA ID:" ,
			"Whois Server:" ,
			"Referral URL:" ,
			"Creation Date:" ,
			"Updated Date:" ,
			"Expiration Date:"
		} } ,
		{ ".net" , "199.7.57.74" , 43 , NULL , "%d-%b-%Y" , {
			"Registrar:" ,
			NULL ,
			NULL ,
			"Sponsoring Registrar IANA ID:" ,
			"Whois Server:" ,
			"Referral URL:" ,
			"Creation Date:" ,
			"Updated Date:" ,
			"Expiration Date:"
		} } ,
		{ ".org" , "199.15.84.131" , 43 , NULL , NULL , {
			"Registrant Name:" ,
			"Registrant Email:" ,
			"Sponsoring Registrar:" ,
			"Sponsoring Registrar IANA ID:" ,
			"WHOIS Server:" ,
			"Referral URL:" ,
			"Creation Date:" ,
			"Updated Date:" ,
			"Registry Expiry Date:"
		} } ,
		{ ".info" , "199.15.85.130" , 43 , NULL , NULL , {
			"Registrant Name:" ,
			"Registrant Email:" ,
			"Sponsoring Registrar:" ,
			"Sponsoring Registrar IANA ID:" ,
			"WHOIS Server:" ,
			"Referral URL:" ,
			"Creation Date:" ,
			"Updated Date:" ,
			"Registry Expiry Date:"
		} } ,
		{ ".edu" , "199.7.57.74" , 43 , NULL , "%d-%b-%Y" , {
			"Registrar:" ,
			NULL ,
			NULL ,
			"Sponsoring Registrar IANA ID:" ,
			"Whois Server:" ,
			"Referral URL:" ,
			"Creation Date:" ,
			"Updated Date:" ,
			"Expiration Date:"
		} } ,
		{ ".biz" , "209.173.53.169" , 43 , NULL , "%a %b %d %T GMT %Y" , {
			"Registrant Name:" ,
			"Registrant Email:" ,
			"Sponsoring Registrar:" ,
			"Sponsoring Registrar IANA ID:" ,
			NULL ,
			"Registrar URL (registration services):" ,
			"Domain Registration Date:" ,
			"Domain Last Updated Date:" ,
			"Domain Expiration Date:"
		} } ,
		{ ".cn" , "218.241.97.14" , 43 , "UTF-8" , NULL , {
			"Registrant:" ,
			"Registrant Contact Email:" ,
			"Sponsoring Registrar:" ,
			NULL ,
			NULL ,
			NULL ,
			"Registration Time:" ,
			NULL ,
			"Expiration Time:"
		} } ,
		/*
		{ ".info" , "199.7.71.74" } ,
		{ ".name" , "199.7.71.74" } ,
		{ ".edu.cn" , "202.112.0.47" } ,
		*/
		/* for template
		{ ".com" , "199.7.59.74" , 43 , NULL , "%d-%b-%Y" , {
			"Registrar:" ,
			"Registrant Contact Email:" ,
			"Sponsoring Registrar:" ,
			"Sponsoring Registrar IANA ID:" ,
			"Whois Server:" ,
			"Referral URL:" ,
			"Creation Date:" ,
			"Updated Date:" ,
			"Expiration Date:"
		} } ,
 		*/
		{ NULL }
	} ;

static int _ReceiveText_QueryDomain( char *domain_name , struct RootNameInfo *prninfo , char *output_buffer , int *p_output_buflen , int output_bufsize )
{
	int			sock ;
	socklen_t		socklen ;
	struct sockaddr_in	sockaddr ;
	char			send_buffer[ 1024 + 1 ] ;
	int			len , offset_len ;
	int			send_len ;
	char			recv_buffer[ 4096 * 10 + 1 ] ;
	int			recv_len ;
	char			*p = NULL ;
	fd_set			read_fds ;
	struct timeval		tv ;
	time_t			timeout , timeout_bak ;
	time_t			tt1 , tt2 , dtt ;
	
	int			nret = 0 ;
	
	timeout = WHOIS_SERVER_TIMEOUT ;
	
	sock = socket( AF_INET , SOCK_STREAM , IPPROTO_TCP );
	if( sock == -1 )
	{
		ErrorLog( __FILE__ , __LINE__ , "socket failed , errno[%d]" , errno );
		(*p_output_buflen) += snprintf( output_buffer+(*p_output_buflen) , output_bufsize-1 - (*p_output_buflen) , "创建socket句柄失败" );
		return -1;
	}
	
	time( & tt1 );
	
	memset( & sockaddr , 0x00 , sizeof(struct sockaddr_in) );
	sockaddr.sin_family = AF_INET ;
	sockaddr.sin_addr.s_addr = inet_addr( prninfo->ip ) ;
	sockaddr.sin_port = htons( prninfo->port );
	socklen = sizeof(struct sockaddr_in) ;
	InfoLog( __FILE__ , __LINE__ , "connect[%s:%d] ..." , prninfo->ip , prninfo->port );
	nret = connect( sock , (struct sockaddr *) & sockaddr , socklen );
	if( nret == -1 )
	{
		ErrorLog( __FILE__ , __LINE__ , "connect[%s:%d] failed , errno[%d]" , prninfo->ip , prninfo->port , errno );
		(*p_output_buflen) += snprintf( output_buffer+(*p_output_buflen) , output_bufsize-1 - (*p_output_buflen) , "连接到WHOIS服务器失败" );
		close( sock );
		return -1;
	}
	
	time( & tt2 );
	dtt = tt2 - tt1 ;
	timeout -= dtt ;
	if( timeout <= 0 )
	{
		ErrorLog( __FILE__ , __LINE__ , "connect[%s:%d] timeout" , prninfo->ip , prninfo->port );
		(*p_output_buflen) += snprintf( output_buffer+(*p_output_buflen) , output_bufsize-1 - (*p_output_buflen) , "连接到WHOIS服务器超时" );
		close( sock );
		return -1;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "connect[%s:%d] ok , elapse [%d]seconds" , prninfo->ip , prninfo->port , dtt );
	}
	
	time( & tt1 );
	
	memset( send_buffer , 0x00 , sizeof(send_buffer) );
	send_len = snprintf( send_buffer , sizeof(send_buffer)-1 , "domain %s\r\n" , domain_name ) ;
	len = 0 ;
	offset_len = 0 ;
	InfoLog( __FILE__ , __LINE__ , "send[%d][%.*s] ..." , send_len , send_len , send_buffer );
	while( send_len > 0 )
	{
		len = send( sock , send_buffer + offset_len , send_len , 0 ) ;
		if( len == -1 )
		{
			ErrorLog( __FILE__ , __LINE__ , "send[%s:%d] failed , errno[%d]" , prninfo->ip , prninfo->port , errno );
			(*p_output_buflen) += snprintf( output_buffer+(*p_output_buflen) , output_bufsize-1 - (*p_output_buflen) , "发送查询命令WHOIS服务器失败" );
			close( sock );
			return -1;
		}
		
		offset_len += len ;
		send_len -= len ;
	}
	if( len == -1 )
		return -1;
	
	time( & tt2 );
	dtt = tt2 - tt1 ;
	timeout -= dtt ;
	if( timeout <= 0 )
	{
		ErrorLog( __FILE__ , __LINE__ , "send[%s:%d] timeout" , prninfo->ip , prninfo->port );
		(*p_output_buflen) += snprintf( output_buffer+(*p_output_buflen) , output_bufsize-1 - (*p_output_buflen) , "发送查询命令WHOIS服务器超时" );
		close( sock );
		return -1;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "send[%s:%d] ok , data[%d][%.*s] , elapse [%d]seconds" , prninfo->ip , prninfo->port , offset_len , offset_len-2 , send_buffer , dtt );
	}
	
	InfoLog( __FILE__ , __LINE__ , "recv ..." );
	memset( recv_buffer , 0x00 , sizeof(recv_buffer) );
	recv_len = sizeof(recv_buffer) - 1 ;
	len = 0 ;
	offset_len = 0 ;
	
	timeout_bak = timeout ;
	while(1)
	{
		time( & tt1 );
		
		FD_ZERO( & read_fds );
		FD_SET( sock , & read_fds );
		memset( & tv , 0x00 , sizeof(struct timeval) );
		tv.tv_sec = timeout ;
		tv.tv_usec = 0 ;
		nret = select( sock+1 , & read_fds , NULL , NULL , & tv ) ;
		if( nret == -1 )
		{
			ErrorLog( __FILE__ , __LINE__ , "select[%s:%d] failed , errno[%d]" , prninfo->ip , prninfo->port , errno );
			(*p_output_buflen) += snprintf( output_buffer+(*p_output_buflen) , output_bufsize-1 - (*p_output_buflen) , "等待WHOIS服务响应失败" );
			close( sock );
			len = -1 ;
			break;
		}
		else if( nret == 0 )
		{
			ErrorLog( __FILE__ , __LINE__ , "select[%s:%d] timeout , data[%d][%.*s]" , prninfo->ip , prninfo->port , offset_len , offset_len , recv_buffer );
			(*p_output_buflen) += snprintf( output_buffer+(*p_output_buflen) , output_bufsize-1 - (*p_output_buflen) , "等待WHOIS服务响应超时" );
			close( sock );
			len = -1 ;
			break;
		}
		
		len = recv( sock , recv_buffer + offset_len , recv_len , 0 ) ;
		if( len == 0 )
		{
			InfoLog( __FILE__ , __LINE__ , "recv[%s:%d] ok , data[%d][%.*s]" , prninfo->ip , prninfo->port , len , len , recv_buffer + offset_len );
			break;
		}
		else if( len == -1 )
		{
			ErrorLog( __FILE__ , __LINE__ , "recv[%s:%d] failed , errno[%d]" , prninfo->ip , prninfo->port , errno );
			(*p_output_buflen) += snprintf( output_buffer+(*p_output_buflen) , output_bufsize-1 - (*p_output_buflen) , "接收WHOIS服务响应失败" );
			close( sock );
			break;
		}
		else
		{
			InfoLog( __FILE__ , __LINE__ , "recv[%s:%d] ok , data[%d][%.*s]" , prninfo->ip , prninfo->port , len , len , recv_buffer + offset_len );
		}
		
		offset_len += len ;
		recv_len -= len ;
		
		time( & tt2 );
		dtt = tt2 - tt1 ;
		timeout -= dtt ;
		if( timeout <= 0 )
		{
			ErrorLog( __FILE__ , __LINE__ , "recv[%s:%d] timeout" , prninfo->ip , prninfo->port );
			(*p_output_buflen) += snprintf( output_buffer+(*p_output_buflen) , output_bufsize-1 - (*p_output_buflen) , "接收WHOIS服务响应超时" );
			close( sock );
			len = -1 ;
			break;
		}
	}
	if( len == -1 )
		return -1;
	close( sock );
	InfoLog( __FILE__ , __LINE__ , "recv[%s:%d] ok , elapse [%d]seconds" , prninfo->ip , prninfo->port , timeout_bak - timeout );
	if( offset_len == 0 )
		return -1;
	
	InfoLog( __FILE__ , __LINE__ , "COMM %s ELAPSE %d SECONDS" , prninfo->ip , WHOIS_SERVER_TIMEOUT - timeout );
	
	if( strstr( recv_buffer , "No match for domain" ) || strstr( recv_buffer , "Not found" ) || strstr( recv_buffer , "NOT FOUND" ) )
	{
		(*p_output_buflen) += snprintf( output_buffer+(*p_output_buflen) , output_bufsize-1 - (*p_output_buflen) , "域名%s还未注册" , domain_name );
	}
	else
	{
		struct tm	stime ;
		char		*registrar_begin = NULL ;
		char		*registrar_end = NULL ;
		char		registrar[ 256 + 1 ] ;
		char		*email_begin = NULL ;
		char		*email_end = NULL ;
		char		email[ 256 + 1 ] ;
		char		*sponsoring_registrar_begin = NULL ;
		char		*sponsoring_registrar_end = NULL ;
		char		sponsoring_registrar[ 256 + 1 ] ;
		char		*sponsoring_registrar_iana_id_begin = NULL ;
		char		*sponsoring_registrar_iana_id_end = NULL ;
		char		sponsoring_registrar_iana_id[ 20 + 1 ] ;
		char		*whois_server_begin = NULL ;
		char		*whois_server_end = NULL ;
		char		whois_server[ 1024 + 1 ] ;
		char		*referral_url_begin = NULL ;
		char		*referral_url_end = NULL ;
		char		referral_url[ 1024 + 1 ] ;
		char		creation_date[ 10 + 1 ] ;
		char		updated_date[ 10 + 1 ] ;
		char		expiration_date[ 10 + 1 ] ;
		
		memset( registrar , 0x00 , sizeof(registrar) );
		if( prninfo->matching_text.REGISTRAR )
		{
			len = strlen(prninfo->matching_text.REGISTRAR) ;
			registrar_begin  = strstr( recv_buffer , prninfo->matching_text.REGISTRAR ) ;
			if( registrar_begin )
			{
				for( registrar_begin += len ; (*registrar_begin) && strchr( " \t\r\n" , (*registrar_begin) ) ; registrar_begin++ );
				
				registrar_end = strpbrk( registrar_begin , "\r\n" ) ;
				if( registrar_end == NULL )
				{
					ErrorLog( __FILE__ , __LINE__ , "[\\n] not found" );
					(*p_output_buflen) += snprintf( output_buffer+(*p_output_buflen) , output_bufsize-1 - (*p_output_buflen) , "WHOIS服务响应无效，不完整的[%s]" , prninfo->matching_text.REGISTRAR ) ;
					return -1;
				}
				
				strncpy( registrar , registrar_begin , MIN(registrar_end-registrar_begin,sizeof(registrar)-1) );
				
				if( prninfo->encoding )
				{
					PUBConvCharacterCode( prninfo->encoding , "GB18030" , registrar , -1 , sizeof(registrar) );
				}
			}
		}
		
		memset( email , 0x00 , sizeof(email) );
		if( prninfo->matching_text.EMAIL )
		{
			len = strlen(prninfo->matching_text.EMAIL) ;
			email_begin  = strstr( recv_buffer , prninfo->matching_text.EMAIL ) ;
			if( email_begin )
			{
				for( email_begin += len ; (*email_begin) && strchr( " \t\r\n" , (*email_begin) ) ; email_begin++ );
				
				email_end = strpbrk( email_begin , "\r\n" ) ;
				if( email_end == NULL )
				{
					ErrorLog( __FILE__ , __LINE__ , "[\\n] not found" );
					(*p_output_buflen) += snprintf( output_buffer+(*p_output_buflen) , output_bufsize-1 - (*p_output_buflen) , "WHOIS服务响应无效，不完整的[%s]" , prninfo->matching_text.EMAIL ) ;
					return -1;
				}
				
				strncpy( email , email_begin , MIN(email_end-email_begin,sizeof(email)-1) );
				
				if( prninfo->encoding )
				{
					PUBConvCharacterCode( prninfo->encoding , "GB18030" , email , -1 , sizeof(email) );
				}
			}
		}
		
		memset( sponsoring_registrar , 0x00 , sizeof(sponsoring_registrar) );
		if( prninfo->matching_text.SPONSORING_REGISTRAR )
		{
			len = strlen(prninfo->matching_text.SPONSORING_REGISTRAR) ;
			sponsoring_registrar_begin  = strstr( recv_buffer , prninfo->matching_text.SPONSORING_REGISTRAR ) ;
			if( sponsoring_registrar_begin )
			{
				for( sponsoring_registrar_begin += len ; (*sponsoring_registrar_begin) && strchr( " \t\r\n" , (*sponsoring_registrar_begin) ) ; sponsoring_registrar_begin++ );
				
				sponsoring_registrar_end = strpbrk( sponsoring_registrar_begin , "\r\n" ) ;
				if( sponsoring_registrar_end == NULL )
				{
					ErrorLog( __FILE__ , __LINE__ , "[\\n] not found" );
					(*p_output_buflen) += snprintf( output_buffer+(*p_output_buflen) , output_bufsize-1 - (*p_output_buflen) , "WHOIS服务响应无效，不完整的[%s]" , prninfo->matching_text.SPONSORING_REGISTRAR ) ;
					return -1;
				}
				
				strncpy( sponsoring_registrar , sponsoring_registrar_begin , MIN(sponsoring_registrar_end-sponsoring_registrar_begin,sizeof(sponsoring_registrar)-1) );
				
				if( prninfo->encoding )
				{
					PUBConvCharacterCode( prninfo->encoding , "GB18030" , sponsoring_registrar , -1 , sizeof(sponsoring_registrar) );
				}
			}
		}
		
		memset( sponsoring_registrar_iana_id , 0x00 , sizeof(sponsoring_registrar_iana_id) );
		if( prninfo->matching_text.SPONSORING_REGISTRAR_IANA_ID )
		{
			len = strlen(prninfo->matching_text.SPONSORING_REGISTRAR_IANA_ID) ;
			sponsoring_registrar_iana_id_begin  = strstr( recv_buffer , prninfo->matching_text.SPONSORING_REGISTRAR_IANA_ID ) ;
			if( sponsoring_registrar_iana_id_begin )
			{
				for( sponsoring_registrar_iana_id_begin += len ; (*sponsoring_registrar_iana_id_begin) && strchr( " \t\r\n" , (*sponsoring_registrar_iana_id_begin) ) ; sponsoring_registrar_iana_id_begin++ );
				
				sponsoring_registrar_iana_id_end = strpbrk( sponsoring_registrar_iana_id_begin , "\r\n" ) ;
				if( sponsoring_registrar_iana_id_end == NULL )
				{
					ErrorLog( __FILE__ , __LINE__ , "[\\n] not found" );
					(*p_output_buflen) += snprintf( output_buffer+(*p_output_buflen) , output_bufsize-1 - (*p_output_buflen) , "WHOIS服务响应无效，不完整的[%s]" , prninfo->matching_text.SPONSORING_REGISTRAR_IANA_ID ) ;
					return -1;
				}
				
				strncpy( sponsoring_registrar_iana_id , sponsoring_registrar_iana_id_begin , MIN(sponsoring_registrar_iana_id_end-sponsoring_registrar_iana_id_begin,sizeof(sponsoring_registrar_iana_id)-1) );
				
				if( prninfo->encoding )
				{
					PUBConvCharacterCode( prninfo->encoding , "GB18030" , sponsoring_registrar_iana_id , -1 , sizeof(sponsoring_registrar_iana_id) );
				}
			}
		}
		
		memset( whois_server , 0x00 , sizeof(whois_server) );
		if( prninfo->matching_text.WHOIS_SERVER )
		{
			len = strlen(prninfo->matching_text.WHOIS_SERVER) ;
			whois_server_begin  = strstr( recv_buffer , prninfo->matching_text.WHOIS_SERVER ) ;
			if( whois_server_begin )
			{
				for( whois_server_begin += len ; (*whois_server_begin) && strchr( " \t\r\n" , (*whois_server_begin) ) ; whois_server_begin++ );
				
				whois_server_end = strpbrk( whois_server_begin , "\r\n" ) ;
				if( whois_server_end == NULL )
				{
					ErrorLog( __FILE__ , __LINE__ , "[\\n] not found" );
					(*p_output_buflen) += snprintf( output_buffer+(*p_output_buflen) , output_bufsize-1 - (*p_output_buflen) , "WHOIS服务响应无效，不完整的[%s]" , prninfo->matching_text.WHOIS_SERVER ) ;
					return -1;
				}
				
				strncpy( whois_server , whois_server_begin , MIN(whois_server_end-whois_server_begin,sizeof(whois_server)-1) );
				
				if( prninfo->encoding )
				{
					PUBConvCharacterCode( prninfo->encoding , "GB18030" , whois_server , -1 , sizeof(whois_server) );
				}
			}
		}
		
		memset( referral_url , 0x00 , sizeof(referral_url) );
		if( prninfo->matching_text.REFERRAL_URL )
		{
			len = strlen(prninfo->matching_text.REFERRAL_URL) ;
			referral_url_begin  = strstr( recv_buffer , prninfo->matching_text.REFERRAL_URL ) ;
			if( referral_url_begin )
			{
				for( referral_url_begin += len ; (*referral_url_begin) && strchr( " \t\r\n" , (*referral_url_begin) ) ; referral_url_begin++ );
				
				referral_url_end = strpbrk( referral_url_begin , "\r\n" ) ;
				if( referral_url_end == NULL )
				{
					ErrorLog( __FILE__ , __LINE__ , "[\\n] not found" );
					(*p_output_buflen) += snprintf( output_buffer+(*p_output_buflen) , output_bufsize-1 - (*p_output_buflen) , "WHOIS服务响应无效，不完整的[%s]" , prninfo->matching_text.REFERRAL_URL ) ;
					return -1;
				}
				
				strncpy( referral_url , referral_url_begin , MIN(referral_url_end-referral_url_begin,sizeof(referral_url)-1) );
				
				if( prninfo->encoding )
				{
					PUBConvCharacterCode( prninfo->encoding , "GB18030" , referral_url , -1 , sizeof(referral_url) );
				}
			}
		}
		
		memset( creation_date , 0x00 , sizeof(creation_date) );
		if( prninfo->matching_text.CREATION_DATE )
		{
			len = strlen(prninfo->matching_text.CREATION_DATE) ;
			p = strstr( recv_buffer , prninfo->matching_text.CREATION_DATE ) ;
			if( p )
			{
				for( p += len ; (*p) && strchr( " \t\r\n" , (*p) ) ; p++ );
				
				if( prninfo->date_format == NULL )
				{
					strncpy( creation_date , p , sizeof(creation_date)-1 );
				}
				else
				{
					memset( & stime , 0x00 , sizeof(struct tm) );
					strptime( p , prninfo->date_format , & stime );
					strftime( creation_date , sizeof(creation_date) , "%Y-%m-%d" , & stime );
				}
			}
		}
		
		memset( updated_date , 0x00 , sizeof(updated_date) );
		if( prninfo->matching_text.UPDATED_DATE )
		{
			len = strlen(prninfo->matching_text.UPDATED_DATE) ;
			p = strstr( recv_buffer , prninfo->matching_text.UPDATED_DATE ) ;
			if( p )
			{
				for( p += len ; (*p) && strchr( " \t\r\n" , (*p) ) ; p++ );
				
				if( prninfo->date_format == NULL )
				{
					strncpy( updated_date , p , sizeof(updated_date)-1 );
				}
				else
				{
					memset( & stime , 0x00 , sizeof(struct tm) );
					strptime( p , prninfo->date_format , & stime );
					strftime( updated_date , sizeof(updated_date) , "%Y-%m-%d" , & stime );
				}
			}
		}
		
		memset( expiration_date , 0x00 , sizeof(expiration_date) );
		if( prninfo->matching_text.EXPIRATION_DATE )
		{
			len = strlen(prninfo->matching_text.EXPIRATION_DATE) ;
			p = strstr( recv_buffer , prninfo->matching_text.EXPIRATION_DATE ) ;
			if( p )
			{
				for( p += len ; (*p) && strchr( " \t\r\n" , (*p) ) ; p++ );
				
				if( prninfo->date_format == NULL )
				{
					strncpy( expiration_date , p , sizeof(expiration_date)-1 );
				}
				else
				{
					memset( & stime , 0x00 , sizeof(struct tm) );
					strptime( p , prninfo->date_format , & stime );
					strftime( expiration_date , sizeof(expiration_date) , "%Y-%m-%d" , & stime );
				}
			}
		}
		
		if(	registrar[0] == '\0' && email[0] == '\0'
			&& sponsoring_registrar[0] == '\0' && sponsoring_registrar_iana_id[0] == '\0'
			&& whois_server[0] == '\0' && referral_url[0] == '\0'
			&& creation_date[0] == '\0' && updated_date[0] == '\0' && expiration_date[0] == '\0' )
		{
			(*p_output_buflen) += snprintf( output_buffer+(*p_output_buflen) , output_bufsize-1 - (*p_output_buflen) , "域名[%s]注册信息无效" , domain_name );
		}
		else
		{
			(*p_output_buflen) += snprintf( output_buffer+(*p_output_buflen) , output_bufsize-1 - (*p_output_buflen) ,
				"域名 : %s\n"
				"所有者 : %s\n"
				"所有者邮箱 : %s\n"
				"注册商 : %s\n"
				"注册商IANA ID : %s\n"
				"WHOIS服务器 : %s\n"
				"关联URL : %s\n"
				"注册日期 : %s\n"
				"更新日期 : %s\n"
				"过期日期 : %s"
				, domain_name
				, registrar , email
				, sponsoring_registrar , sponsoring_registrar_iana_id
				, whois_server , referral_url
				, creation_date , updated_date , expiration_date
				) ;
		}
	}
	
	return 0;
}

static struct RootNameInfo *GetRootnameInfoPtr( char *root_name )
{
	struct RootNameInfo *p = NULL ;
	
	for( p = & (g_rootname_info[0]) ; p->root_name ; p++ )
	{
		if( strcmp( p->root_name , root_name ) == 0 )
			return p;
	}
	
	return NULL;
}

int ReceiveText_QueryDomain( char *params , char *output_buffer , int *p_output_buflen , int output_bufsize )
{
	int		len ;
	char		*p = NULL ;
	char		*domain_name = params ;
	char		*root_name = NULL ;
	
	int		nret = 0 ;
	
	len = strlen("www.") ;
	p = strrchr( domain_name , '.' ) ;
	if( strncmp( domain_name , "www." , len ) == 0 && ( p && p - domain_name > len ) )
	{
		memmove( domain_name , domain_name+len , strlen(domain_name+len)+1 );
	}
	
	root_name = strrchr( domain_name , '.' ) ;
	if( root_name == NULL )
	{
		struct RootNameInfo	*prninfo = NULL ;
		
		char			domain_name2[ 1024 + 1 ] ;
		
		for( prninfo = & (g_rootname_info[0]) ; prninfo->root_name ; prninfo++ )
		{
			if( prninfo != & (g_rootname_info[0]) )
			{
				(*p_output_buflen) += snprintf( output_buffer + (*p_output_buflen) , output_bufsize-1 - (*p_output_buflen) , "\n\n" ) ;
			}
			
			memset( domain_name2 , 0x00 , sizeof(domain_name2) );
			snprintf( domain_name2 , sizeof(domain_name2)-1 , "%s%s" , domain_name , prninfo->root_name );
			nret = _ReceiveText_QueryDomain( domain_name2 , prninfo , output_buffer , p_output_buflen , output_bufsize ) ;
			if( nret )
				return nret;
		}
	}
	else
	{
		struct RootNameInfo	*prninfo = NULL ;
		
		prninfo = GetRootnameInfoPtr( root_name ) ;
		if( prninfo == NULL )
		{
			(*p_output_buflen) += snprintf( output_buffer + (*p_output_buflen) , output_bufsize-1 - (*p_output_buflen) , "暂时不支持该域名类型[%s]" , root_name ) ;
			return 0;
		}
		
		nret = _ReceiveText_QueryDomain( domain_name , prninfo , output_buffer , p_output_buflen , output_bufsize ) ;
		if( nret )
			return nret;
	}
	
	return 0;
}

