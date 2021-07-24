#include "head.h"

//from "unistd.h"
extern int optind, opterr, optopt;
//index,
//(>>error, 0 for no),
//optopt for these strings not in options
extern char *optarg; //to save the parsmeter of each option

//define some control parsmeter
char DefaultDnsServerIp[] = "192.168.1.1"; //Default DnsServer
char *DnsServerIP = DefaultDnsServerIp;
u_int16_t Type_requset = TYPE_A; //1 for A, 2 for NS
int Recursion_key = 0;           //default off,  1 for on

int udp_socket; //socket id
char DefaultHostname[] = "hustunique.com\0";
char *hostname = DefaultHostname; //查询的域名

int main(int argc, char *argv[])
{

    parseArgv(argc, argv);

    u_int8_t RequestBuffer[BUF_MAX_SIZE];
    u_int8_t ResponseBuffer[BUF_MAX_SIZE];
    int offset;

    offset = buildRequest(RequestBuffer, hostname); // 出了点小问题

    //send to dns server and receive the massage
    int recvsize = sendAndRecvDnsMassage(RequestBuffer, offset, ResponseBuffer);

    //parse the response
    OutPut *output;

    output = parseResponse(ResponseBuffer, recvsize);

    printf("hostname:%s\nIp:%s\nType:%d\nClass:%d\n", output->hostname, output->Ip, output->Type, output->Class);
    printf("Ns:%s\n", output->NameServer[0]);
    //先打印看看
    //printf("\n\nrecvsize = %d\n", recvsize);
    //for (int i = 0; i < recvsize; i++)
    //{
    //    printf("| %#x |", ((unsigned char *)ResponseBuffer)[i]);
    //}
    //printf("\n");
    return 0;
}

int parseArgv(int argc, char *argv[])
{
    //parse the argvs
    int ch;
    while ((ch = getopt(argc, argv, "s:4rt:")) != -1)
    {
        printf("optind: %d\n", optind);
        switch (ch)
        {
        case 's': //server
            printf("HAVE option: -s\n\n");
            printf("The argument of -s is %s\n\n", optarg);
            strcpy(DnsServerIP, optarg);
            printf("now the dnsserverip is %s\n", DnsServerIP);
            break;
        case 'r': //recursion on
            printf("HAVE option: -r\n");
            Recursion_key = 1; //recursion on
            break;
        case 't': //type: "a" or "ns"
            printf("HAVE option: -t\n");
            printf("The argument of -t is %s\n\n", optarg);

            if (strcmp(optarg, "ns") == 0)
            {
                Type_requset = TYPE_NS;
            }
            else if (strcmp(optarg, "a") == 0)
            {
                Type_requset = TYPE_A;
            }
            else
            {
                printf("Unsupported parameter!\n");
                exit(0);
            }
            //
            break;
        case '?':
            printf("Unknown option: %c\n", (char)optopt);
            exit(0);
            break;
        }
    }
    //the last argv[optind] is host
    printf("the host is %s\n", argv[optind]);
    hostname = strcat(argv[optind], "\0");
    printf("now the dnsserverip is %s\n", DnsServerIP);
    return 0;
}

