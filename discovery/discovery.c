/*
 * discovery_agent.c
 *
 *  Created on: 2017年3月10日
 *      Author: wangz
 */

#include <sys/un.h>
#include <fcntl.h>
 #include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <ifaddrs.h>
#include <string.h>
#include <assert.h>
#include <poll.h>
#include <errno.h>
 
 
 typedef struct devReport_st *devReport_t;
typedef struct dataTrans_st *dataTrans_t;
typedef struct modifyIP_st *modifyIP_t;
typedef struct reboot_st *reboot_t;

static const char* MULTICAST_IP = "224.129.1.1"; //多播组地址
static const int MULTICAST_PORT = 29735; //多播组端口
int getDevInfo( devReport_t dev);

#define FILE_PATH "/mnt/usr/netconfig.txt"
static int prepare_socket(){
	int s = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if(s == -1){
		return -1;
	}
	int r = 0;
	const int reuse_on = 1;
	const int routenum = 10;
	const int loopback = 0; //禁止回馈
	r |= setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse_on, sizeof(reuse_on));
	r |= setsockopt(s, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&routenum, sizeof(routenum));
	r |= setsockopt(s, IPPROTO_IP, IP_MULTICAST_LOOP, (char*)&loopback, sizeof(loopback));
	if(r){
		fprintf(stderr, "can not set multicast TTL\n");
		close(s);
		return -1;
	}

	struct sockaddr_in local;
	memset(&local, 0, sizeof(local));
	local.sin_family = AF_INET;
	local.sin_port = htons(MULTICAST_PORT);
	//INADDR_ANY为0.0.0.0
	local.sin_addr.s_addr = INADDR_ANY;
	r |= bind(s, (struct sockaddr*)(&local), sizeof(local));
	if(r){
		fprintf(stderr, "Failed to bind\n");
		close(s);
		return -1;
	}

	//多播组结构
	struct ip_mreq mreq;
	memset(&mreq, 0, sizeof(mreq));
	//本机地址
	mreq.imr_interface.s_addr = INADDR_ANY;
	//点分十进制地址转化为IP地址
	mreq.imr_multiaddr.s_addr = inet_addr(MULTICAST_IP);

	//加入一个多播组
	r |= setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq));
	if(r){
		fprintf(stderr, "Failed to add the multicast group.\n");
		close(s);
		return -1;
	}
	return s;
}



enum {
	DIS_CMD_REPORT_UNACT,
	DIS_CMD_REPORT,
	DIS_CMD_NOTIFY,
	DIS_CMD_DETECT = 100,
	DIS_CMD_ACTIVATE,
	DIS_CMD_MODIFY_IP,
	DIS_CMD_MODIFY_ADMINPASS,
	DIS_CMD_REBOOT,
};
struct devReport_st
{
	char model[40];
	char serial[40]; // filled by mac address if not activated.
	char ip[20];
	char mask[20];
	char gateway[20];
	char dns[20];
	char ver_soft[20];
	char ver_sys[20];
	char ctx[8];
};

struct modifyIP_st
{
	
	char ip[20];
	char mask[20];
	char gateway[20];
	char dns[20];
	char serial_chk[40];
	char admin_pass[40];
	char ctx[8]; //用于确定一个临时的会话
};

struct reboot_st
{
	char serial_chk[40];
	char admin_pass[40];
	char ctx[8]; //用于确定一个临时的会话
};

struct dataTrans_st
{
	int cmd;
	int datalen;
};

static volatile int stop_sender = 0;
static volatile int need_restart_sender = 0;
static volatile int g_need_restart_process = 0;
static volatile time_t next_send = 0;
static volatile int exiting = 0;

static char dev_ctx[32];
static char dev_mac[32];

static void do_multicast_send(int sock, const void *data, int len){
	
    //发送用户输入的数据到多播组
    struct sockaddr_in remote;
    memset(&remote, 0, sizeof(remote));
    remote.sin_addr.s_addr = inet_addr ( MULTICAST_IP );
    remote.sin_family = AF_INET ;
    remote.sin_port = htons(MULTICAST_PORT);
	sendto(sock, data,  len, 0, (struct sockaddr*)(&remote), sizeof(remote));
}

    
static void do_modify_ip(modifyIP_t modifyIP)
{
		char wBuf[512]={0};
		printf("%s\n",modifyIP->ctx);
		if(0 != memcmp(modifyIP->ctx, dev_ctx, 8))
		return;
		//printf(">>>>>>>>>> modify ip \n");
		FILE *fp;
		fp = fopen(FILE_PATH,"w");
		if(NULL == fp ) return ;
		char str[] = "This is runoob.com";
		sprintf(wBuf,"mac:%s\n",dev_mac);
		sprintf(wBuf+strlen(wBuf),"address:%s\n",modifyIP->ip);
		sprintf(wBuf+strlen(wBuf),"gateway:%s\n",modifyIP->gateway);
		sprintf(wBuf+strlen(wBuf),"netmask:%s\n",modifyIP->mask);
		sprintf(wBuf+strlen(wBuf),"hostname:%s\n",modifyIP->dns);
		sprintf(wBuf+strlen(wBuf),"id:%s\n",dev_ctx);
		printf("write:%s",wBuf);
		 fwrite(wBuf, strlen(wBuf) , 1, fp );
		//printf("%s",wBuf);
		fclose(fp);
		
		
}


static void do_reboot(reboot_t reboot){
	if(0 != memcmp(reboot->ctx, dev_ctx, 8))
		return;
	printf(">>>>>>>>>> reboot \n");
	system("reboot");
}


