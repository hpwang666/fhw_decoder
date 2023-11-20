#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

int main(int argc, char **argv)
{
   
   
	const char* sep = ",";

	char a[12]="1,2";
	//sprintf(a,"1,2,3");
	char *p=NULL;
	p = strtok(a,sep);
	if(p)
		printf("%d\r\n",atoi(p));
	p=strtok(NULL,",");
	if(p)
		printf("%s\n",p);
	return 0;
}
