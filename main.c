#include <stdio.h>
#include "uart.h"
#include "ascii.h"
#include "at_parse.h"
#include <string.h>
#include "libserialport.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <pthread.h>
#define MAX_SIZE 255
typedef struct MsgBody
{
    long type;
    char content[MAX_SIZE];
} TMsgBody;

const char response[] = "\n\n\n\n\nAT\r\nOK\r\n";
void parse_serial(char *byte_buff, int byte_num)
{
    for (int i = 0; i < byte_num; i++)
    {
        printf("%c", byte_buff[i]);
    }
    printf("\n");
}
void waitTrans()
{
    usleep(1000 * 30);
}
void sendAT(struct sp_port *port, char *out_buffer)
{
    enum sp_return serialResult;
    // Add 'RX ready' event to serial port
    printf("write start\r\n");
    struct sp_event_set *eventSet;
    sp_new_event_set(&eventSet);
    sp_add_port_events(eventSet, port, SP_EVENT_TX_READY);

    sp_wait(eventSet, 0);
    serialResult = sp_blocking_write(port, out_buffer, sizeof(out_buffer), 0);
    if (serialResult < 0)
    {
        printf("send error %d\r\n", serialResult);
    } else {
        printf("write end\r\n");
    }
}
typedef struct ThrReadPort
{
    struct sp_port *port;
    TMsgBody *msg;
} TThrReadPort;
void readInit(struct sp_port *port, int waitTime)
{
    char *in_buffer;
    enum sp_return serialResult;

    // Add 'RX ready' event to serial port
    printf("\r\nread start\r\n");
    struct sp_event_set *eventSet;
    sp_new_event_set(&eventSet);
    sp_add_port_events(eventSet, port, SP_EVENT_RX_READY);
    sp_wait(eventSet, waitTime);
    printf("\r\nread wait\r\n");

    enum sp_return    bytes_waiting = sp_input_waiting(port);
    in_buffer = malloc(bytes_waiting * sizeof(char));
    serialResult = sp_blocking_read(port, in_buffer, bytes_waiting, 0);
    printf("%s", in_buffer);
    fflush(stdout);
    free(in_buffer);
}
void *thr_func(void *p)
{
    // TThrReadPort *m = (TThrReadPort *)p;
    // while (1)
    // {
    //     printf("fhr func %s \r\n", m->msg->content);
    //     sleep(3);
    //     readInit(m->port);
    // }
    return NULL;
}
int main()
{
    TMsgBody msg;

    ESPInit();
    int i = 0;
    struct sp_port **ports;
    sp_list_ports(&ports);
    while (ports[i] != NULL)
    {
        printf("port name: %s\r\n", sp_get_port_name(ports[i]));
        i++;
    }

    struct sp_port *port = ports[2];
    if (!port)
    {
        printf("port 0 fail\r\n");
        return 1;
    }
    if (SP_OK != sp_open(port, SP_MODE_READ | SP_MODE_WRITE))
    {
        printf("no support read write\r\n");
        return 2;
    }
    usleep(1000 * 1000 * 2);

    printf("ok %s\n", sp_get_port_name(port));

    sp_set_baudrate(port, 115200);

    // sp_set_baudrate(port, 74880);

    unsigned long bytes_waiting;
    char *in_buffer;
    // sp_flush( port, SP_BUF_BOTH);
    readInit(port, 3000);

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
        TThrReadPort thrPort;
        thrPort.msg = &msg;
        thrPort.port = port;
        pthread_t ntid;
        int err;
        err = pthread_create(&ntid, NULL, thr_func, &thrPort);
        if (err != 0)
        {
            printf("can't create thread:%s \r\n", strerror(err));
            exit(-1);
        }
        else
        {
            printf("create thread success:%d \r\n", err);
        }
        int msgsz = sizeof(TMsgBody);
        while (1)
        {
            int rev_result = msgrcv(qhandle, &msg, msgsz, 0, 0);
            if (errno == EEXIST)
            {
                printf("Exists %d \r\n", EEXIST);
            }
            if (strcmp(msg.content, "quit") == 4)
            {
                break;
            }
            if (strlen(msg.content) >= 1)
            {
                sendAT(port, msg.content);
                // sendAT(port, "AT+CWMODE=?");
                readInit(port, 0);
            }
        }
    }

    sp_free_port_list(ports);

    return 0;
}
