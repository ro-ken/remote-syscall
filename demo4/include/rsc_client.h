#ifndef RSC_CLIENT_H_
#define RSC_CLIENT_H_  1

// remote syscall request encode and remote syscall result decode
int RequestEncode(struct user_regs_struct * regs, struct rsc_header * header, char * write_buffer, pid_t pid);                                                                         
int ResultDecode(struct user_regs_struct * regs, struct rsc_header * header, char * extra_buffer, pid_t pid);

// pointer paraments handle
int     PointerEncode(struct rsc_header * header, char * write_buffer, pid_t pid);
char *  OutputPointerEncode(unsigned int p_location, unsigned int p_count, struct rsc_header * header);
char *  InputPointerEncode(unsigned int p_location, unsigned int p_count, struct rsc_header * header, pid_t pid);
int     OutputPointerDecode(struct user_regs_struct * regs, struct rsc_header * header, char * extra_buffer, pid_t pid);

// create socket connect in client
int     SocketConnect(char* ip_addr, int port);

// syscall bitmap operation
void    ResetBitmap(unsigned long long int * syscall_bitmap, unsigned int syscall);
int     IsSet(unsigned long long int * syscall_bitmap, unsigned int syscall);
void    SetBitmap(unsigned long long int * syscall_bitmap, unsigned int syscall);

void DebugPrintf(struct rsc_header * header);

// ptrace operation tracee process memory data
void GetData(pid_t child, unsigned long addr, char *str, int len);
void PutData(pid_t child, unsigned long addr, char *str, int len);

#endif