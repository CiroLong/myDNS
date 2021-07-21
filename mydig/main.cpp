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
char *hostname; //查询的域名

int main(int argc, char *argv[])
{

    parse(argc, argv);

    u_int8_t RequestBuffer[128];
    int offset;
    offset = buildRequest(RequestBuffer, hostname);

    //send to dns server
    sendDnsMassage(RequestBuffer, offset);

    //parse the response

    return 0;
}

int parse(int argc, char *argv[])
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
    hostname = argv[optind];
    printf("now the dnsserverip is %s\n", DnsServerIP);
}

int buildRequest(u_int8_t RequestBuffer[128], char *hostname)
{
    //build request
    u_int16_t twoByteBuffer;
    int offset = 0;
    memset(RequestBuffer, 0, sizeof(RequestBuffer));

    //header
    Header_DNS header;
    srand(time(0));
    //ID
    header.RequestID = rand();
    //codes and flags
    if (Recursion_key)
        header.Code_And_Flag = htons(QUERY_FLAG | OPCODE_STANDARD_QUERY | RECURSION_DESIRED_FLAG);
    else
        header.Code_And_Flag = htons(QUERY_FLAG | OPCODE_STANDARD_QUERY);
    header.Question_Count = 1;
    memcpy(RequestBuffer + offset, (Header_DNS *)&header, sizeof(header));
    offset += sizeof(header);

    //build question
    char *start = hostname;
    char *end = start;
    u_int8_t len = 0;

    while (*end)
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

    return offset;
}

ssize_t sendDnsMassage(u_int8_t RequestBuffer[128], int offset)
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
    memset(&ser_addr, 0, sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;                     //IPv4
    ser_addr.sin_addr.s_addr = inet_addr(DnsServerIP); //将ip号转化为in_addr_t
    ser_addr.sin_port = htons(DNS_PORT);               //转化为网络字节序

    return sendto(udp_socket, RequestBuffer, (size_t)offset, 0, (struct sockaddr *)&ser_addr, sizeof(ser_addr));
}