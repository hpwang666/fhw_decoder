﻿/*  Make the necessary includes and set up the variables.  */

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "md5.h"
#include "httpclient.h"

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

	sub1 = strstr(src, s1);
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
ssize_t super_recv(httpclient_t ct)
{
	int res, len, i;
	char lastdata[2048];
	char len_str[10];
	const char *sub1;
	memset(ct->httpBuf, 0, 4096);
	for (i = 0; i < 5; i++)
	{
		memset(lastdata, '\0', 2048);
		memset(len_str, '\0', 10);
		res = recv(ct->httpFD, lastdata, 2048, 0);
		// printf("i=%d,1:%s\n",i,lastdata);
		if (res > 0)
		{
			strcat(ct->httpBuf, lastdata);
			// printf("i=%d,2:%s\n",i,ct->httpBuf);
		}
		else
		{
			res = -1;
			break;
		}
		char *_http = strstr(ct->httpBuf, "HTTP/1.1");
		char *_Content = strstr(ct->httpBuf, "Content-Length:");
		char *_content = strstr(ct->httpBuf, "content-length:");
		char *_CONTENT = strstr(ct->httpBuf, "CONTENT-LENGTH:");
		if (_http != NULL && (_content != NULL || _Content != NULL || _CONTENT != NULL))
		{
			if (_Content != NULL)
				mid(ct->httpBuf, "Content-Length: ", "\r\n", len_str);
			else if (_content != NULL)
				mid(ct->httpBuf, "content-length: ", "\r\n", len_str);
			else
				mid(ct->httpBuf, "CONTENT-LENGTH: ", "\r\n", len_str);
			// printf("i=%d,3:%s\n",i,len_str);
			len = atoi(len_str);
			if (len == 0)
				break;
			else
			{
				sub1 = strstr(ct->httpBuf, "\r\n\r\n");
				if (sub1 == NULL)
					continue;
				// printf("ctt:%s#",sub1);
				if (len == (strlen(sub1) - 4))
					break;
			}
		}
	}
	// printf("i=%d",i);
	return res;
}
int httpClientGet(httpclient_t ct, char *uri)
{
	int len;
	struct sockaddr_in address;
	int res;
	char code_buf[10];
	const char *h_header = "User-Agent: FHJT_Http_Client\r\nAccept: */*\r\nConnection: Keep-alive\r\nAccept-Encoding: identity\r\n";

	ct->httpFD = socket(AF_INET, SOCK_STREAM, 0);
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(ct->auth->serverIP); //inet_addr()完成地址格式转换
	address.sin_port = htons(80);							 //端口

	res = connect(ct->httpFD, (struct sockaddr *)&address, sizeof(address));
	if (res == -1)
	{
		perror("failed when connect");
		ct->httpST = connect_NONE;
		httpClearConn(ct);
		return -1;
	}
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
	// res = recv(ct->httpFD, ct->httpBuf, 4096, 0);//+++++++++++++++++++
	res = super_recv(ct);
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
			//printf("***closeing\n\r");//---last message
			// res = recv(ct->httpFD, ct->httpBuf, 4096, 0);
			// if (res == 0)
			// {
			// 	close(ct->httpFD);
			// }
			ct->httpFD = socket(AF_INET, SOCK_STREAM, 0);
			len = sizeof(address);
			res = connect(ct->httpFD, (struct sockaddr *)&address, len);
			if (res == -1)
			{
				perror("failed when connect");
				ct->httpST = connect_NONE;
				httpClearConn(ct);
				return -1;
			}
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
		res = super_recv(ct);
		if (res <= 0)
		{
			httpClearConn(ct);
			return -1;
		}
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
			return res;
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
	struct sockaddr_in address;
	int res;

	char *h_header = "User-Agent: FHJT_Http_Client\r\nAccept: */*\r\nConnection: Keep-alive\r\nAccept-Encoding: identity\r\n";
	char *h_con_type = "Content-Type: text/xml; charset=\"UTF-8\"\r\n";

	/*  Create a socket for the client.  */

	ct->httpFD = socket(AF_INET, SOCK_STREAM, 0);
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(ct->auth->serverIP); //inet_addr()完成地址格式转换
	address.sin_port = htons(80);							 //端口
	len = sizeof(address);
	res = connect(ct->httpFD, (struct sockaddr *)&address, len);
	if (res == -1)
	{
		perror("failed when connect");
		ct->httpST = connect_NONE;
		httpClearConn(ct);
		return -1;
	}
	ct->method = strdup("PUT");
	sprintf(ct->header, "PUT %s HTTP/1.1\r\n", uri);
	len = strlen(ct->header);
	sprintf(ct->header + len, "Host: %s\r\n", ct->auth->serverIP);
	strcat(ct->header, h_header);
	strcat(ct->header, h_con_type);
	len = strlen(ct->header);
	if(content)
		sprintf(ct->header + len, "Content-Length: %d\r\n", strlen(content));
	strcat(ct->body, "\r\n");
	if(content)
		strcat(ct->body, content);

	printf("<<<%s%s",ct->header,ct->body);
	res = httpSend(ct);
	if (res < 0)
	{
		httpClearConn(ct);
		return -1;
	}

	res = recv(ct->httpFD, ct->httpBuf, 4096, 0);
	//printf(">>>%s\n",ct->httpBuf);
	if (res <= 0)
	{
		httpClearConn(ct);
		return -1;
	}

	res = generate_auth(ct, uri);
	if (res == -1)
	{
		httpClearConn(ct);
		return -1;
	}
	if (res == 0)
	{
		printf("***closeing\n\r");
		res = recv(ct->httpFD, ct->httpBuf, 4096, 0);
		if (res == 0)
		{
			close(ct->httpFD);
		}
		ct->httpFD = socket(AF_INET, SOCK_STREAM, 0);
		len = sizeof(address);
		res = connect(ct->httpFD, (struct sockaddr *)&address, len);
		if (res == -1)
		{
			perror("failed when connect");
			ct->httpST = connect_NONE;
			httpClearConn(ct);
			return -1;
		}
	}

	if(res == 1) printf("***keep alive\n\r");
	strcat(ct->header, "Authorization: ");
	strcat(ct->header, ct->httpBuf);
	strcat(ct->header, "\r\n");

	printf("<<<%s%s",ct->header,ct->body);
	res = httpSend(ct);
	if (res < 0)
	{
		httpClearConn(ct);
		return -1;
	}

	res = recv(ct->httpFD, ct->httpBuf, 4096, 0);
	printf(">>>%s\n",ct->httpBuf);
	if (res <= 0)
	{
		httpClearConn(ct);
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
