#include "http.h"

int serv_sock;
int clnt_sock;
int child; //子进程标识符

struct sockaddr_in serv_addr;
struct sockaddr_in clnt_addr;

// Value specific to client connections
int keep_alive = TRUE; // Default in HTTP/1.1
int content_length = -1;
int cookie = FALSE;
int header_err_flag = FALSE;
struct tm *if_modified_since;
int time_is_valid = TRUE;
char *content = NULL;
int not_eng = FALSE;
int acceptable_text = TRUE;
int acceptable_charset = TRUE;
int acceptable_encoding = TRUE;
char from_email[512];
char user_agent[512];

int main()
{
    //创建套接字
    serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serv_sock < 0)
    {
        printf("create socket error\n");
        exit(0);
    }
    //将套接字和IP、端口绑定
    {
        memset(&serv_addr, 0, sizeof(serv_addr));                                  //每个字节都用0填充
        serv_addr.sin_family = AF_INET;                                            //使用IPv4地址
        serv_addr.sin_addr.s_addr = inet_addr(HOST_NAME);                          //具体的IP地址
        serv_addr.sin_port = htons(PORT);                                          //端口
        if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) //需要强制转化为sockaddr类型
        {
            printf("bind socket error\n");
            close(serv_sock);
            exit(0);
        }

        //进入监听状态，等待用户发起请求
        if (-1 == listen(serv_sock, MAX_CONNECTIONS))
            printf("listen error!\n");
    }

    //接收客户端发送数据
    //unsigned char buffer[BUF_MAX_SIZE] = {0};
    //unsigned char responseBuffer[BUF_MAX_SIZE] = {0};
    //while (0 == read(clnt_sock, buffer, sizeof(buffer) - 1))
    //{
    //   continue;
    //}
    //printf("buffer length:%d\n", strlen((char *)buffer));

    //改用函数， 且不一次性读取

    while (1)
    {
        memset(&clnt_addr, 0, sizeof(clnt_addr)); //清空

        //接收客户端请求
        //创建客户端套接字
        socklen_t clnt_addr_size = sizeof(clnt_addr);
        clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_addr, &clnt_addr_size);
        if (clnt_sock < 0)
        {
            printf("Can't accept the request!");
            close(serv_sock);
            exit(0);
        }

        //在一个子进程中对其响应
        child = fork();
        if (child == 0) //用fork()实现并发
        {
            while (keep_alive)
            {
                content_length = -1;
                cookie = FALSE;
                header_err_flag = FALSE;
                if_modified_since = NULL;
                time_is_valid = TRUE;
                content = NULL;
                not_eng = FALSE;
                acceptable_text = TRUE;
                acceptable_charset = TRUE;
                acceptable_encoding = TRUE;

                dnsHandleFunction();
                if (content != NULL)
                    free(content);
            }
            exit(0);
        }
    }

    close(serv_sock);

    return 0;
}

