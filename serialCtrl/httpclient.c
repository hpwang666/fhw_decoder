﻿/*  Make the necessary includes and set up the variables.  */

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "connet.h"

#include <errno.h>
#include <fcntl.h>
#include "md5.h"
#include "httpclient.h"

static int connect_server(char *ip, int port);
int recv_noblock(httpclient_t ct);
int httpParse(httpclient_t ct)
{
	char *_nonce = strstr(ct->httpBuf, "nonce");
	char *_conn = strstr(ct->httpBuf, "Connection"); //you daxie CONNECTION
	char *_realm = strstr(ct->httpBuf, "realm");
	char *_head = strchr(_nonce, '"');
	char *_tail = strchr(_head + 2, '"');
	unsigned char nonce_len = _tail - _head - 1;

	ct->auth->nonce = (char *)calloc(1, nonce_len + 1);
	strncpy(ct->auth->nonce, _head + 1, nonce_len);

	_head = strchr(_realm, '"');
	_tail = strchr(_head + 2, '"');

	ct->auth->realm = (char *)calloc(1, _tail - _head - 1 + 1);
	strncpy(ct->auth->realm, _head + 1, _tail - _head - 1);

	if (_conn == NULL)
		return -1;
	if (0 == strncmp(_conn + 12, "keep-alive", 10))
		return 1;
	if (0 == strncmp(_conn + 12, "close", 5))
		return 0;
	return -2;
}
int generate_auth(httpclient_t ct, char *uri)
{
	char ha1Buf[33];
	char ha2Buf[33];
	unsigned char tmp[16];
	int res;
	int cnonce = rand();
	static unsigned char fuck[4096];

	res = httpParse(ct);
	memset(fuck, '\0', 4096);
	memset(ct->httpBuf, '\0', 4096);
	sprintf(ct->httpBuf, "%s:%s:%s", ct->auth->user, ct->auth->realm, ct->auth->pass);
	memcpy(fuck, ct->httpBuf, 4096);
	md5_hash(fuck, strlen(ct->httpBuf), tmp);
	hex_from_raw(tmp, 16, ha1Buf);

	memset(fuck, '\0', 4096);
	memset(ct->httpBuf, '\0', 4096);
	snprintf(ct->httpBuf, 4096, "%s:%s", ct->method, uri);
	memcpy(fuck, ct->httpBuf, 4096);
	md5_hash(fuck, strlen(ct->httpBuf), tmp);
	hex_from_raw(tmp, 16, ha2Buf);

	memset(fuck, '\0', 4096);
	memset(ct->httpBuf, '\0', 4096);
	sprintf(ct->httpBuf, "%s:%s:00000001:%08x:auth:%s",
			ha1Buf, ct->auth->nonce, cnonce, ha2Buf);
	memcpy(fuck, ct->httpBuf, 4096);
	md5_hash(fuck, strlen(ct->httpBuf), tmp);
	hex_from_raw(tmp, 16, ha1Buf);

	snprintf(ct->httpBuf, 4096, "Digest username=\"%s\", realm=\"%s\", "
								"nonce=\"%s\", uri=\"%s\", response=\"%s\", qop=auth, "
								"nc=00000001, cnonce=\"%08x\"",
			 ct->auth->user, ct->auth->realm, ct->auth->nonce, uri, ha1Buf, cnonce);
	return res;
}

