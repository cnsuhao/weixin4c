#include "weixin4c_public.h"

extern void _setenv();
extern void *_dlopen();

typedef int funcInitEnvProc();
typedef int funcCleanEnvProc();

typedef int funcReceiveSubscribeEventProc( char *output_buffer , int *p_output_buflen , int output_bufsize );
typedef int funcReceiveUnsubscribeEventProc( char *output_buffer , int *p_output_buflen , int output_bufsize );
typedef int funcReceiveTextMsgProc( char *input_buffer , int input_buflen , int input_bufsize , char *output_buffer , int *p_output_buflen , int output_bufsize );

