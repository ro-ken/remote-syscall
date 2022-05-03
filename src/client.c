#include <sys/types.h>
#include <sys/socket.h>
#include<sys/syscall.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>
 
 
#define PORT 9990
#define SERVER_IP "106.13.194.114"  //百度云地址
#define SIZE 1024
 
int client_socket;
int listen_socket;

char* itoa(int num,char* str,int radix);
long  atol(const char *nptr);
int  atoi(const char *nptr);
void argstostr(char* buf,int number,long RDI,long RSI,long RDX,long R10,long R8,long R9);
int remote_syscall(int number,long RDI,long RSI,long RDX,long R10,long R8,long R9);
int init_socket();
int disconnect();



int main()
{
	init_socket();

    // 39 获取pid
	int res = remote_syscall(SYS_getpid,-2,3,-4,5,-6,7);  

    printf("res = %d\n",res);

	disconnect();
	
	return 0;
}

// 远程系统调用
int remote_syscall(int number,long RDI,long RSI,long RDX,long R10,long R8,long R9){

	char buf[SIZE] = {0};
	
    argstostr(buf,number,RDI,RSI,RDX,R10,R8,R9);    //准备参数

    write(client_socket, buf, strlen(buf));     //发送参数
    printf("buf = %s\n", buf);
    int ret = read(client_socket, buf, strlen(buf));    //接受返回值
    
    int res = atoi(buf);
	
	return res;
}

char* itoa(int num,char* str,int radix)
{
    char index[]="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";//索引表
    unsigned unum;//存放要转换的整数的绝对值,转换的整数可能是负数
    int i=0,j,k;//i用来指示设置字符串相应位，转换之后i其实就是字符串的长度；转换后顺序是逆序的，有正负的情况，k用来指示调整顺序的开始位置;j用来指示调整顺序时的交换。
 
    //获取要转换的整数的绝对值
    if(radix==10&&num<0)//要转换成十进制数并且是负数
    {
        unum=(unsigned)-num;//将num的绝对值赋给unum
        str[i++]='-';//在字符串最前面设置为'-'号，并且索引加1
    }
    else unum=(unsigned)num;//若是num为正，直接赋值给unum
 
    //转换部分，注意转换后是逆序的
    do
    {
        str[i++]=index[unum%(unsigned)radix];//取unum的最后一位，并设置为str对应位，指示索引加1
        unum/=radix;//unum去掉最后一位
 
    }while(unum);//直至unum为0退出循环
 
    str[i]='\0';//在字符串最后添加'\0'字符，c语言字符串以'\0'结束。
 
    //将顺序调整过来
    if(str[0]=='-') k=1;//如果是负数，符号不用调整，从符号后面开始调整
    else k=0;//不是负数，全部都要调整
 
    char temp;//临时变量，交换两个值时用到
    for(j=k;j<=(i-1)/2;j++)//头尾一一对称交换，i其实就是字符串的长度，索引最大值比长度少1
    {
        temp=str[j];//头部赋值给临时变量
        str[j]=str[i-1+k-j];//尾部赋值给头部
        str[i-1+k-j]=temp;//将临时变量的值(其实就是之前的头部值)赋给尾部
    }
 
    return str;//返回转换后的字符串
 
}


long  atol(const char *nptr)
{
        int c;              /* 当前要转换的字符(一个一个字符转换成数字) */
        long total;         /* 当前转换结果 */
        int sign;           /* 标志转换结果是否带负号*/
 
        /*跳过空格，空格不进行转换*/
        while ( isspace((int)(unsigned char)*nptr) )
            ++nptr;
 
        c = (int)(unsigned char)*nptr++;//获取一个字符准备转换 
        sign = c;           /*保存符号标示*/
        if (c == '-' || c == '+')
            c = (int)(unsigned char)*nptr++;    /*跳过'+'、'-'号，不进行转换*/
 
        total = 0;//设置转换结果为0 
 
        while (isdigit(c)) {//如果字符是数字 
            total = 10 * total + (c - '0');     /* 根据ASCII码将字符转换为对应的数字，并且乘10累积到结果 */
            c = (int)(unsigned char)*nptr++;    /* 取下一个字符 */
        }
 
         //根据符号指示返回是否带负号的结果 
        if (sign == '-')
            return -total;
        else
            return total;  
}

int  atoi(const char *nptr)
{
        return (int)atol(nptr);
}
 

//把参数转化为字符串进行发送
void argstostr(char* buf,int number,long RDI,long RSI,long RDX,long R10,long R8,long R9){
    itoa(number,buf,10);
    buf[strlen(buf)] = ',';
    itoa(RDI,buf + strlen(buf),10);
    buf[strlen(buf)] = ',';
    itoa(RSI,buf + strlen(buf),10);
    buf[strlen(buf)] = ',';
    itoa(RDX,buf + strlen(buf),10);
    buf[strlen(buf)] = ',';
    itoa(R10,buf + strlen(buf),10);
    buf[strlen(buf)] = ',';
    itoa(R8,buf + strlen(buf),10);
    buf[strlen(buf)] = ',';
    itoa(R9,buf + strlen(buf),10);
}



// 初始化套接字
int init_socket(){
	client_socket = socket(AF_INET, SOCK_STREAM, 0);   //创建和服务器连接套接字
	if(client_socket == -1)
	{
		perror("socket");
		return -1;
	}

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	
	addr.sin_family = AF_INET;  /* Internet地址族 */
    addr.sin_port = htons(PORT);  /* 端口号 */
    addr.sin_addr.s_addr = htonl(INADDR_ANY);   /* IP地址 */
	inet_aton(SERVER_IP, &(addr.sin_addr));
 
	int addrlen = sizeof(addr);
	listen_socket =  connect(client_socket,  (struct sockaddr *)&addr, addrlen);  //连接服务器
	if(listen_socket == -1)
	{
		perror("connect");
		return -1;
	}
	
	printf("成功连接到一个服务器\n");
}

// 断开连接
int disconnect(){
	return close(listen_socket);
}
