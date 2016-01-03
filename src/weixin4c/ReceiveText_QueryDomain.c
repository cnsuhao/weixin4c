#include "public.h"
#include "private.h"

char *strptime(const char *s, const char *format, struct tm *tm);

#define WHOIS_SERVER_TIMEOUT	20
#define WHOIS_SERVER_IP		"199.7.57.74"
#define WHOIS_SERVER_PORT	43

#define REGISTRAR			"Registrar: "
#define SPONSORING_REGISTRAR_IANA_ID	"Sponsoring Registrar IANA ID: "
#define WHOIS_SERVER			"Whois Server: "
#define REFERRAL_URL			"Referral URL: "
#define CREATION_DATE			"Creation Date: "
#define UPDATED_DATE			"Updated Date: "
#define EXPIRATION_DATE			"Expiration Date: "

int ReceiveText_QueryDomain( char *params , char *output_buffer , int output_bufsize )
{
	char			*domain_name = params ;
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
		snprintf( output_buffer , output_bufsize-1 , "创建socket句柄失败" );
		return -1;
	}
	
	time( & tt1 );
	
	memset( & sockaddr , 0x00 , sizeof(struct sockaddr_in) );
	sockaddr.sin_family = AF_INET ;
	sockaddr.sin_addr.s_addr = inet_addr( WHOIS_SERVER_IP ) ;
	sockaddr.sin_port = htons( WHOIS_SERVER_PORT );
	socklen = sizeof(struct sockaddr_in) ;
	InfoLog( __FILE__ , __LINE__ , "connect[%s:%d] ..." , WHOIS_SERVER_IP , WHOIS_SERVER_PORT );
	nret = connect( sock , (struct sockaddr *) & sockaddr , socklen );
	if( nret == -1 )
	{
		ErrorLog( __FILE__ , __LINE__ , "connect[%s:%d] failed , errno[%d]" , WHOIS_SERVER_IP , WHOIS_SERVER_PORT , errno );
		snprintf( output_buffer , output_bufsize-1 , "连接到WHOIS服务器失败" );
		close( sock );
		return -1;
	}
	
	time( & tt2 );
	dtt = tt2 - tt1 ;
	timeout -= dtt ;
	if( timeout <= 0 )
	{
		ErrorLog( __FILE__ , __LINE__ , "connect[%s:%d] timeout" , WHOIS_SERVER_IP , WHOIS_SERVER_PORT );
		snprintf( output_buffer , output_bufsize-1 , "连接到WHOIS服务器超时" );
		close( sock );
		return -1;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "connect[%s:%d] ok , elapse [%d]seconds" , WHOIS_SERVER_IP , WHOIS_SERVER_PORT , dtt );
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
			ErrorLog( __FILE__ , __LINE__ , "send[%s:%d] failed , errno[%d]" , WHOIS_SERVER_IP , WHOIS_SERVER_PORT , errno );
			snprintf( output_buffer , output_bufsize-1 , "发送查询命令WHOIS服务器失败" );
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
		ErrorLog( __FILE__ , __LINE__ , "send[%s:%d] timeout" , WHOIS_SERVER_IP , WHOIS_SERVER_PORT );
		snprintf( output_buffer , output_bufsize-1 , "发送查询命令WHOIS服务器超时" );
		close( sock );
		return -1;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "send[%s:%d] ok , data[%d][%.*s] , elapse [%d]seconds" , WHOIS_SERVER_IP , WHOIS_SERVER_PORT , offset_len , offset_len-2 , send_buffer , dtt );
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
			ErrorLog( __FILE__ , __LINE__ , "select[%s:%d] failed , errno[%d]" , WHOIS_SERVER_IP , WHOIS_SERVER_PORT , errno );
			snprintf( output_buffer , output_bufsize-1 , "等待WHOIS服务响应失败" );
			close( sock );
			len = -1 ;
			break;
		}
		else if( nret == 0 )
		{
			ErrorLog( __FILE__ , __LINE__ , "select[%s:%d] timeout , data[%d][%.*s]" , WHOIS_SERVER_IP , WHOIS_SERVER_PORT , offset_len , offset_len , recv_buffer );
			snprintf( output_buffer , output_bufsize-1 , "等待WHOIS服务响应超时" );
			close( sock );
			len = -1 ;
			break;
		}
		
		len = recv( sock , recv_buffer + offset_len , recv_len , 0 ) ;
		if( len == 0 )
		{
			InfoLog( __FILE__ , __LINE__ , "recv[%s:%d] ok , data[%d][%.*s]" , WHOIS_SERVER_IP , WHOIS_SERVER_PORT , len , len , recv_buffer + offset_len );
			break;
		}
		else if( len == -1 )
		{
			ErrorLog( __FILE__ , __LINE__ , "recv[%s:%d] failed , errno[%d]" , WHOIS_SERVER_IP , WHOIS_SERVER_PORT , errno );
			snprintf( output_buffer , output_bufsize-1 , "接收WHOIS服务响应失败" );
			close( sock );
			break;
		}
		else
		{
			InfoLog( __FILE__ , __LINE__ , "recv[%s:%d] ok , data[%d][%.*s]" , WHOIS_SERVER_IP , WHOIS_SERVER_PORT , len , len , recv_buffer + offset_len );
		}
		
		offset_len += len ;
		recv_len -= len ;
		
		time( & tt2 );
		dtt = tt2 - tt1 ;
		timeout -= dtt ;
		if( timeout <= 0 )
		{
			ErrorLog( __FILE__ , __LINE__ , "recv[%s:%d] timeout" , WHOIS_SERVER_IP , WHOIS_SERVER_PORT );
			snprintf( output_buffer , output_bufsize-1 , "接收WHOIS服务响应超时" );
			close( sock );
			len = -1 ;
			break;
		}
	}
	if( len == -1 )
		return -1;
	close( sock );
	InfoLog( __FILE__ , __LINE__ , "recv[%s:%d] ok , elapse [%d]seconds" , WHOIS_SERVER_IP , WHOIS_SERVER_PORT , timeout_bak - timeout );
	if( offset_len == 0 )
		return -1;
	
	InfoLog( __FILE__ , __LINE__ , "COMM %s ELAPSE %d SECONDS" , WHOIS_SERVER_IP , WHOIS_SERVER_TIMEOUT - timeout );
	
	p = strstr( recv_buffer , "No match for domain" ) ;
	if( p )
	{
		snprintf( output_buffer , output_bufsize-1 , "域名%s还未注册" , domain_name );
	}
	else
	{
		char		*p1 = NULL ;
		struct tm	stime ;
		char		*registrar_begin = NULL ;
		char		*registrar_end = NULL ;
		char		*sponsoring_registrar_iana_id_begin = NULL ;
		char		*sponsoring_registrar_iana_id_end = NULL ;
		char		*whois_server_begin = NULL ;
		char		*whois_server_end = NULL ;
		char		*referral_url_begin = NULL ;
		char		*referral_url_end = NULL ;
		char		creation_date[ 12 + 1 ] ;
		char		updated_date[ 12 + 1 ] ;
		char		expiration_date[ 12 + 1 ] ;
		
		/* registrar */
		len = strlen(REGISTRAR) ;
		registrar_begin  = strstr( recv_buffer , REGISTRAR ) ;
		if( registrar_begin  == NULL )
		{
			ErrorLog( __FILE__ , __LINE__ , "[%s] not found" , REGISTRAR );
			snprintf( output_buffer , output_bufsize-1 , "WHOIS服务响应无效，缺少[%s]" , REGISTRAR );
			return -1;
		}
		registrar_begin += len ;
		registrar_end = strstr( registrar_begin , "\n" ) ;
		if( registrar_end == NULL )
		{
			ErrorLog( __FILE__ , __LINE__ , "[\\n] not found" );
			snprintf( output_buffer , output_bufsize-1 , "WHOIS服务响应无效，不完整的[%s]" , REGISTRAR );
			return -1;
		}
		
		/* sponsoring_registrar_iana_id */
		len = strlen(SPONSORING_REGISTRAR_IANA_ID) ;
		sponsoring_registrar_iana_id_begin = strstr( recv_buffer , SPONSORING_REGISTRAR_IANA_ID ) ;
		if( sponsoring_registrar_iana_id_begin == NULL )
		{
			ErrorLog( __FILE__ , __LINE__ , "[%s] not found" , SPONSORING_REGISTRAR_IANA_ID );
			snprintf( output_buffer , output_bufsize-1 , "WHOIS服务响应无效，缺少[%s]" , SPONSORING_REGISTRAR_IANA_ID );
			return -1;
		}
		sponsoring_registrar_iana_id_begin += len ;
		sponsoring_registrar_iana_id_end = strstr( sponsoring_registrar_iana_id_begin , "\n" ) ;
		if( sponsoring_registrar_iana_id_end == NULL )
		{
			ErrorLog( __FILE__ , __LINE__ , "[\\n] not found" );
			snprintf( output_buffer , output_bufsize-1 , "WHOIS服务响应无效，不完整的[%s]" , SPONSORING_REGISTRAR_IANA_ID );
			return -1;
		}
		
		/* whois_server */
		len = strlen(WHOIS_SERVER) ;
		whois_server_begin = strstr( recv_buffer , WHOIS_SERVER ) ;
		if( whois_server_begin == NULL )
		{
			ErrorLog( __FILE__ , __LINE__ , "[%s] not found" , WHOIS_SERVER );
			snprintf( output_buffer , output_bufsize-1 , "WHOIS服务响应无效，缺少[%s]" , WHOIS_SERVER );
			return -1;
		}
		whois_server_begin += len ;
		whois_server_end = strstr( whois_server_begin , "\n" ) ;
		if( whois_server_end == NULL )
		{
			ErrorLog( __FILE__ , __LINE__ , "[\\n] not found" );
			snprintf( output_buffer , output_bufsize-1 , "WHOIS服务响应无效，不完整的[%s]" , WHOIS_SERVER );
			return -1;
		}
		
		/* referral_url */
		len = strlen(REFERRAL_URL) ;
		referral_url_begin = strstr( recv_buffer , REFERRAL_URL ) ;
		if( referral_url_begin )
		{
			referral_url_begin += len ;
			referral_url_end = strstr( referral_url_begin , "\n" ) ;
			if( referral_url_end == NULL )
			{
				ErrorLog( __FILE__ , __LINE__ , "[\\n] not found" );
				return -1;
			}
		}
		else
		{
			referral_url_begin = "" ;
			referral_url_end = referral_url_begin + 1 ;
		}
		
		/* creation_date */
		len = strlen(CREATION_DATE) ;
		p1 = strstr( recv_buffer , CREATION_DATE ) ;
		if( p1 == NULL )
		{
			ErrorLog( __FILE__ , __LINE__ , "[%s] not found" , CREATION_DATE );
			snprintf( output_buffer , output_bufsize-1 , "WHOIS服务响应无效，缺少[%s]" , CREATION_DATE );
			return -1;
		}
		memset( & stime , 0x00 , sizeof(struct tm) );
		strptime( p1 + len , "%d-%b-%Y" , & stime );
		strftime( creation_date , sizeof(creation_date)-1 , "%Y-%m-%d" , & stime );
		
		/* updated_date */
		len = strlen(UPDATED_DATE) ;
		p1 = strstr( recv_buffer , UPDATED_DATE ) ;
		if( p1 == NULL )
		{
			ErrorLog( __FILE__ , __LINE__ , "[%s] not found" , UPDATED_DATE );
			snprintf( output_buffer , output_bufsize-1 , "WHOIS服务响应无效，缺少[%s]" , UPDATED_DATE );
			return -1;
		}
		memset( & stime , 0x00 , sizeof(struct tm) );
		strptime( p1 + len , "%d-%b-%Y" , & stime );
		strftime( updated_date , sizeof(updated_date)-1 , "%Y-%m-%d" , & stime );
		
		/* expiration_date */
		len = strlen(EXPIRATION_DATE) ;
		p1 = strstr( recv_buffer , EXPIRATION_DATE ) ;
		if( p1 == NULL )
		{
			ErrorLog( __FILE__ , __LINE__ , "[%s] not found" , EXPIRATION_DATE );
			snprintf( output_buffer , output_bufsize-1 , "WHOIS服务响应无效，缺少[%s]" , EXPIRATION_DATE );
			return -1;
		}
		memset( & stime , 0x00 , sizeof(struct tm) );
		strptime( p1 + len , "%d-%b-%Y" , & stime );
		strftime( expiration_date , sizeof(expiration_date)-1 , "%Y-%m-%d" , & stime );
		
		/* together all infomation */
		snprintf( output_buffer , output_bufsize ,
			"域名 : %s\n"
			"注册商 : %.*s\n"
			"赞助注册IANA ID : %.*s\n"
			"WHOIS服务器 : %.*s\n"
			"关联URL : %.*s\n"
			"注册日期 : %s\n"
			"更新日期 : %s\n"
			"过期日期 : %s\n"
			, domain_name
			, (int)(registrar_end-registrar_begin) , registrar_begin
			, (int)(sponsoring_registrar_iana_id_end-sponsoring_registrar_iana_id_begin) , sponsoring_registrar_iana_id_begin
			, (int)(whois_server_end-whois_server_begin) , whois_server_begin
			, (int)(referral_url_end-referral_url_begin) , referral_url_begin
			, creation_date
			, updated_date
			, expiration_date
			);
	}
	
	return 0;
}