int buildRequest(u_int8_t RequestBuffer[], char *hostname)
{
    //build request
    u_int16_t twoByteBuffer;
    int offset = 0;
    memset(RequestBuffer, 0, BUF_MAX_SIZE);
    //header
    Header_DNS *header = (Header_DNS *)malloc(sizeof(Header_DNS));
    memset(header, 0, sizeof(Header_DNS));
    srand(time(0));
    //ID
    header->RequestID = rand();
    //codes and flags
    if (Recursion_key)
        header->Code_And_Flag = htons(QUERY_FLAG | OPCODE_STANDARD_QUERY | RECURSION_DESIRED_FLAG);
    else
        header->Code_And_Flag = htons(QUERY_FLAG | OPCODE_STANDARD_QUERY);
    header->Question_Count = htons(1);
    memcpy(RequestBuffer + offset, (u_int8_t *)header, sizeof(Header_DNS));
    offset += sizeof(Header_DNS);

    //printf("offset = %d\n", offset); //头部没有问题
    //for (int i = 0; i < offset; i++)
    // {
    //    printf("%.2x |", ((unsigned char *)header)[i]);
    //}

    //build question
    char *start = hostname;
    char *end = start;
    u_int8_t len = 0;

    while (*end) //用于建立查询名 Name 字段 , some bug
    {
        end = start;
        while (*end && (*end != '.'))
        {
            end++;
        }

        if (end - start > 0)
        {
            len = end - start;
            memcpy(RequestBuffer + offset, (u_int8_t *)&len, sizeof(len));
            offset += sizeof(len);
            memcpy(RequestBuffer + offset, (u_int8_t *)start, len * sizeof(u_int8_t));
            offset += len * sizeof(u_int8_t);
        }
        start = end + 1;
    }
    len = 0;
    memcpy(RequestBuffer + offset, (u_int8_t *)&len, sizeof(len));
    offset += sizeof(len);
    //Type
    twoByteBuffer = htons(Type_requset);
    memcpy(RequestBuffer + offset, (u_int16_t *)&twoByteBuffer, sizeof(twoByteBuffer));
    offset += sizeof(twoByteBuffer);
    //Class
    twoByteBuffer = htons(CLASS_IN);
    memcpy(RequestBuffer + offset, (u_int16_t *)&twoByteBuffer, sizeof(twoByteBuffer));
    offset += sizeof(twoByteBuffer);
    //build question ok

    //printf("offset = %d\n", offset);//报文没有问题
    //for (int i = 0; i < offset; i++)
    //{
    //    printf("|%d |%c|%.2x|\n", ((unsigned char *)RequestBuffer)[i], ((unsigned char *)RequestBuffer)[i], ((unsigned char *)RequestBuffer)[i]);
    //}

    return offset;
}

