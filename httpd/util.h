#ifndef _UTIL_H
#define _UTIL_H
#include "stdio.h"
#include <stdint.h>



uint16_t atoi16(char* str,uint16_t base); 			      				/* Convert a string to integer number */
uint32_t atoi32(char* str,uint16_t base); 			     	 				/* Convert a string to integer number */
void itoa(uint16_t n,uint8_t* str, uint8_t len);
int validatoi(char* str, int base, int* ret); 						/* Verify character string and Convert it to (hexa-)decimal. */
char c2d(uint8_t c); 					                    				/* Convert a character to HEX */

uint16_t swaps(uint16_t i);
uint32_t swapl(uint32_t l);

void replacetochar(char * str, char oldchar, char newchar);

void mid(char* src, char* s1, char* s2, char* sub);
void inet_addr_(unsigned char* addr,unsigned char *ip);

char* inet_ntoa_pad(unsigned long addr);



int verify_ip_address(char* src );
int verify_plc_ctrl(char* src );

uint16_t checksum(unsigned char * src, unsigned int len);		/* Calculate checksum of a stream */

uint8_t check_dest_in_local(uint32_t destip);			            /* Check Destination in local or remote */

#endif
