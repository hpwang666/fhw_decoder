#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

int main(int argc, char **argv)
{
  

    //if(mkfifo("tmpfifo",0644) == -1){
    //    perror("mkfifo error");
       // exit(EXIT_FAILURE);
    //}
    int fd ;
    fd = open("tmpfifo",O_WRONLY);
    if(fd == -1){
        perror("open error");
        exit(EXIT_FAILURE);
    }
    char buf[16]="sssss";
    int n = 0;
    while(1){
		n=write(fd,buf,5);
		printf("writen %d bytes\n",n);
		sleep(4);
       
    }
  
    close(fd);
    printf("write success\n");
    return 0;
}
