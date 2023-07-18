#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

int main(int argc, char **argv)
{
   
   

    int fd ;
    fd = open("tmpfifo",O_RDONLY|O_NONBLOCK);
    if(fd == -1){
        perror("open error");
        exit(EXIT_FAILURE);
    }
    char buf[1024*4];
    int n = 0;
    while(1){
		n = read(fd,buf,64);
		sleep(1);
       if(n>0) printf("%d:%s\n",n,buf);
	   else printf("no data\n");
    }
    close(fd);
    //close(outfd);
    unlink("tmpfifo");
    printf("read success\n");
    return 0;
}