int dnsHandleFunction()
{
    char buffer[BUF_MAX_SIZE] = {0};
    int buffer_len = 0;
    buffer_len = read(clnt_sock, buffer, BUF_MAX_SIZE); //read_socket()出bug力!
    // 得到方法， 路由
    char method[256] = {0};
    char url[256] = {0};
    char version[256] = {0};

    int offset = 0;

    int len = 0;
    while (buffer[offset] != ' ' && buffer[offset] != '\n' && buffer[offset] != EOF)
    {
        offset++;
        len++;
    }
    offset++; //滤过空格
    memcpy(method, buffer, (size_t)len);
    printf("method:%s\n", method);
    if (strcmp(method, "GET") != 0)
    {
        //无效访问
        printf("Bad Request!\n");
        statusBadRequest(clnt_sock);
        exit(0);
    }

    int i = offset;
    len = 0;
    while (buffer[offset] != ' ' && buffer[offset] != '\n' && buffer[offset] != EOF)
    {
        offset++;
        len++;
    }
    offset++; //滤过空格,之后本来得拿 HTTP 版本
    memcpy(url, buffer + i, (size_t)len);
    fprintf(stdout, "url = %s\n", url);

    //解析url， //还没时间做
    //...
    //路由就不管了
    //直接拿参数
    routerParameter *rParameter = (routerParameter *)malloc(sizeof(routerParameter));
    memset(rParameter, 0, sizeof(rParameter));
    //strcpy(rParameter->name, "hustunique.com\0");
    //strcpy(rParameter->server, "192.168.1.1");
    //strcpy(rParameter->type, "a"); //"ns"
    rParameter->recursion = 1;
    {
        char key[64] = {0};
        char value[64] = {0};
        if (!is_valid_fname(url))
        {
            statusBadRequest(clnt_sock);
            close(clnt_sock); //终于意识到要写个退出函数了，暂时先这样
            exit(0);
        }

        char *start = strstr(url, "?");
        char *mid = strstr(start, "=");
        char *end = strstr(mid, "&");
        if (start != NULL)
        {
            if (mid == NULL)
            {
                statusBadRequest(clnt_sock);
                close(clnt_sock);
                exit(0);
            }
            while (end != NULL)
            {
                memcpy(key, start + 1, mid - start - 1);
                memcpy(value, mid + 1, end - mid - 1);
                if (strcmp(key, "name") == 0)
                {
                    strcpy(rParameter->name, value);
                }
                else if (strcmp(key, "type") == 0)
                {
                    strcpy(rParameter->type, value);
                }
                else if (strcmp(key, "server") == 0)
                {
                    strcpy(rParameter->server, value);
                }
                else if (strcmp(key, "recursion") == 0)
                {
                    rParameter->recursion = atoi(value);
                }

                memset(key, 0, sizeof(key));
                memset(value, 0, sizeof(value));
                start = end;
                mid = strstr(start, "=");
                end = strstr(mid, "&");
                if (mid == NULL)
                {
                    statusBadRequest(clnt_sock);
                    close(clnt_sock);
                    exit(0);
                }
            }
            end = url + strlen(url);
            memcpy(key, start + 1, mid - start - 1);
            memcpy(value, mid + 1, end - mid - 1);
            if (strcmp(key, "name") == 0)
            {
                strcpy(rParameter->name, value);
            }
            else if (strcmp(key, "type") == 0)
            {
                strcpy(rParameter->type, value);
            }
            else if (strcmp(key, "server") == 0)
            {
                strcpy(rParameter->server, value);
            }
            else if (strcmp(key, "recursion") == 0)
            {
                rParameter->recursion = atoi(value);
            }

            //debug
            printf("name:%s\nserver:%s\n", rParameter->name, rParameter->server);
        }
    }
    if (rParameter->name == NULL)
    {
        statusBadRequest(clnt_sock);
        close(clnt_sock);
        exit(0);
    }
    //调用mydig

    char *argv[10] = {0};
    int args = 0;
    argv[args++] = "./mydig";
    argv[args++] = rParameter->name;
    if (rParameter->recursion == 1)
    {
        argv[args++] = "-r";
    }
    if (*(rParameter->server) != '\0')
    {
        argv[args++] = "-s";
        argv[args++] = rParameter->server;
    }
    if (*(rParameter->type) != '\0')
    {
        argv[args++] = "-t";
        argv[args++] = rParameter->type;
    }
    OutPut *msg_mtdig = myDig(args, argv);
    char msg[BUF_MAX_SIZE] = {0};
    sprintf(msg, "{\"hostname\":\"%s\",\"ip\":\"%s\",\"nameserver\":\"%s\",}", msg_mtdig->hostname, msg_mtdig->Ip, msg_mtdig->NameServer);

    statusOK(clnt_sock, msg);
    //放弃headers,得写个函数解析，呜呜
    //关闭套接字
    close(clnt_sock);
    exit(0);
}

int write_socket(int fd, char *msg, int size)
{
    int bytes_sent = 0;
    int total_sent = 0;
    int retries = 0;

    while (retries < MAX_RETRIES && size > 0)
    {
        bytes_sent = write(fd, msg, size);

        if (bytes_sent > 0)
        {
            msg += bytes_sent;
            size -= bytes_sent;
            total_sent += bytes_sent;
        }
        else
        {
            retries++;
        }
    }

    if (bytes_sent >= 0) //最后一次发送没有问题
    {
        return total_sent; //返回总共发送字节数
    }
    return -1; //有问题
}

int read_socket(int fd, char *buffer, int size)
{
    int bytes_recvd = 0;
    int retries = 0;
    int total_recvd = 0;

    while (retries < MAX_RETRIES && size > 0 && strstr(buffer, ">") == NULL)
    {
        bytes_recvd = read(fd, buffer, size);

        if (bytes_recvd > 0)
        {
            buffer += bytes_recvd;
            size -= bytes_recvd;
            total_recvd += bytes_recvd;
        }
        else
        {
            retries++;
        }
    }

    if (bytes_recvd >= 0)
    {
        // 与wirte相同
        return total_recvd;
    }
    return -1;
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

    return 0;
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
    return 0;
}

int is_valid_fname(char *fname)
{
    char *it = fname;
    while (TRUE)
    {
        if (strncmp(it, "..", 2) == 0)
        {
            return FALSE;
        }
        it = strchr(it, '/');
        if (it == NULL)
            break;
        it++;
    }
    return TRUE;
}