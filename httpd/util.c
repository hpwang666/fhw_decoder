/**
******************************************************************************
* @file    			utility.c                                            
* @version 			V1.2										  		
* @date    			2017-11-01								 
* @brief 				公共实用函数 延时函数 数据格式转换      					  	
*
* @company  		深圳炜世科技有限公司
* @information  WIZnet W5500官方代理商，全程技术支持，价格绝对优势！
* @website  		www.wisioe.com																							 	 				
* @forum        www.w5500.cn																								 		
* @qqGroup      383035001																										 
******************************************************************************
*/



#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include "util.h"

//static u8  fac_us=0;		     							/*us延时倍乘数*/
//static u16 fac_ms=0;			  		 					/*ms延时倍乘数*/

/**
*@brief		初始化延迟函数
*@param		SYSCLK:系统时钟
*@return	无
*/
//void systick_init (u8 sysclk)
//{
//	SysTick->CTRL&=0xfffffffb;							/*bit2清空,选择外部时钟  HCLK/8*/
//	fac_us=sysclk/8;		    
//	fac_ms=(u16)fac_us*1000;
//}								    

/**
*@brief	  秒延时函数
*@param		time_s:要延时秒时间数
*@return	无
*/

/**
*@brief	 	字符转转化为8位整型函数
*@param		str:要转化字符串， base:
*@return	num:返回转化后的整型数
*/
uint16_t atoi16(char* str,uint16_t base	)
{
  unsigned int num = 0;
  while (*str !=0)
          num = num * base + c2d(*str++);
  return num;
}

/**
*@brief	 	字符转转化为32位整型函数
*@param		str:要转化字符串， base:
*@return	num:返回转化后的整型数
*/
uint32_t atoi32(char* str,uint16_t base	)
{
  uint32_t num = 0;
  while (*str !=0)
          num = num * base + c2d(*str++);
  return num;
}

/**
*@brief	 	整型数转化为字符串函数
*@param		n:要转化整数， str:存放转化后的字符串  len：整型数长度
*@return	无
*/
void itoa(uint16_t n,uint8_t* str, uint8_t len)
{
  uint8_t i=len-1;

  memset(str,0,len);
  do{
	str[i--]=n%10+'0';
  }while((n/=10)>0);

 return;
}


/**
*@brief	 	把字符串转化为十进制或十六进制数函数
*@param		str:要转化字符串， len：整型数长度
*@return	成功 - 1, 失败 - 0
*/
int validatoi(char* str, int base,int* ret)
{
  int c;
  char* tstr = str;
  if(str == 0 || *str == '\0') return 0;
  while(*tstr != '\0')
  {
    c = c2d(*tstr);
    if( c >= 0 && c < base) tstr++;
    else    return 0;
  }
  
  *ret = atoi16(str,base);
  return 1;
}

/**
*@brief	 	用新的字符去替换字符串中特殊的字符
*@param		str:替换后字符串，oldchar:特殊的字符，newchar：新的字符	
*@return	无
*/
void replacetochar(char * str,	char oldchar,char newchar	)
{
  int x;
  for (x = 0; str[x]; x++) 
    if (str[x] == oldchar) str[x] = newchar;	
}

/**
*@brief	 	把十进制数转化为字符型
*@param		c:要转化十进制数据
*@return	返回一个字符型数据
*/
char c2d(uint8_t c	)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return 10 + c -'a';
	if (c >= 'A' && c <= 'F')
		return 10 + c -'A';

	return (char)c;
}

/**
*@brief	 	16位字符高8位低8位转换
*@param		i:要转化的数据
*@return	转换后的数据
*/
uint16_t swaps(uint16_t i)
{
  uint16_t ret=0;
  ret = (i & 0xFF) << 8;
  ret |= ((i >> 8)& 0xFF);
  return ret;	
}
/**
*@brief	 	32位字符高低位变换
*@param		i:要转化的数据
*@return	转换后的数据
*/
uint32_t swapl(uint32_t l)
{
  uint32_t ret=0;
  ret = (l & 0xFF) << 24;
  ret |= ((l >> 8) & 0xFF) << 16;
  ret |= ((l >> 16) & 0xFF) << 8;
  ret |= ((l >> 24) & 0xFF);
  return ret;
}

/**
*@brief	 	字符串处理
*@param		src目标字符串 s1 s2操作字符串
*@return	无
*/
void mid(char* src, char* s1, char* s2, char* sub)
{
	char* sub1;
	char* sub2;
	uint16_t n;

	sub1=strstr(src,s1);
	if(sub1==NULL)
	{
		sub="";
		return;
	}
		
	sub1+=strlen(s1);
	sub2=strstr(sub1,s2);
	if(sub2==NULL)
	{
		sub="";
		return;
	}
	n=sub2-sub1;
	strncpy(sub,sub1,n);
	sub[n]=0;
}

/**
*@brief	 	ip网络地址转换
*@param		adr：地址 ip：ip
*@return	无
*/
void inet_addr_(unsigned char* addr,unsigned char *ip)
{
	int i;
	char taddr[30];
	char * nexttok;
	char num;
	strcpy(taddr,(char *)addr);
	
	nexttok = taddr;
	for(i = 0; i < 4 ; i++)
	{
		nexttok = strtok(nexttok,".");
		if(nexttok[0] == '0' && nexttok[1] == 'x') num = atoi16(nexttok+2,0x10);
		else num = atoi16(nexttok,10);
		
		ip[i] = num;
		nexttok = NULL;
	}
}	
 

