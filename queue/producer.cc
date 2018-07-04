#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
typedef struct MsgBody
{
    long type;
    char content;
} TMsgBody;
#define MSG_FILE "/tmp/msg.c"
int main(int argc, char *argv[])
{
    struct MsgBody msg;
    struct MsgBody  rec_msg;
    key_t qname = 12347;
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
        int msgsz = sizeof(struct MsgBody) - sizeof(long);
        msg.type = 10;
        msg.content = '1';
        char result = msgsnd(qhandle, (void *)&msg, msgsz, 0);
        if (result == 0)
        {
            printf("send ok\r\n");
        }
        else
        {
            printf("send fail msgid %zd, %d\r\n", qhandle, result);
        }


        int rev_result = msgrcv(qhandle, &rec_msg, msgsz, 1, IPC_NOWAIT);
        printf("rev_result  %d", rev_result);
        if (rev_result == 0)
        {
            printf("rev_content  %c", rec_msg.content);
        }
    }
}
