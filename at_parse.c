#include "ascii.h"
#include "uart.h"
#include <stdlib.h>
#define BUFFER_MAX_SIZE 99  //缓冲区大小
unsigned char buffer[BUFFER_MAX_SIZE];  //缓冲区：用于存放从ESP8266接收来的各种信息

//连接到TCP服务器的指令：AT+CIPSTART="TCP","192.168.1.110",1234。后面的CR和NL是AT指令的固定结尾，EOT用于SerialPort_SendData发送时识别结尾。
unsigned char cmd_connectToTCPServer[] = {0x41, 0x54, 0x2B, 0x43, 0x49, 0x50, 0x53, 0x54, 0x41, 0x52, 0x54, 0x3D, 0x22, 0x54, 0x43, 0x50, 0x22, 0x2C, 0x22, 0x31, 0x39, 0x32, 0x2E, 0x31, 0x36, 0x38, 0x2E, 0x31, 0x2E, 0x31, 0x31, 0x30, 0x22, 0x2C, 0x31, 0x32, 0x33, 0x34, CR, NL, EOT};
int counter = 0;    //用于ESP8266的执行步骤计数
int writeIndex = 0;    //缓冲区写索引

void (*ESP01_Event_WifiConnected)();
void (*ESP01_Event_IpGot)();
void (*ESP01_Event_TcpServerConnected)();
void (*ESP01_Event_MsgReceived)(unsigned char* head);

//注意：下面代码推荐从后往前看，从注释标"1. "处开始。

void prepareForData(unsigned char byte);    //因为第四步和第三步会相互调用，所以这里只是做了个声明（C语言的矫情点）。

//4. 将信息插入到缓冲区并送给。
void insertDataIntoBuffer(unsigned char byte){
    if(byte == '\\'){
        //检测到'\'后，将信息送出到
        buffer[writeIndex] = '\0';
        (*ESP01_Event_MsgReceived)(buffer);
        SerialPort_Event_ByteReceived = &prepareForData;    //回到第三步，准备接收下一条信息
        writeIndex = 0;
        return;
    }
    buffer[writeIndex++] = byte;
}

//3. 准备信息：这里是过度步骤，前面可以观察到，ESP8266在接收发来的信息时是有个头的，这里的作用就是去头。
void prepareForData(unsigned char byte){
    if(byte == ':'){
        SerialPort_Event_ByteReceived = &insertDataIntoBuffer;
        writeIndex = 0;
    }
}

//识别回馈指令：用于识别接收到的是WIFI CONNECTED（连上热点）还是WIFI IP GOT（获得IP）还是CONNECT（连上TCP服务器）
void parseCmd(){
    switch(counter){
        case 1:
            if(buffer[0] == 'W' && buffer[5] == 'C'){
                (*ESP01_Event_WifiConnected)();
                counter += 1;
            }
            break;
        case 2:
            if(buffer[0] == 'W' && buffer[5] == 'G'){
                (*ESP01_Event_IpGot)();
                counter += 1;
            }
            break;
        case 3:
            if(buffer[0] == 'A' && buffer[3] == 'C')
                counter += 1;
            break;
        case 4:
            if(buffer[0] == 'C' && buffer[3] == 'N' && buffer[6] == 'T'){
                (*ESP01_Event_TcpServerConnected)();
                SerialPort_Event_ByteReceived = &prepareForData;  //连接到TCP服务器后，进入第三步。
            }
    }
}

//2. 这里开始向缓冲区存储信息，用于识别。
void insertBuffer(unsigned char byte){
    if(byte == NL){
        //收到尾（NL）后，将缓冲区的回馈信息送去识别
        parseCmd();
        writeIndex = 0;
        return;
    }
    buffer[writeIndex++] = byte;
}

//1. 接收头：头是无用信息，但我们要通过头里面的一些字符，推算出什么时候到达第二步（WIFI CONNECTED）
void headerReceived(unsigned char byte){
    if(byte == NL){
        //头内有5个NL，只要数够5个，下一个就是第二步的内容了。
        if(++counter == 5){
            os_log("5 NL finish");
            SerialPort_Event_ByteReceived = &insertBuffer;  //跳到第二步
            counter = 1;
        }
    }
}

//同.h中的声明
void ESPInit(){
    SerialPort_Init_High();
    SerialPort_Event_ByteReceived = &headerReceived;  //事件注册
}

//同.h中的声明
void ESPConnectToTCPServer(){
    SerialPort_SendData(cmd_connectToTCPServer);
}
 
 