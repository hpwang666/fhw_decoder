#ifndef _UART_H
#define _UART_H



int initSerial(void)  ;
int uartSend(int serial_fd, char *data, int datalen);  
int serialRecv(int serial_fd,char *data)  ; 
int closeSerial(int serial_fd);
#endif