static void* _do_receive(void* arg){
	int sock = (int)(uintptr_t)arg;
    char recvbuf[4096];
	char decbuf[2048];
	
    
	
	devReport_t devReport =(devReport_t) calloc(1,sizeof(struct devReport_st));
	dataTrans_t dataTrans =(dataTrans_t) calloc(1,sizeof(struct dataTrans_st));
	
    for(;!exiting;) {
    	fd_set rdset;
    	FD_ZERO(&rdset);
    	FD_SET(sock, &rdset);
    	struct timeval to = {5, 0};
    	select(sock+1, &rdset, NULL, NULL, &to);
    	if(! FD_ISSET(sock, &rdset)){
    		continue;
    	}
        struct sockaddr_in client;
        int clientLen = sizeof(client);
    	memset(&client, 0, clientLen);

    	int ret = recvfrom(sock, recvbuf, sizeof(recvbuf), 0, (struct sockaddr*)(&client), &clientLen);
    	if ( ret == 0){
    		continue;
    	} else if( ret == -1 ) {
    		exit(1);
    	} else {
    		struct dataTrans_st* dt = (struct dataTrans_st*) recvbuf;
    		switch(dt->cmd){
    	
    		case DIS_CMD_DETECT:
    			printf("%s > dev detection\n", inet_ntoa(client.sin_addr));
    			getDevInfo(devReport);
				
				dataTrans->cmd = DIS_CMD_REPORT;
				dataTrans->datalen = sizeof(struct devReport_st);
				memcpy(decbuf, dataTrans, sizeof(struct dataTrans_st));
				memcpy(decbuf+sizeof(struct dataTrans_st), devReport, sizeof(struct devReport_st));
				do_multicast_send(sock,decbuf,sizeof(struct dataTrans_st)+sizeof(struct devReport_st));
				
    			break;
    		
    		case DIS_CMD_MODIFY_IP:
    			do_modify_ip((modifyIP_t)(recvbuf+sizeof(struct dataTrans_st)));
    			break;
			case DIS_CMD_REBOOT:
				do_reboot((reboot_t)(recvbuf+sizeof(struct dataTrans_st)));
				break;
    		default:
    			//printf("%s >>>>>>>>>> \n", inet_ntoa(client.sin_addr));
    			break;
    		}
    	}
    }
	return NULL;
}

static char *getip()
{
    struct ifaddrs *ifaddr, *ifa;
    int family, s;
    char *host = NULL;

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return NULL;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;

        family = ifa->ifa_addr->sa_family;

        if (!strcmp(ifa->ifa_name, "lo"))
            continue;
        if (family == AF_INET) {
            if ((host = malloc(NI_MAXHOST)) == NULL)
                return NULL;
            s = getnameinfo(ifa->ifa_addr,
                    (family == AF_INET) ? sizeof(struct sockaddr_in) :
                    sizeof(struct sockaddr_in6),
                    host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            if (s != 0) {
                return NULL;
            }
            freeifaddrs(ifaddr);
            return host;
        }
    }
    return NULL;
}

int getDevInfo( devReport_t dev)
{
	uint8_t readBuf[512]={0};
	FILE *fp;
	char   *ip;
	char *head,*tail;
	fp = fopen(FILE_PATH,"r");
	if(NULL == fp) return -1;
	
	fread(readBuf,1,512,fp);
	
	memcpy(dev->ctx,dev_ctx,8);
	memcpy(dev->serial,dev_ctx,12);
	
	snprintf(dev->model,40,"fhk201");
	snprintf(dev->ver_soft,20,"v1.0.0");
	snprintf(dev->ver_sys,20,"v1.0");
	snprintf(dev->dns,20,"0.0.0.0");
	
	ip = getip();
	if(NULL ==ip) return -1;
	
	sprintf(dev->ip,"%s", ip);
	free( ip );
	
	head = strstr(readBuf,"mask:");
	if (NULL == head ) return -1;
	head+=5;
	tail = strstr(head,"\n");
	if (NULL == tail ) return -1;
  	snprintf(dev->mask,tail-head+1,"%s", head);
	
	head = strstr(readBuf,"gateway:");
	if (NULL == head ) return -1;
	head+=8;
	tail = strstr(head,"\n");
	if (NULL == tail ) return -1;
  	snprintf(dev->gateway,tail-head+1,"%s", head);


	head = strstr(readBuf,"mac:");
	if (NULL == head ) return -1;
	head+=4;
	tail = strstr(head,"\n");
	if (NULL == tail ) return -1;
  	snprintf(dev_mac,tail-head+1,"%s", head);
	fclose(fp);
	return 1;
}

int main(int argc, char* argv[]){
	
	int s = prepare_socket();
	char *head;
	char readBuf[512];
	if(s == -1){
		// maybe we have no route to the multicast destinations, add a route entry for it.
		fprintf(stderr, "can not prepare multicast socket\n");
		return 1;
	}
	printf("multicast socket on %d\n", s);
	
	FILE *fp;
	fp = fopen(FILE_PATH,"r");
	if(NULL == fp) return -1;
	fread(readBuf,1,512,fp);
	head = strstr(readBuf,"id:");
	if (NULL == head ) return -1;
	memcpy(dev_ctx,head+3,12);
	fclose(fp);
	dev_ctx[12]=0;
	printf("id:%s\r\n",dev_ctx);
    pthread_t  receiver;
    pthread_create(&receiver, NULL, _do_receive, (void*)(uintptr_t)s);
    
	while(!exiting){
		
		sleep(1);
	}
	
	pthread_join(receiver, NULL);
    close(s);
	printf("exit\n");
	return 0;
}
