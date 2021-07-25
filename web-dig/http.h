#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "../mydig/mydig.cpp"
#include "../mydig/head.h"

#define BUF_MAX_SIZE 8096

#define TRUE 1
#define FALSE 0

#define PORT 8080
#define HOST_NAME "127.0.0.1"
#define MAX_CONNECTIONS 5
#define MAX_TIMESTAMP_LENGTH 64
#define SERVER_STRING "Server: ciroHttpServer/1.0\r\n"

#define MAX_RETRIES 3 //反复尝试最大次数

typedef struct routerParameter
{
    char name[64];   //DefaultHostname
    char type[10];   //"a" 或者 "ns"
    char server[20]; //DefaultDnsServerIp;
    int recursion;   //1为递归, 0 为非递归
} routerParameter;

int statusOK(int clntsock, char *body);
int statusBadRequest(int clnt_sock);
int write_socket(int fd, char *msg, int size);
int read_socket(int fd, char *buffer, int size);
int is_valid_fname(char *fname);
int dnsHandleFunction();