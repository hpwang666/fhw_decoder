#ifndef _HTTPSERVER_H
#define _HTTPSERVER_H
#include <stdint.h>
#include "connet.h"
#include "util.h"









/* HTTP Method */
#define		METHOD_ERR			0				/**< Error Method. */
#define		METHOD_GET			1				/**< GET Method.   */
#define		METHOD_HEAD			2				/**< HEAD Method.  */
#define		METHOD_POST			3				/**< POST Method.  */

/* HTTP GET Method */
#define		PTYPE_ERR			0				/**< Error file. */
#define		PTYPE_HTML			1				/**< HTML	file.  */
#define		PTYPE_GIF			2				/**< GIF	file.  */
#define		PTYPE_TEXT			3				/**< TEXT file.  */
#define		PTYPE_JPEG			4				/**< JPEG file.  */
#define		PTYPE_FLASH			5				/**< FLASH file. */
#define		PTYPE_MPEG			6				/**< MPEG file.  */
#define		PTYPE_PDF			7				/**< PDF file.   */

#ifdef USE_FATFS_FLASH
#define WEB_ROOT_PATH 	"/.sys"
#endif
/* HTML Doc. for ERROR */
#define ERROR_HTML_PAGE "HTTP/1.1 404 OK\r\nContent-Type: text/html\r\nContent-Length: 78\r\n\r\n<HTML>\r\n<BODY>\r\nSorry, the page you requested was not found.\r\n</BODY>\r\n</HTML>\r\n\0"

#define ERROR_REQUEST_PAGE "HTTP/1.1 400 OK\r\nContent-Type: text/html\r\nContent-Length: 50\r\n\r\n<HTML>\r\n<BODY>\r\nInvalid request.\r\n</BODY>\r\n</HTML>\r\n\0"

/* Response header for HTML*/
#define RES_HTMLHEAD_OK	"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: "

/* Response head for TEXT */
#define RES_TEXTHEAD_OK	"HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: "

/* Response head for GIF */
#define RES_GIFHEAD_OK	"HTTP/1.1 200 OK\r\nContent-Type: image/gif\r\nContent-Length: "

/* Response head for JPEG */
#define RES_JPEGHEAD_OK	"HTTP/1.1 200 OK\r\nContent-Type: image/jpeg\r\nContent-Length: "		

/* Response head for FLASH */
#define RES_FLASHHEAD_OK "HTTP/1.1 200 OK\r\nContent-Type: application/x-shockwave-flash\r\nContent-Length: "

/* Response head for MPEG */
#define RES_MPEGHEAD_OK "HTTP/1.1 200 OK\r\nContent-Type: video/mpeg\r\nContent-Length: "	

/* Response head for PDF */
#define RES_PDFHEAD_OK "HTTP/1.1 200 OK\r\nContent-Type: application/pdf\r\nContent-Length: "

//digital I/O out put control result response
#define DOUT_RES_1  "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 1\r\n\r\n1"
#define DOUT_RES_0  "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 1\r\n\r\n0"

#define MAX_URI_SIZE	4096 //(TX_RX_MAX_BUF_SIZE/2 - sizeof(char)*2)		

/**
 @brief 	Structure of HTTP REQUEST 
 */

typedef struct http_request_st *http_request_t;
struct http_request_st
{
  uint8_t	METHOD;													/**< request method(METHOD_GET...). */
  uint8_t	TYPE;													/**< request type(PTYPE_HTML...).   */
  char	URI[MAX_URI_SIZE];										/**< request file name.             */
  u_char tx_buf[MAX_URI_SIZE];//__attribute__((at(0x68002000)));
  u_char rx_buf[MAX_URI_SIZE];
};

typedef struct _http_trans_control
{
	uint8_t transmit_task;
#ifdef USE_FATFS_FLASH
	uint16_t transmit_count;
	uint32_t remain_filesize;			/* Data stream */
	char filename[256];
	FIL fil;	// FatFs File objects
	FRESULT fr;	// FatFs function common result code
#endif
}http_trans_control;

void unescape_http_url(char * url);								/* convert escape character to ascii */

void parse_http_request(http_request_t , uint8_t *,uint32_t);			/* parse request from peer */
	
void make_http_response_head(u_char *, uint8_t, uint32_t);	/* make response header */

unsigned char* get_http_param_value(char* uri, char* param_name);/* get the user-specific parameter value */

void proc_http(conn_t c, uint8_t * buf,uint32_t len);

int do_https(event_t ev);



#endif
