//**************************************************************
 
#ifndef _MODBUSCRC_H
#define _MODBUSCRC_H
 
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>


uint16_t modbus_CRC16(uint8_t *pFrame, uint16_t count);
uint16_t mb_CRC16(uint8_t* pbuffer, int length);
 
#endif
