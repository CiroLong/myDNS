#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "time.h"

int main()
{
    //创建套接字
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    //向服务器（特定的IP和端口）发起请求
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));           //每个字节都用0填充
    serv_addr.sin_family = AF_INET;                     //使用IPv4地址
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); //具体的IP地址
    serv_addr.sin_port = htons(8080);                   //端口
    connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    //发送ping
    char sdbuffer[40] = "ping";
    char rvbuffer[40] = {0};
    while (1)
    {
        sleep(1); //5seconds
        send(sock, sdbuffer, strlen(sdbuffer), 0);
        printf("send ping\n");

        read(sock, rvbuffer, sizeof(rvbuffer) - 1);
        printf("Message form server: %s\n", rvbuffer);
    }

    //关闭套接字
    close(sock);

    return 0;
}