void mid(const char *src, const char *s1, const char *s2, char *sub)
{
	char *sub1;
	char *sub2;
	int n;

	sub1 = strstr((char *)src, s1);
	if (sub1 == NULL)
	{
		sub[0] = 0;
		return;
	}
	sub1 += strlen(s1);
	sub2 = strstr(sub1, s2);
	if (sub2 == NULL)
	{
		sub[0] = 0;
		return;
	}
	n = sub2 - sub1;
	strncpy(sub, sub1, n);
	sub[n] = 0;
}
int recv_noblock(httpclient_t ct)
{
	int ret=-1;
	fd_set read_fds;
	struct timeval timeout;
	int available=0;
	int recv_len =0;
	int fds =0;
	if(ct->httpFD ==-1)
		return -1;

	timeout.tv_sec = 0;
	timeout.tv_usec = 500 * 1000;       /* 连接超时时长：100ms */

	FD_ZERO(&read_fds);
	FD_SET(ct->httpFD, &read_fds);

	ret = select(ct->httpFD + 1, &read_fds,NULL, NULL, &timeout);
	switch (ret)
	{
		case -1:        /* select错误 */
			{
				printf("recv error1: %s(errno: %d)\n", strerror(errno), errno);
				close(ct->httpFD);
				ct->httpFD = -1;
				break;
			}
		case 0:         /* 超时 */
			{
				printf("select timeout...\n");
				close(ct->httpFD);
				ct->httpFD = -1;
				break;
			}
		default:
			{
				for(fds=0;fds <ct->httpFD + 1;fds++){
					if(FD_ISSET(fds,&read_fds)){
						if(fds == ct->httpFD){
							if (ioctl(ct->httpFD, FIONREAD,&available) == -1) {
								printf("error when ioctl recv\r\n");
							}
							else {
								recv_len= recv(ct->httpFD, ct->httpBuf+ct->pkgLen, 4096, 0);	
							}
						}
					}
				}

				break;
			}
	}
	return recv_len>0?recv_len:-1;
}



static int connect_server(char *ip, int port)
{
	int sockfd = -1;
	struct sockaddr_in servaddr;
	int flags = 0;

	int fds =0;
	if ((sockfd = socket(AF_INET, SOCK_STREAM , 0)) < 0)
	{
		printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
		return sockfd;
	}

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &servaddr.sin_addr) <= 0)
    {
        printf("inet_pton error for %s\n", ip);
        close(sockfd);
        sockfd = -1;
        return sockfd;
    }

    //把链路设置为非阻塞
    flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
    
    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
    {
        if (errno != EINPROGRESS)       /* EINPROGRESS 表示连接正在建立的过程中 */
        {
            printf("connect error0: %s(errno: %d)\n", strerror(errno), errno);
            close(sockfd);
            sockfd = -1;
        }
        else
        {
            int ret;
            fd_set write_fds;
            struct timeval timeout;
            
            timeout.tv_sec = 0;
	        timeout.tv_usec = 200 * 1000;       /* 连接超时时长：100ms */

            FD_ZERO(&write_fds);
            FD_SET(sockfd, &write_fds);
  
            ret = select(sockfd + 1,NULL,&write_fds, NULL, &timeout);
            switch (ret)
            {
                case -1:        /* select错误 */
                {
                    printf("connect error1: %s(errno: %d)\n", strerror(errno), errno);
                    close(sockfd);
                    sockfd = -1;
                    break;
                }
                case 0:         /* 超时 */
                {
                    printf("select timeout...\n");
                    close(sockfd);
                    sockfd = -1;
                    break;
                }
                default:
                {
                    int error = -1;
                    socklen_t optLen = sizeof(socklen_t);
                    
					for(fds=0;fds <sockfd+ 1;fds++){
						if(FD_ISSET(fds,&write_fds)){
							if(fds == sockfd){
								getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (char *)&error, &optLen);       /* 通过 getsockopt 替代 FD_ISSET 判断是否连接 */
								if (error != 0)
								{
									printf("connect error2: %s(errno: %d)\n", strerror(errno), errno);
									close(sockfd);
									sockfd = -1;
								}
							}
						}
					}

					break;
				}
			}
		}
	}

#if 0
	if(sockfd!= -1)
	{
		flags = fcntl(sockfd, F_GETFL, 0);
		flags &=~O_NONBLOCK;
		fcntl(sockfd, F_SETFL, flags  );
	}
