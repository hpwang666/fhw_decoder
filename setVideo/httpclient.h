/*  Make the necessary includes and set up the variables.  */

#ifndef HTTPCLIENT_H_
#define HTTPCLIENT_H_

typedef  enum{connect_NONE = 0,
			connect_ING,
			connect_OK,
			connect_BAD,           /* 连接不稳定 */
			connect_CLOSED} status;

struct http_auth_st
{
	char* serverIP;
	char* user;
	char* pass;
	char* realm;
	char* nonce;
} ;
typedef struct http_auth_st *http_auth_t;


struct httpclient_st
{
	http_auth_t auth;
	char* method;
	char* header;//4096
	char* body;//4096
	char* httpBuf;//8192
	char* recvContent;//4096
	char* sendContent;//4096
	char* authForPtz;
	int pkgLen;
	int contentLen;
	status httpST;//http status
	int httpFD;
};
typedef struct httpclient_st *httpclient_t;


int generate_auth(httpclient_t ct,char *uri);
httpclient_t 	httpClientCreat(char *ip,char *user,char *passwd);
int 	httpClientPut(httpclient_t ct,char *uri,char *content);
int 	httpClientGet(httpclient_t ct,char *uri);
int 	httpSend(httpclient_t ct);
int     httpclientFree(httpclient_t ct);
int 	httpParse(httpclient_t ct);
int 	httpClearConn(httpclient_t ct);

#endif
