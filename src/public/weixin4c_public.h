#ifndef _H_PUBLIC_
#define _H_PUBLIC_

#include "fcgi_stdio.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <dlfcn.h>
#include <iconv.h>

#include "LOGC.h"

#include "curl/curl.h"

#ifndef MAX
#define MAX(_a_,_b_) (_a_>_b_?_a_:_b_)
#endif

#ifndef MIN
#define MIN(_a_,_b_) (_a_<_b_?_a_:_b_)
#endif

char *PUBStringNoEnter( char *str );

int PUBHexExpand( char *HexBuf , int HexBufLen , char *AscBuf );

void TakeoffCDATA( char *str_with_cdata );

int PUBReadEntireFile( char *filename , char *mode , char *p_buf , long *p_file_size );
int PUBReadEntireFileSafely( char *filename , char *mode , char **pp_buf , long *p_file_size );
int PUBWriteEntireFile( char *filename , char *mode , char *p_buf , long file_size );

int PUBConvCharacterCodeEx( char *from_character_code , char *in_buf , int in_len , char *to_character_code , char *out_buf , int out_len );
int PUBConvCharacterCode( char *from_character_code , char *to_character_code , char *buf , int len , int size );
char *PUBConvCharacterCodeStatic( char *from_character_code , char *to_character_code , char *buf , int len );
void PUBFreeCharacterCodeStatic();
int PUBDupConvCharacterCode( char *from_character_code , char *to_character_code , char *buf , int len , char **out_dup );

/*
int PUBDecodingUnicode( char *str );
*/

int PUBGetUrlParamPtr( char *key , char **pp_value , int *p_value_len );
int PUBGetUrlParam( char *key , char *value , int value_size );
int PUBDupUrlParam( char *key , char **pp_value );

void PUBSendContentTypeHtml();

int PUBReallocPostBuffer( int post_len );
int PUBReadPostBuffer();
char *PUBGetPostBufferPtr();
int PUBGetPostBufferLength();
void PUBFreePostBuffer();

struct CurlResponseBuffer
{
	int		buf_size ;
	int		str_len ;
	char		*base ;
} ;

size_t CurlResponseProc( char *buffer , size_t size , size_t nmemb , void *p );
void CurlFreeBuffer( struct CurlResponseBuffer *pbuf );

#endif