#endif
	return sockfd;
}
int httpClientGet(httpclient_t ct, char *uri)
{
	int len;
	int res;
	char code_buf[10];
	char contentLen[10];
	u_char *tail;
	const char *h_header = "User-Agent: FHJT_Http_Client\r\nAccept: */*\r\nConnection: Keep-alive\r\nAccept-Encoding: identity\r\n";

#if 0
	ct->httpFD = socket(AF_INET, SOCK_STREAM, 0);
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(ct->auth->serverIP); //inet_addr()完成地址格式转换
	address.sin_port = htons(80);							 //端口

	res = connect(ct->httpFD, (struct sockaddr *)&address, sizeof(address));
#endif
	res = connect_server(ct->auth->serverIP,80);
	if (res == -1)
	{
		printf("failed0 when connect\r\n");
		ct->httpST = connect_NONE;
		httpClearConn(ct);
		return -1;
	}
	ct->httpFD=res;
	ct->method = strdup("GET");
	sprintf(ct->header, "GET %s HTTP/1.1\r\n", uri);
	len = strlen(ct->header);
	sprintf(ct->header + len, "Host: %s\r\n", ct->auth->serverIP);
	strcat(ct->header, h_header);
	strcat(ct->body, "\r\n");
	res = httpSend(ct);
	if (res < 0)
	{
		httpClearConn(ct);
		return -1;
	}
//	 res = recv(ct->httpFD, ct->httpBuf, 4096, 0);//+++++++++++++++++++
	res = recv_noblock(ct);
	if (res <= 0)
	{
		httpClearConn(ct);
		return -1;
	}

	// printf("###1h:%s,b:%s,f:%s\r\n#\r\n", ct->header, ct->body, ct->httpBuf); ////------1recv,if 200,401,400-------------------

	mid(ct->httpBuf, "HTTP/1.1 ", " ", code_buf);
	if (strcmp(code_buf, "401") == 0)
	{
		res = generate_auth(ct, uri);
		if (res == -1)
		{
			httpClearConn(ct);
			return -1;
		};
		if (res == 0)
		{
			printf("***closeing\n\r");//---last message
			// res = recv(ct->httpFD, ct->httpBuf, 4096, 0);
			// if (res == 0)
			// {
			// 	close(ct->httpFD);
			// }
			res = connect_server(ct->auth->serverIP,80);
			if (res == -1)
			{
				perror("failed when connect");
				ct->httpST = connect_NONE;
				httpClearConn(ct);
				return -1;
			}
			ct->httpFD=res;
		}
		//if(res == 1) printf("***keep alive\n\r");
		strcat(ct->header, "Authorization: ");
		strcat(ct->header, ct->httpBuf);
		strcat(ct->header, "\r\n");
		// printf("h:%s,b:%s,f:%s\r\n\r\n", ct->header, ct->body, ct->httpBuf); //--------2send,have digest
		res = httpSend(ct);
		if (res < 0)
		{
			httpClearConn(ct);
			return -1;
		}
		// res = recv(ct->httpFD, ct->httpBuf, 4096, 0);//++++++++++++++++++++++++++++++++++
		memset(ct->httpBuf, '\0', 4096);
		ct->pkgLen=0;
		res = recv_noblock(ct);
		if (res <= 0)
		{
			httpClearConn(ct);
			return -1;
		}
		ct->pkgLen = res;
		// printf("###2h:%s,b:%s,f:%s\r\n\r\n", ct->header, ct->body, ct->httpBuf); //-------2recv,real message
		mid(ct->httpBuf, "HTTP/1.1 ", " ", code_buf);
		if (strcmp(code_buf, "401") == 0)
		{
			printf("##pass error\r\n");
			return -1;
		}
		else
		{
			if (strcmp(code_buf, "200") != 0)
				printf("##warning:%s,\r\n##RES:%s\r\n",ct->header,ct->httpBuf);

			mid(ct->httpBuf, "Content-Length: ", " ", contentLen);
			ct->contentLen = atoi(contentLen);

			tail = str_nstr((u_char *)ct->httpBuf,"\r\n\r\n",ct->pkgLen);
			//  printf("%d %d\r\n",tail+4-(u_char *)ct->httpBuf,ct->contentLen) ;
			if(ct->contentLen +(tail+4-(u_char *)ct->httpBuf)!=ct->pkgLen){
				
				res = recv_noblock(ct);
				ct->pkgLen +=res;
			}
			return ct->pkgLen;
		}			
	}
	else if (strcmp(code_buf, "404") == 0)
	{
		return -404;
	}else
	{
		if (strcmp(code_buf, "200") != 0)
			printf("##warning:%s,\r\n##RES:%s\r\n",ct->header,ct->httpBuf);
		return res;
	}
}
int httpClientPut(httpclient_t ct, char *uri, char *content)
{
	int len;
	int res;
	char code_buf[10];

	char contentLen[10];
	u_char *tail;

	const char *h_header = "User-Agent: FHJT_Http_Client\r\nAccept: */*\r\nConnection: Keep-alive\r\nAccept-Encoding: identity\r\n";
	const char *h_con_type = "Content-Type: text/xml; charset=\"UTF-8\"\r\n";

	/*  Create a socket for the client.  */

	res = connect_server(ct->auth->serverIP,80);
	if (res == -1)
	{
		printf("failed0 when connect\r\n");
		ct->httpST = connect_NONE;
		httpClearConn(ct);
		return -1;
	}
	ct->httpFD=res;
	ct->method = strdup("PUT");
	sprintf(ct->header, "PUT %s HTTP/1.1\r\n", uri);
	len = strlen(ct->header);
	sprintf(ct->header + len, "Host: %s\r\n", ct->auth->serverIP);
	strcat(ct->header, h_header);
	strcat(ct->header, h_con_type);
	len = strlen(ct->header);
	if(content)
		sprintf(ct->header + len, "Content-Length: %d\r\n",(int) strlen(content));
	strcat(ct->body, "\r\n");
	if(content)
		strcat(ct->body, content);

	//printf("<<<%s%s",ct->header,ct->body);
	res = httpSend(ct);
	if (res < 0)
	{
		httpClearConn(ct);
		return -1;
	}

	memset(ct->httpBuf, '\0', 4096);
	ct->pkgLen=0;
	res = recv_noblock(ct);
	if (res <= 0)
	{
		httpClearConn(ct);
		return -1;
	}
	// printf("###2h:%s,b:%s,f:%s\r\n\r\n", ct->header, ct->body, ct->httpBuf); //-------2recv,real message
	mid(ct->httpBuf, "HTTP/1.1 ", " ", code_buf);
	if (strcmp(code_buf, "401") == 0)
	{

		res = generate_auth(ct, uri);
		if (res == -1)
		{
			httpClearConn(ct);
			return -1;
		}
		if (res == 0)
		{
			printf("***closeing\n\r");
			res = connect_server(ct->auth->serverIP,80);
			if (res == -1)
			{
				perror("failed when connect");
				ct->httpST = connect_NONE;
				httpClearConn(ct);
				return -1;
			}
			ct->httpFD=res;
		}
		else printf("***keep alive\n\r");
		strcat(ct->header, "Authorization: ");
		strcat(ct->header, ct->httpBuf);
		strcat(ct->header, "\r\n");

		//printf("<<<%s%s",ct->header,ct->body);
		res = httpSend(ct);
		if (res < 0)
		{
			httpClearConn(ct);
			return -1;
		}

		res = recv_noblock(ct);
		if (res <= 0)
		{
			httpClearConn(ct);
			return -1;
		}
		ct->pkgLen = res;
		mid(ct->httpBuf, "HTTP/1.1 ", " ", code_buf);
		if (strcmp(code_buf, "401") == 0)
		{
			printf("##pass error\r\n");
			return -1;
		}
		else
		{
			if (strcmp(code_buf, "200") != 0)
				printf("##warning0:%s,\r\n##RES:%s\r\n",ct->header,ct->httpBuf);

			mid(ct->httpBuf, "Content-Length: ", " ", contentLen);
			ct->contentLen = atoi(contentLen);

			tail = str_nstr((u_char *)ct->httpBuf,"\r\n\r\n",ct->pkgLen);
			//  printf("%d %d\r\n",tail+4-(u_char *)ct->httpBuf,ct->contentLen) ;
			if(ct->contentLen +(tail+4-(u_char *)ct->httpBuf)!=ct->pkgLen){
				printf("read again \r\n");
				res = recv_noblock(ct);
				ct->pkgLen +=res;
				printf("%s",ct->httpBuf);
			}
			return ct->pkgLen;
		}			
	}
	else if (strcmp(code_buf, "404") == 0)
	{
		printf("page not found\r\n");
		return -404;
	}else
	{
		if (strcmp(code_buf, "200") != 0)
			printf("##warning:%s,\r\n##RES:%s\r\n",ct->header,ct->httpBuf);
		return -1;
	}
	return res;
}
int httpSend(httpclient_t ct)
{
	int res = 0;
	snprintf(ct->httpBuf, 4096, "%s", ct->header);
	strcat(ct->httpBuf, ct->body);
	res = send(ct->httpFD, ct->httpBuf, strlen(ct->httpBuf), 0);
	memset(ct->httpBuf, '\0', 4096);
	return res;
}

