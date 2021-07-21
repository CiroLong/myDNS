#include "cstdio"
#include "unistd.h"  //for getopt()
#include "stdlib.h"
#include "cstring"
#include <time.h>
#include <arpa/inet.h>


#define DNS_HEADER (12)                  //12bytes
#define TTL_SIZE (4)                    //size of TTL

//Flags and Codes
#define QUERY_FLAG               (0)       //QRFLAG, 0 for query
#define RESPONSE_FLAG            (1<<15)   //1 for response
#define OPCODE_STANDARD_QUERY    (0)       //opcode 0 for standard query
#define OPCODE_INVERSE_QUERY     (1<<11)   //1 for inverse query
#define OPCODE_STATUS_REQUEST    (2<<11)   //2 for server status request
#define AUTHORITATIVE_FLAG       (1<<10)   //Authoritative Answer Flag
#define TRUNCATION_FLAG          (1<<9)    //Truncated Flag to truncate UDP to 512
#define RECURSION_DESIRED_FLAG   (1<<8)    //Recursion desired 
#define RECURSION_AVAILABLE_FLAG (1<<7)    //Recursion available
//3 zero zone
#define RESPONSE_NO_ERROR        (0)       //last 4 bits for response code
#define RESPONSE_FORMAT_ERROR    (1)
#define RESPONSE_SERVER_FAILURE  (2)
#define RESPONSE_NAME_ERROR      (3)       
#define RESPONSE_NOT_IMPLEMENTED (4)
#define RESPONSE_REFUSED         (5)
#define TYPE_A      (0x0001)                //default query type ----A
#define TYPE_NS     (0x0010)
#define CLASS_IN    (0x0001)
//port number of dns server listening on
#define DNS_PORT     (53)


typedef struct Header_DNS
{
    u_int16_t RequestID = 0;
    u_int16_t Code_And_Flag = 0;
    u_int16_t Question_Count = 0;
    u_int16_t Answer_Record_Count = 0;
    u_int16_t Authority_Record_Count = 0;
    u_int16_t Addition_Record_Count = 0;
}Header_DNS;

//from "unistd.h"
extern int optind,opterr,optopt;
//index, 
//(>>error, 0 for no),
//optopt for these strings not in options 
extern char* optarg;//to save the parsmeter of each option


//define some control parsmeter
char DefaultDnsServerIp[] = "192.168.1.1";  //Default DnsServer
char *DnsServerIP = DefaultDnsServerIp;
u_int16_t Type_requset = TYPE_A;                         //1 for A, 2 for NS
int Recursion_key = 0;                      //default off,  1 for on

int main(int argc, char *argv[]){
    //parse the argvs
    int ch;
    while ((ch = getopt(argc, argv, "s:4rt:")) != -1)
    {
        printf("optind: %d\n", optind);
        switch (ch) 
        {
            case 's'://server
                printf("HAVE option: -s\n\n");
                printf("The argument of -s is %s\n\n", optarg);
                strcpy(DnsServerIP,optarg);
                printf("now the dnsserverip is %s\n",DnsServerIP);
                break;
            case 'r'://recursion on
                printf("HAVE option: -r\n");
                Recursion_key = 1;//recursion on
                break;
            case 't'://type: "a" or "ns"
                printf("HAVE option: -t\n");
                printf("The argument of -t is %s\n\n", optarg);

                if(strcmp(optarg, "ns") == 0){
                    Type_requset = TYPE_NS;
                }else if ( strcmp(optarg, "a") == 0){
                    Type_requset = TYPE_A;
                }else{
                    printf("Unsupported parameter!\n");
                    exit(0);
                }
                //
                break;
            case '?':
                printf("Unknown option: %c\n",(char)optopt);
                exit(0);
                break;
            }
    }
    //the last argv[optind] is host
    printf("the host is %s\n",argv[optind]);
    printf("now the dnsserverip is %s\n",DnsServerIP);
 
 
    //dig RR of TLD



    //via parsmeter server to the assigned server



    //-r recursion or non-recursion

    



    //build request
    u_int8_t RequestBuffer[128];
    u_int16_t twoByteBuffer;
    int offset=0;
    memset(RequestBuffer,0,sizeof(RequestBuffer));

        //header
    Header_DNS header;
    srand(time(0));
            //ID
    header.RequestID = rand();
            //codes and flags
    if(Recursion_key)
            header.Code_And_Flag = htons(QUERY_FLAG | OPCODE_STANDARD_QUERY | RECURSION_DESIRED_FLAG);
    else    header.Code_And_Flag = htons(QUERY_FLAG | OPCODE_STANDARD_QUERY);
    header.Question_Count = 1;
    memcpy(RequestBuffer + offset, (Header_DNS*)&header, sizeof(header));
    offset += sizeof(header);

        //build question
    char *start = argv[optind];
    char *end = start;
    u_int8_t len = 0;

    while( *end){
        end = start;
        while ( *end && (*end != '.') ) {
            end++;
        }

        if ( end - start > 0) {
            len = end - start;
            memcpy(RequestBuffer + offset, (u_int8_t*)&len, sizeof(len));
            offset += sizeof(len);
            memcpy(RequestBuffer + offset, (u_int8_t*)start, len * sizeof(u_int8_t));
            offset += len * sizeof(u_int8_t);
        }
        start = end + 1;
    }
    len = 0;
    memcpy(RequestBuffer + offset, (u_int8_t*)&len, sizeof(len));
    offset += sizeof(len);
                //Type
    twoByteBuffer = htons(Type_requset);
    memcpy(RequestBuffer + offset, (u_int16_t*)&twoByteBuffer, sizeof(twoByteBuffer));
    offset += sizeof(twoByteBuffer);
                //Class
    twoByteBuffer = htons(CLASS_IN);
    memcpy(RequestBuffer + offset, (u_int16_t*)&twoByteBuffer, sizeof(twoByteBuffer));
    offset += sizeof(twoByteBuffer);
    //build question ok

    return 0;
}