#ifndef RSC_CLIENT_H_
#define RSC_CLIENT_H_  2

#include "rsc.h"

// Create and initial socket server
int InitialSocket(int port, const char* ip);

// remote syscall request handle
int RSCHandle(int sockfd_c);


int RequestDecode(struct rsc_header * header, char * buffer);
int RequestExecute(struct rsc_header * header);
char * ResultEncode(struct rsc_header * header);

// RSCQ pointer handle
int InputPointerDecode(struct rsc_header * header, char * buffer);
int OutputPointerDecode(struct rsc_header * header);

// output pointer handle
char * OutputPointerEncode(struct rsc_header * header);


#endif