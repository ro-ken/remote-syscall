#ifndef _RSC_CLIENT_H
#define _RSC_CLIENT_H  2

#include "rsc_include.h"

// 建立 socket server
int initial_socket(int port, const char* ip);

// RSCQ handle
int syscall_request_decode(struct rsc_header * header, char * buffer);
int syscall_request_execute(struct rsc_header * header);
char * syscall_return_encode(struct rsc_header * header);

// RSCQ pointer handle
int in_pointer_decode_server(struct rsc_header * header, char * buffer);
int out_pointer_decode_server(struct rsc_header * header);

// output pointer handle
char * out_pointer_encode_server(struct rsc_header * header);


#endif