#include "public.h"

char *PUBStringNoEnter( char *str )
{
	char	*ptr = NULL ;
	
	if( str == NULL )
		return NULL;
	
	for( ptr = str + strlen(str) - 1 ; ptr >= str ; ptr-- )
	{
		if( (*ptr) == '\r' || (*ptr) == '\n' )
			(*ptr) = '\0' ;
		else
			break;
	}
	
	return str;
}

int PUBHexExpand( char *HexBuf , int HexBufLen , char *AscBuf )
{
	int	i,j=0;
	char	c;
	
	for(i=0;i<HexBufLen;i++){
		c=(HexBuf[i]>>4)&0x0f;
		if(c<=9) AscBuf[j++]=c+'0';
		else AscBuf[j++]=c+'a'-10;

		c=HexBuf[i]&0x0f;
		if(c<=9) AscBuf[j++]=c+'0';
		else AscBuf[j++]=c+'a'-10;
		}
	AscBuf[j]=0;
	return(0);
}

int PUBReadEntireFileSafely( char *filename , char *mode , char **pp_buf , long *p_file_size )
{
	FILE	*fp = NULL ;
	long	file_size ;
	char	*p_buf = NULL ;
	int	n ;
	
	fp = fopen( filename , mode ) ;
	if( fp == NULL )
	{
		return -1;
	}
	
	fseek( fp , 0 , SEEK_END );
	file_size = ftell( fp ) ;
	p_buf = (char*)malloc( file_size + 1 ) ;
	if( p_buf == NULL )
	{
		fclose( fp );
		return -2;
	}
	memset( p_buf , 0x00 , file_size + 1 );
	
	fseek( fp , 0 , SEEK_SET );
	
	n = (int)fread( p_buf , file_size , 1 , fp ) ;
	if( n != 1 )
	{
		free( p_buf );
		fclose( fp );
		return -2;
	}
	
	fclose( fp );
	
	if( pp_buf )
		(*pp_buf) = p_buf ;
	else
		free( p_buf );
	if( p_file_size )
		(*p_file_size) = file_size ;
	
	return 0;
}

int PUBWriteEntireFile( char *filename , char *mode , char *p_buf , long file_size )
{
	FILE	*fp = NULL ;
	int	n ;
	
	fp = fopen( filename , mode ) ;
	if( fp == NULL )
	{
		return -1;
	}
	
	if( file_size >= 0 )
	{
		n = (int)fwrite( p_buf , file_size , 1 , fp );
		if( n != 1 )
		{
			return -2;
		}
	}
	else
	{
		fprintf( fp , "%s" , p_buf );
	}
	
	fclose( fp );
	
	return 0;
}

int PUBConvCharacterCodeEx( char *from_character_code , char *in_buf , int in_len , char *to_character_code , char *out_buf , int out_len )
{
	char	*in_ptr = NULL ;
	char	*out_ptr = NULL ;
	size_t	in_left ;
	size_t	out_left ;
	iconv_t	v ;
	size_t	r ;
	int	converted ;
	
	in_ptr = in_buf ;
	in_left = in_len ;
	out_ptr = out_buf ;
	out_left = out_len ;
	
	v = iconv_open( to_character_code , from_character_code ) ;
	if( v == (iconv_t)-1 )
		return -1;
	
	r = iconv( v , & in_ptr , & in_left , & out_ptr , & out_left ) ;
	iconv_close( v );
	if( r == (size_t)-1 )
		return -2;
	
	converted = out_len - (int)out_left ;
	out_buf[ out_len-out_left ] = '\0' ;
	
	return out_len-out_left;
}

int PUBConvCharacterCode( char *from_character_code , char *to_character_code , char *buf , int len , int size )
{
	char	*tmp = NULL ;
	int	converted ;
	
	tmp = (char*)malloc( size ) ;
	if( tmp == NULL )
		return -5;
	memset( tmp , 0x00 , size );
	
	if( len == -1 )
		len = strlen( buf ) ;
	converted = PUBConvCharacterCodeEx( from_character_code , buf , len , to_character_code , tmp , size-1 ) ;
	if( converted >= 0 )
	{
		memcpy( buf , tmp , converted );
		buf[ converted ] = '\0' ;
	}
	free( tmp );
	
	return converted;
}

int PUBDupConvCharacterCode( char *from_character_code , char *to_character_code , char *buf , int len , char **out_dup )
{
	int	size ;
	char	*tmp = NULL ;
	int	converted ;
	
	size = len * 2 + 1 ;
	
	tmp = (char*)malloc( size ) ;
	if( tmp == NULL )
		return -5;
	memset( tmp , 0x00 , size );
	
	if( len == -1 )
		len = strlen( buf ) ;
	converted = PUBConvCharacterCodeEx( from_character_code , buf , len , to_character_code , tmp , size-1 ) ;
	if( converted >= 0 )
	{
		if( out_dup )
		{
			(*out_dup) = tmp ;
		}
	}
	
	return converted;
}

