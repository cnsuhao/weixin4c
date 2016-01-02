#ifndef _H_PRIVATE_
#define _H_PRIVATE_

#include "public.h"

#include "openssl/sha.h"

#include "IDL_xml.dsc.h"

int cgiinit();
int cgimain();
int cgiclean();

int VerifyServer( char *signature , char *timestamp , char *nonce , char *echostr );
int ReceiveText( char *post_data , int post_data_len , xml *p_req );

#endif

