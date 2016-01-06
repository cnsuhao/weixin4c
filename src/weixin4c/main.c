#include "weixin4c.h"

int main()
{
	struct Environment	env , *penv = & env ;
	int			init_flag = 0 ;
	
	int			nret = 0 ;
	
	_setenv();
	
	memset( penv , 0x00 , sizeof(struct Environment) );
	
	while( FCGI_Accept() >= 0 )
	{
		if( init_flag == 0 )
		{
			nret = cgiinit( penv ) ;
			if( nret )
			{
				ErrorLog( __FILE__ , __LINE__ , "cgiinit failed[%d]" , nret );
				return -nret;
			}
			
			init_flag = 1 ;
		}
		
		nret = cgimain( penv ) ;
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "cgimain failed[%d]" , nret );
			nret = cgiclean( penv ) ;
			return -nret;
		}
break;
	}
	
	nret = cgiclean( penv ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "cgiclean failed[%d]" , nret );
		return -nret;
	}
	
	return 0;
}