/**
*@brief	 	将16地址转化为十进制格式
*@param		addr:  要转化地址
*@return	返回十进制格式地址
*/
char* inet_ntoa_pad(unsigned long addr)
{
	static char addr_str[16];
	memset(addr_str,0,16);
	printf(addr_str,"%03d.%03d.%03d.%03d",(int)(addr>>24 & 0xFF),(int)(addr>>16 & 0xFF),(int)(addr>>8 & 0xFF),(int)(addr & 0xFF));
	return addr_str;
}
 
/**
*@brief	 	验证IP地址
*@param		ip addr
*@return	成功 - 1, 失败 - 0
*/
int verify_ip_address(char* src )
{
	int j,i;
	int tnum;
	char tsrc[50];
	char* tok = tsrc;
	
	strcpy(tsrc,src);
	
	for(i = 0; i < 4; i++)
	{
		tok = strtok(tok,".");
		if ( !tok ) return 0;
		j=strlen(tok);
		if(j>3) return 0;
		for(j=0;j<strlen(tok);j++){
			if(tok[j]>0x39||tok[j]<0x30)
				return 0;
		}
		if(atoi(tok)>255) return 0;
		tok = NULL;
	}
	return 1;	
}


int verify_num(char* src )
{
	int j;
	
	for(j=0;j<strlen(src);j++){
		if(src[j]>0x39||src[j]<0x30)
			return 0;
	}
	return 1;	
}

int verify_plc_ctrl(char* src )
{
	int j,i;
	int tnum;
	char tsrc[50];
	char* tok = tsrc;
	
	strcpy(tsrc,src);
	if(strlen(src)==0) return 0;
	
	for(i = 0; i < 4; i++)
	{
		tok = strtok(tok,",");
		if ( !tok ) return 1;
		j=strlen(tok);
		if(j>3) return 0;
		for(j=0;j<strlen(tok);j++){
			if(tok[j]>0x39||tok[j]<0x30)
				return 0;
		}
		if(atoi(tok)>16) return 0;
		tok = NULL;
	}
	return 1;	
}

#if 0
/**
*@brief	 	将32地址转化为十进制格式
*@param		addr:  要转化地址
*@return	返回十进制格式地址
*/
char* inet_ntoa(
	unsigned long addr	
	)
{
	static char addr_str[32];
	memset(addr_str,0,32);
	sprintf(addr_str,"%d.%d.%d.%d",(int)(addr>>24 & 0xFF),(int)(addr>>16 & 0xFF),(int)(addr>>8 & 0xFF),(int)(addr & 0xFF));
	return addr_str;
}



/**
*@brief		将一个 主机模式的unsigned short型数据转换到大端模式的TCP/IP 网络字节格式的数据.
*@param		要转换的数据
*@return 	大端模式的数据
*/ 
uint16_t htons( 
	uint16_t hostshort	/**< A 16-bit number in host byte order.  */
	)
{
	#if ( SYSTEM_ENDIAN == _ENDIAN_LITTLE_ )
		return swaps(hostshort);
	#else
		return hostshort;
	#endif		
}

/**
*@brief		将一个 主机模式的unsigned long型数据转换到大端模式的TCP/IP 网络字节格式的数据.
*@param		要转换的数据
*@return 	大端模式的数据
*/ 
uint32_t htonl(
	unsigned long hostlong		/**< hostshort  - A 32-bit number in host byte order.  */
	)
{
#if ( SYSTEM_ENDIAN == _ENDIAN_LITTLE_ )
	return swapl(hostlong);
#else
	return hostlong;
#endif	
}



/**
*@brief		将一个大端模式的TCP/IP 网络字节格式的数据转换到主机模式的unsigned short型数据
*@param		要转换的数据
*@return 	unsigned short模式的数据
*/ 
uint32_t ntohs(
	unsigned short netshort	/**< netshort - network odering 16bit value */
	)
{
#if ( SYSTEM_ENDIAN == _ENDIAN_LITTLE_ )	
	return htons(netshort);
#else
	return netshort;
#endif		
}


/**
*@brief		将一个大端模式的TCP/IP 网络字节格式的数据转换到主机模式的unsigned long型数据
*@param		要转换的数据
*@return 	unsigned long模式的数据
*/ 
uint32_t ntohl(unsigned long netlong)
{
#if ( SYSTEM_ENDIAN == _ENDIAN_LITTLE_ )
	return htonl(netlong);
#else
	return netlong;
#endif		
}
#endif
/**
*@brief		计算字符串校验值
*@param		要转换的数据
*@return 	校验值
*/ 
unsigned short checksum(
	unsigned char * src, 	/**< pointer to stream  */
	unsigned int len		/**< size of stream */
	)
{
	uint16_t sum, tsum, i, j;
	uint32_t lsum;

	j = len >> 1;

	lsum = 0;

	for (i = 0; i < j; i++) 
	{
		tsum = src[i * 2];
		tsum = tsum << 8;
		tsum += src[i * 2 + 1];
		lsum += tsum;
	}

	if (len % 2) 
	{
		tsum = src[i * 2];
		lsum += (tsum << 8);
	}

	sum = lsum;
	sum = ~(sum + (lsum >> 16));
	return (uint16_t) sum;	
}

/**
*@brief		检查地址是否匹配
*@param		要转换的数据
*@return 	成功为1 失败为0
*/ 
uint8_t check_dest_in_local(uint32_t destip)
{
/* 	int i = 0;
	uint8_t * pdestip = (uint8_t*)&destip;
	for(i =0; i < 4; i++)
	{
		if((pdestip[i] & IINCHIP_READ(SUBR0+i)) != (IINCHIP_READ(SIPR0+i) & IINCHIP_READ(SUBR0+i)))
			return 1;	// Remote
	} */
	return 0;
}
