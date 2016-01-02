#include "public.h"
#include "private.h"

int main()
{
	int		nret = 0 ;
	
	nret = cgiinit() ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "cgiinit failed[%d]" , nret );
		return -nret;
	}
	
	while( FCGI_Accept() >= 0 )
	{
		nret = cgimain() ;
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "cgimain failed[%d]" , nret );
			return -nret;
		}
break;
	}
	
	nret = cgiclean() ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "cgiclean failed[%d]" , nret );
		return -nret;
	}
	
	return 0;
}

