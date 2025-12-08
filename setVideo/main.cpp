#include "setStream.h"
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <sys/stat.h>
#include <sys/types.h>

#include "httpclient.h"
#include "sstr.h"

using namespace std;

int main(int argc, char *argv[])
{
	int res=0;
	char passwd[32];
	char encodeType[16];
	char resolution[16];
	char camIp[32][32];

	char tempIp[64];
	char preIp[64];
	char tempStartEnd[32];
	int ipStart,ipEnd;
	int i=0;
	u_char *head;

	
	if(argc != 5){
		printf("please input like this ./setVideo passwd  h264 1080p 192.168.1.1-10\r\n");
		exit(1);
	}

	sprintf(passwd,"%s",argv[1]);
	if(strcmp(argv[2],"h264")==0)
		sprintf(encodeType,"H.264");
	else if(strcmp(argv[2],"h265")==0)
		sprintf(encodeType,"H.265");
	else {
		printf("codec type is not h264 not h265\r\n");
		exit(1);
	}

	if(strcmp(argv[3],"1080p")==0)
		sprintf(resolution,"1080p");
	else  if(strcmp(argv[3],"720p")==0){
		sprintf(resolution,"720p");
	}else{
		printf("resolution is not 720p not 1080p\r\n");
		exit(1);
	}

	sprintf(tempIp,"%s",argv[4]);

	head=str_nstr((u_char*)tempIp,(char *)"-",strlen(tempIp));
	if(head==NULL) {
		printf("when you set ip please input like this 192.168.1.3-10\r\n");
		exit(1);
	}
	else{
		i=head-(u_char*)tempIp;
		do{
			i--;
		}while(tempIp[i]!='.'&&i>0);
		snprintf(preIp,i+2,"%s",tempIp);

		snprintf(tempStartEnd,head-(u_char*)&tempIp[i],"%s",tempIp+i+1);
		ipStart=atoi(tempStartEnd);
		sprintf(tempStartEnd,"%s",head+1);
		ipEnd=atoi(tempStartEnd);

		if((ipEnd-ipStart)>31){
			printf("only support 32 ip one time\r\n");
			exit(1);
		}
		if(ipEnd<ipStart){
			printf("end < start\r\n");
			exit(1);
		}


		printf("%s %d-%d\r\n",preIp,ipStart,ipEnd);

		for(i=0;i<(ipEnd-ipStart+1);i++){
			sprintf(camIp[i],"%s%d",preIp,ipStart+i);
		}
		
	}

	for(i=0;i<(ipEnd-ipStart+1);i++){
			printf("\r\n\r\n>>>set %s\r\n",camIp[i]);
			res=setMainStream(camIp[i],passwd,encodeType,resolution);
			if(res>0){
				res=setSubStream(camIp[i],passwd,encodeType);
				if(res>0)
					printf(">>>>>>>>>>>>>>>>>>> set cam %s success <<<<<<<<<<<<<<<<<<<\r\n",camIp[i]);
			}
			else printf("***************** set cam %s failed ******************\r\n",camIp[i]);
	}
	
	
    return 0;
}