httpclient_t httpClientCreat(char *ip, char *user, char *passwd)
{
	httpclient_t ct = (httpclient_t)calloc(1, sizeof(struct httpclient_st));
	ct->auth = (http_auth_t)calloc(1, sizeof(struct http_auth_st));
	ct->auth->serverIP = strdup(ip);
	if (user && passwd)
	{
		ct->auth->user = strdup(user);
		ct->auth->pass = strdup(passwd);
	}
	ct->header = (char *)calloc(1, 1024);
	ct->body = (char *)calloc(1, 1024);
	ct->httpBuf = (char *)calloc(1, 4096);
	return ct;
}
int httpclientFree(httpclient_t ct)
{
	if (ct->httpFD)
	{
		close(ct->httpFD);
		ct->httpFD = -1;
	}
	if (ct->method)
	{
		free(ct->method);
		ct->method = NULL;
	}
	if (ct->header)
	{
		free(ct->header);
		ct->header = NULL;
	}
	if (ct->body)
	{
		free(ct->body);
		ct->body = NULL;
	}
	if (ct->httpBuf)
	{
		free(ct->httpBuf);
		ct->httpBuf = NULL;
	}
	if (ct->auth->user)
	{
		free(ct->auth->user);
		ct->auth->user = NULL;
	}
	if (ct->auth->pass)
	{
		free(ct->auth->pass);
		ct->auth->pass = NULL;
	}
	if (ct->auth->serverIP)
	{
		free(ct->auth->serverIP);
		ct->auth->serverIP = NULL;
	}
	if (ct->auth->nonce)
	{
		free(ct->auth->nonce);
		ct->auth->nonce = NULL;
	}
	if (ct->auth->realm)
	{
		free(ct->auth->realm);
		ct->auth->realm = NULL;
	}

	if (ct->auth)
	{
		free(ct->auth);
		ct->auth = NULL;
	}
	if (ct)
	{
		free(ct);
	}

	return 0;
}

int httpClearConn(httpclient_t ct)
{
	if (ct->httpFD)
	{
		close(ct->httpFD);
		ct->httpFD = -1;
	}
	if (ct->method)
	{
		free(ct->method);
		ct->method = NULL;
	}
	if (ct->header)
	{
		memset((ct->header), '\0', 1024);
	}
	if (ct->body)
	{
		memset((ct->body), '\0', 1024);
	}
	if (ct->httpBuf)
	{
		memset((ct->httpBuf), '\0', 4096);
	}

	if (ct->auth->nonce)
	{
		free(ct->auth->nonce);
		ct->auth->nonce = NULL;
	}
	if (ct->auth->realm)
	{
		free(ct->auth->realm);
		ct->auth->realm = NULL;
	}
	return 0;
}
