#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#define MAX_SIZE 255
typedef struct MsgBody
{
    long type;
    char content[MAX_SIZE];
} TMsgBody;
int main(int argc, char *argv[])
{
    char stin[MAX_SIZE];
    TMsgBody msg;
    scanf("%s\r\n",stin);
    
    // printf("%s end",stin);
    // exit(0);
    key_t qname = 12349;
    if (qname < 0)
    {
        printf("qname ftok fail %d\r\n", qname);
        exit(1);
    }
    ssize_t qhandle = msgget(qname, IPC_CREAT | 0666);
    if (qhandle < 0)
    {
        printf("open msg fail\r\n");
        exit(1);
    }
 
    if (qhandle)
    {
        int msgsz = sizeof(TMsgBody);
        msg.type = 10;
        strncpy(msg.content, stin, MAX_SIZE);
        strcat(msg.content, "\r\n");
        
        char result = msgsnd(qhandle, (void *)&msg, msgsz, 0);
        if (result == 0)
        {
            printf("send ok\r\n");
        }
        else
        {
            printf("send fail msgid %zd, %d\r\n", qhandle, result);
        }
         
        if (errno == EEXIST)
        {
            printf("Exists %d \r\n", EEXIST);
        }
    }
}
