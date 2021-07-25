#include "http.h"

int serv_sock;

int main()
{
    //创建套接字
    serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    //将套接字和IP、端口绑定
    {
        struct sockaddr_in serv_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));                          //每个字节都用0填充
        serv_addr.sin_family = AF_INET;                                    //使用IPv4地址
        serv_addr.sin_addr.s_addr = inet_addr(HOST_NAME);                  //具体的IP地址
        serv_addr.sin_port = htons(PORT);                                  //端口
        bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)); //需要强制转化为sockaddr类型
    }

    //进入监听状态，等待用户发起请求
    if (-1 == listen(serv_sock, 20))
        printf("listen error!\n");

    //接收客户端请求
    //创建客户端套接字
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_size = sizeof(clnt_addr);
    int clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_addr, &clnt_addr_size);

    //接收客户端发送数据
    unsigned char buffer[BUF_MAX_SIZE] = {0};
    unsigned char responseBuffer[BUF_MAX_SIZE] = "HTTP/1.1 200 OK\n\r";
    while (0 == read(clnt_sock, buffer, sizeof(buffer) - 1))
    {
        continue;
    }
    printf("buffer length:%d\n", strlen((char *)buffer));
    //printf("%s", buffer);

    //得到方法， 路由
    char Method[32] = {0};
    char Router[256] = {0};
    int offset = 0;

    int len = 0;
    while (buffer[offset] != ' ' && buffer[offset] != '\n' && buffer[offset] != EOF)
    {
        offset++;
        len++;
    }
    printf("len = %d\n", len);
    memcpy(Method, buffer, (size_t)len);
    printf("method:%s\n", Method);
    if (strcmp(Method, "GET") != 0)
    {
        //无效访问
        printf("Bad Request!\n");
        statusBadRequest(clnt_sock);
        return 0; //暂时先return
    }

    statusOK(clnt_sock, "ok");
    // printf("send pong\n");

    //关闭套接字
    close(clnt_sock);
    close(serv_sock);

    return 0;
}

int write_socket(int fd, char *msg, int size)
{
    int bytes_sent = 0;
    int total_sent = 0;

    while (size > 0)
    {
        bytes_sent = write(fd, msg, size);

        if (bytes_sent > 0)
        {
            msg += bytes_sent;
            size -= bytes_sent;
            total_sent += bytes_sent;
        }
    }

    if (bytes_sent >= 0) //最后一次发送没有问题
    {
        return total_sent; //返回总共发送字节数
    }
    return -1; //有问题
}

//只写了200 OK 和 400 BadRequest
int statusOK(int clntsock, char *body)
{
    // 200 OK
    // 原来可以一次一次发
    char buffer[BUF_MAX_SIZE];
    sprintf(buffer, "HTTP/1.1 200 OK\r\n");
    write_socket(clntsock, buffer, strlen(buffer));
    sprintf(buffer, SERVER_STRING);
    write_socket(clntsock, buffer, strlen(buffer));
    sprintf(buffer, "Content-Type: text/plain\r\n");
    write_socket(clntsock, buffer, strlen(buffer));
    sprintf(buffer, "Content-Length: %d\r\n", strlen(body));
    write_socket(clntsock, buffer, strlen(buffer));
    write_socket(clntsock, "\r\n", strlen("\r\n"));
    //body
    write_socket(clntsock, body, strlen(body));
}

int statusBadRequest(int clntsock)
{
    // 400 Error
    char buffer[BUF_MAX_SIZE];
    char body[BUF_MAX_SIZE];
    sprintf(buffer, "HTTP/1.1 400 Bad Request\r\n");
    write_socket(clntsock, buffer, strlen(buffer));
    sprintf(buffer, SERVER_STRING);
    write_socket(clntsock, buffer, strlen(buffer));
    sprintf(buffer, "Content-Type: text/html\r\n");
    write_socket(clntsock, buffer, strlen(buffer));

    sprintf(body, "<HTML><HEAD><TITLE>Bad Request</TITLE></HEAD>\r\n");
    sprintf(body + strlen(body), "<BODY><P>The request cannot be fulfilled due to bad syntax.</P></BODY></HTML>\r\n");

    sprintf(buffer, "Content-Length: %d\r\n", strlen(body));
    write_socket(clntsock, buffer, strlen(buffer));
    write_socket(clntsock, "\r\n", strlen("\r\n"));
    // body
    write_socket(clntsock, body, strlen(body));
}