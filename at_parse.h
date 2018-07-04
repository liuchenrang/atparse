#ifndef __ESP8266__
#define __ESP8266__

extern void (*ESP01_Event_WifiConnected)();    //事件：Wi-Fi已连接
extern void (*ESP01_Event_IpGot)();    //事件：IP地址已获得
extern void (*ESP01_Event_TcpServerConnected)();    //事件：已连接到TCP服务器
extern void (*ESP01_Event_MsgReceived)(unsigned char* head);    //事件：已获得消息，head为消息数组头

void ESPInit();  //无线模块初始化
void ESPConnectToTCPServer();  //连接TCP服务器

#endif
 