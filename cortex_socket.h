#ifndef CORTEX_SOCKET_H
#define CORTEX_SOCKET_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <ctype.h>
#include <netdb.h>
#include <unistd.h>

typedef int SOCKET;
#define INVALID_SOCKET          -1
#define SOCKET_ERROR            -1

SOCKET Socket_CreateForBroadcasting(unsigned long IP_Address,
                                    unsigned short uPort);

int ConvertToIPAddress(const char *szNameOrAddress, struct in_addr *Address);

int Broadcast(SOCKET sockfd, unsigned short uPort, const char *Buffer, int nBytes);

SOCKET Socket_CreateLargeMultiCast(in_addr MyAddress, unsigned short MyPort,
                                   in_addr MultiCastAddress);

int Cortex_GetAllOfMyAddresses(unsigned long Addresses[], int nMax);

int GetHostByAddr(unsigned char Address[4], char *szName);

#endif
