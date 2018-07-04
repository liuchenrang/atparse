#ifndef __UART__
#define __UART__
 void (*SerialPort_Event_ByteReceived)(unsigned char bt);
 void SerialPort_Init_High();
 void SerialPort_SendData(unsigned char *cc);
 void os_log(char *msg);
#endif

