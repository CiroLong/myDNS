#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main()
{
    //创建套接字
    int serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    //将套接字和IP、端口绑定
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));                          //每个字节都用0填充
    serv_addr.sin_family = AF_INET;                                    //使用IPv4地址
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");                //具体的IP地址
    serv_addr.sin_port = htons(8080);                                  //端口
    bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)); //需要强制转化为sockaddr类型

    //进入监听状态，等待用户发起请求
    if (-1 == listen(serv_sock, 20))
        printf("listen error!\n");

    //接收客户端请求
    //创建客户端套接字
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_size = sizeof(clnt_addr);
    int clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_addr, &clnt_addr_size);

    //向客户端发送数据
    char rvbuffer[40] = {0};
    while (1)
    {
        while (0 == read(clnt_sock, rvbuffer, sizeof(rvbuffer) - 1))
        {
            continue;
        }
        printf("msg from client: %s\n", rvbuffer);
        write(clnt_sock, "pong", sizeof("pong"));
        printf("send pong\n");
    }

    //关闭套接字
    close(clnt_sock);
    close(serv_sock);

    return 0;
}
