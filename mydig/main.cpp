#include "cstdio"
#include "unistd.h"  //for getopt()
#include "stdlib.h"
#include "cstring"


#define DNSHEADER (12)                  //12bytes
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
#define TYPE_A      (0x0001)
#define CLASS_IN    (0x0001)



































//from "unistd.h"
extern int optind,opterr,optopt;
//index, 
//(>>error, 0 for no),
//optopt for these strings not in options 
extern char* optarg;//to save the parsmeter of each option

//some 

char DefaultDnsServerIp[] = "192.168.1.1";//Default DnsServer
char *DnsServerIP = DefaultDnsServerIp;

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
                //
                break;
            case 't'://type: "a" or "ns"
                printf("HAVE option: -t\n");
                printf("The argument of -t is %s\n\n", optarg);
                //
                break;
            case '?':
                printf("Unknown option: %c\n",(char)optopt);
                break;
            }
    }
    //the last argv[optind] is host
    printf("the host is %s\n",argv[optind]);
    printf("now the dnsserverip is %s\n",DnsServerIP);
 
 
    //dig RR of TLD



    //via parsmeter server to the assigned server



    //-r recursion or non-recursion

    
    

    return 0;
}