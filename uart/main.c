/*added in 2020/4/8*/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "uart.h"

#undef  _DEBUG
#define _DEBUG
#ifdef _DEBUG
	#define debug(...) printf(__VA_ARGS__)
#else
	#define debug(...)
#endif 




static int got_sig_term = 0;
static void on_sig_term(int sig)
{
	got_sig_term = 1;
	printf("term\n\r");
}

int main()
{
	int fd = initSerial()  ;
	char bufIn[1024];
	int len=0;

	uartSend(fd, "init serial", 11);  
	while(!got_sig_term)
	{
		//sleep(1);
		len = serialRecv(fd,bufIn)  ; 
		if(len){
			bufIn[len]=0;
			//printf("%s\r\n",bufIn);
		}
	}
	
	return 0;
}





