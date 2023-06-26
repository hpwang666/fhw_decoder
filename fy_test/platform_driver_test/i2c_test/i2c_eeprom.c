#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>

int main(void){
    int fd, size, len, i;
    char buf[50] = {0};
    //char *bufw="Hi,this is an eepromtest!";
    char *bufw="ieeprom!";

    len=strlen(bufw);

    fd= open("/sys/bus/i2c/devices/0-0050/eeprom", O_RDWR);
    if(fd < 0)
    {
        printf("####i2c test device open failed####/n");
        return (-1);
    }

    lseek(fd,0x40,SEEK_SET);
    if((size=write(fd,bufw, len)) < 0)
    {
        printf("write error\n");
        return 1;
    }

    printf("writeok\n");
    lseek(fd, 0x40, SEEK_SET);

    if((size=read(fd, buf, len)) < 0)
    {
        printf("readerror\n");
        return 1;
    }
    printf("readok\n");

    for(i=0; i< len; i++)
	printf("[%d]=0x%x\n", i, buf[i]);


    sleep(2);
    printf("\n--%s--\n", buf);
    sleep(2);

    close(fd);

    return 0;
}
