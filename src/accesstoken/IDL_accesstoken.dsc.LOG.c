/* It had generated by DirectStruct v1.6.1 */
#include "IDL_accesstoken.dsc.h"

#ifndef FUNCNAME_DSCLOG_accesstoken
#define FUNCNAME_DSCLOG_accesstoken	DSCLOG_accesstoken
#endif

#ifndef PREFIX_DSCLOG_accesstoken
#define PREFIX_DSCLOG_accesstoken	printf( 
#endif

#ifndef NEWLINE_DSCLOG_accesstoken
#define NEWLINE_DSCLOG_accesstoken	"\n"
#endif

int FUNCNAME_DSCLOG_accesstoken( accesstoken *pst )
{
	int	index[10] = { 0 } ; index[0] = 0 ;
	PREFIX_DSCLOG_accesstoken "accesstoken.access_token[%s]" NEWLINE_DSCLOG_accesstoken , pst->access_token );
	PREFIX_DSCLOG_accesstoken "accesstoken.expires_in[%d]" NEWLINE_DSCLOG_accesstoken , pst->expires_in );
	return 0;
}