#include "stdio.h"
#include "unistd.h" //for getopt()
#include "stdlib.h"
#include "string.h"
#include "time.h"
#include "arpa/inet.h"
#include "sys/socket.h"
#include "netinet/in.h"

#define DNS_HEADER (12) //12bytes
#define TTL_SIZE (4)    //size of TTL

//Flags and Codes
#define QUERY_FLAG (0)                    //QRFLAG, 0 for query
#define RESPONSE_FLAG (1 << 15)           //1 for response
#define OPCODE_STANDARD_QUERY (0)         //opcode 0 for standard query
#define OPCODE_INVERSE_QUERY (1 << 11)    //1 for inverse query
#define OPCODE_STATUS_REQUEST (2 << 11)   //2 for server status request
#define AUTHORITATIVE_FLAG (1 << 10)      //Authoritative Answer Flag
#define TRUNCATION_FLAG (1 << 9)          //Truncated Flag to truncate UDP to 512
#define RECURSION_DESIRED_FLAG (1 << 8)   //Recursion desired
#define RECURSION_AVAILABLE_FLAG (1 << 7) //Recursion available
//3 zero zone
#define RESPONSE_NO_ERROR (0) //last 4 bits for response code
#define RESPONSE_FORMAT_ERROR (1)
#define RESPONSE_SERVER_FAILURE (2)
#define RESPONSE_NAME_ERROR (3)
#define RESPONSE_NOT_IMPLEMENTED (4)
#define RESPONSE_REFUSED (5)
#define TYPE_A (0x0001) //default query type ----A
#define TYPE_NS (0x0010)
#define CLASS_IN (0x0001)
//port number of dns server listening on
#define DNS_PORT (53)

typedef struct Header_DNS
{
    u_int16_t RequestID = 0;
    u_int16_t Code_And_Flag = 0;
    u_int16_t Question_Count = 0;
    u_int16_t Answer_Record_Count = 0;
    u_int16_t Authority_Record_Count = 0;
    u_int16_t Addition_Record_Count = 0;
} Header_DNS;

int buildRequest(u_int8_t RequestBuffer[128], char *hostname);
ssize_t sendDnsMassage(u_int8_t RequestBuffer[128], int offset);