int sendAndRecvDnsMassage(u_int8_t RequestBuffer[], int offset, u_int8_t ResponseBuffer[])
{
    //create socket
    udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (udp_socket < 0)
    {
        printf("create socket fail.\n");
        exit(0);
    }
    //为socket绑定参数
    struct sockaddr_in ser_addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    memset(&ser_addr, 0, sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;                     //IPv4, AF_INET6 for IPv6
    ser_addr.sin_addr.s_addr = inet_addr(DnsServerIP); //将ip号转化为in_addr_t
    ser_addr.sin_port = htons(DNS_PORT);               //转化为网络字节序

    memset(ResponseBuffer, 0, BUF_MAX_SIZE);
    if (-1 == sendto(udp_socket, RequestBuffer, (size_t)offset, 0, (struct sockaddr *)&ser_addr, addr_len))
    {
        printf("send message error");
        exit(0);
    }

    int recvSize = recvfrom(udp_socket, ResponseBuffer, BUF_MAX_SIZE, 0, (struct sockaddr *)&ser_addr, &addr_len);
    if (recvSize == -1)
    {
        printf("recvice message error");
        exit(0);
    }

    close(udp_socket);
    return recvSize;
}

OutPut *parseResponse(u_int8_t ResponseBuffer[], int recvsize)
{
    //先声明返回值
    OutPut *output = (OutPut *)malloc(sizeof(OutPut));
    memset(output, 0, sizeof(OutPut));

    Header_DNS *header = NULL;
    header = (Header_DNS *)ResponseBuffer;
    header->Question_Count = ntohs(header->Question_Count);
    header->Code_And_Flag = ntohs(header->Code_And_Flag);
    header->Answer_Record_Count = ntohs(header->Answer_Record_Count);
    header->Authority_Record_Count = ntohs(header->Authority_Record_Count);
    header->Addition_Record_Count = ntohs(header->Addition_Record_Count);

    if (header->Code_And_Flag & RESPONSE_FLAG)
        printf("it's a response\n");
    if (!(header->Code_And_Flag & OPCODE_STANDARD_QUERY))
        printf("it's a standard query\n");
    if (header->Code_And_Flag & AUTHORITATIVE_FLAG)
        printf("it's a authoritative answer.\n");
    if (header->Code_And_Flag & RECURSION_DESIRED_FLAG)
        printf("Recursion desired!\n");
    if (header->Code_And_Flag & RESPONSE_NO_ERROR)
    {
        printf("reponse error!\n");
        printf("error code: %d\n", header->Code_And_Flag & (0x07));
        exit(0);
    }
    else
    {
        printf("it's a right response\n");
    }

    //parse the address of question
    int offset = 0; //已读字节数
    offset += sizeof(Header_DNS);
    u_int8_t *start = &((u_int8_t *)ResponseBuffer)[offset];
    int len = 0;
    while (*start != 0)
    {
        if (!isalnum(*start))
            len = *start;
        for (int i = 0; i < len; i++)
        {
            start++;
            offset++;
            //printf("%c", *start);
        }
        //printf(".");
        start++;
        offset++;
    }
    offset++;
    //printf("\noffset = %d\n", offset);

    part_Query *part_query = (part_Query *)&(ResponseBuffer[offset]);
    part_query->Type = ntohs(part_query->Type);
    part_query->Class = ntohs(part_query->Class);
    offset += sizeof(part_Query);

    //解析 the RR
    u_int8_t *namepr = &((u_int8_t *)ResponseBuffer)[offset];
    if (*namepr & NAME_REDIRECTION) //c0 重定向
    {
        printf("Yes, Redirection!\n");
        u_int16_t *realNamePr = (u_int16_t *)&((ResponseBuffer)[offset]);
        *realNamePr = ntohs(*realNamePr); //唉,两字节就得从网络字节序转化为主机字节序
        int redirection = *realNamePr & 0x3fff;
        offset += sizeof(u_int16_t);

        //打印Name
        namepr = &((u_int8_t *)ResponseBuffer)[redirection];
        int len_name = 0;
        int host_pr = 0; //help sprintf
        while (*namepr != 0)
        {
            if (!isalnum(*namepr))
                len_name = *namepr;
            for (int i = 0; i < len_name; i++)
            {
                namepr++;
                redirection++;
                sprintf(output->hostname + host_pr++, "%c", *namepr);
            }
            sprintf(output->hostname + host_pr++, ".");
            namepr++;
            redirection++;
        }
        output->hostname[host_pr - 1] = '\0';
        redirection++;
        //printf("%s", output->hostname); //ok
    }
    else
    {
        //打印Name
        int len_name = 0;
        int host_pr = 0; //help sprintf
        while (*namepr != 0)
        {
            if (!isalnum(*namepr))
                len_name = *namepr;
            for (int i = 0; i < len_name; i++)
            {
                namepr++;
                offset++;
                sprintf(output->hostname + host_pr++, "%c", *namepr);
            }
            sprintf(output->hostname + host_pr++, ".");
            namepr++;
            offset++;
        }
        output->hostname[host_pr - 1] = '\0'; //"hustunique.com."去除最后的'.'
        offset++;
    }

    //只读第一条记录
    partof_Resource *RR = (partof_Resource *)&(ResponseBuffer[offset]);
    offset += sizeof(partof_Resource) - 2; //？为什么多了两个字节，人工减去，？？这什么bug
    RR->Type = ntohs(RR->Type);
    RR->Class = ntohs(RR->Class);
    RR->Time_to_Live = ntohl(RR->Time_to_Live); //4字节
    RR->Resource_Data_Length = ntohs(RR->Resource_Data_Length);
    //printf("%.2x | %.2x | %d | %.2x\n", RR->Type, RR->Class, RR->Time_to_Live, RR->Resource_Data_Length);ok

    output->Type = RR->Type;
    output->Class = RR->Class;

    if (RR->Type == TYPE_A) //仅支持IPv4地址
    {
        u_int32_t *ptr = (u_int32_t *)&(ResponseBuffer[offset]);
        offset += sizeof(u_int32_t);
        //*ptr = ntohl(*ptr);//使用inet_ntoa 函数不需要转换字节序
        in_addr *add_out = (in_addr *)malloc(sizeof(in_addr));
        add_out->s_addr = *ptr;
        strcpy(output->Ip, inet_ntoa(*add_out));
    }
    else if (RR->Type == TYPE_NS)
    {
        printf("ns\n");
        char *namepr = (char *)&(ResponseBuffer[offset]);
        int len_name = 0;
        int ns_pr = 0; //help sprintf
        while (*namepr != 0)
        {
            if (!isalnum(*namepr))
                len_name = *namepr;
            for (int i = 0; i < len_name; i++)
            {
                namepr++;
                offset++;
                sprintf(output->NameServer[0] + ns_pr++, "%c", *namepr);
            }
            sprintf(output->NameServer[0] + ns_pr++, ".");
            namepr++;
            offset++;
        }
        output->NameServer[0][ns_pr - 1] = '\0'; //"hustunique.com."去除最后的'.'
        offset++;
    }
    return output;
}