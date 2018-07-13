/*=========================================================
 //
 // File: Socket.cpp  v200
 //
 // Created by Ned Phipps, Oct-2004
 //
 =============================================================================*/

#include <stdio.h>
#include <errno.h>

#include "cortex_socket.h"
#include "cortex_intern.h"


int ProcessSocketError();

int setReceiveBufferSize(SOCKET sockfd, int size);

/*===========================================================
 //
 //  Socket_CreateForBroadcasting
 //
 //  Description: creates and initializes a broadcast socket
 //
 //  Parameters:
 //      IP_Address:  4 byte ethernet address
 //      uPort:       port number on this machine.
 //
 //  Returns:
 //      SOCKET:  socket id, or
 //               -1 if error occurred
 //
 //  Notes:
 //
 //      The socket is bound to the specified IP address
 //      and the port number.
 //
 //      The socket can be used to send to one destination
 //      or to broadcast from.
 //
 //------------------------------------------------------------*/
SOCKET Socket_CreateForBroadcasting(unsigned long IP_Address, //in network byte order, not used
                                    unsigned short uPort)
{
  struct sockaddr_in my_addr; // socket address information
  static unsigned long ivalue;
//  static unsigned long bFlag;
//  int nlengthofsztemp = 64; // neede by getsockopt();
  SOCKET sockfd;

  // Create a socket.
  // param 1 = address family, AF_INET is Internet, incl. UDP & TCP
  // param 2 = stream or datagram, param 3 = protocol, 0 = TCP. In all examples
  // I've seen, 0 is the value used, but I'm not sure why other values aren't used,
  // such as IPPROTO_TCP   6  for tcp
  // and     IPPROTO_UDP  17  for user datagram protocol
  if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
    LogMessage(VL_Error, "%s: Socket creation failed", __PRETTY_FUNCTION__);
    ProcessSocketError();
    return -1;
  }

  // We bind the socket to our local NIC card.

  memset(&my_addr, 0, sizeof(my_addr));
  my_addr.sin_family = AF_INET;
  my_addr.sin_port = htons(uPort);

// Use any (or all?) of the local NIC cards.
  my_addr.sin_addr.s_addr = INADDR_ANY;
  //my_addr.sin_addr.s_addr = IP_Address;

#if 0 // Siggraph used this particular address for its second ethernet card
  my_addr.sin_addr.S_un.S_un_b.s_b1 = 10;
  my_addr.sin_addr.S_un.S_un_b.s_b2 = 1;
  my_addr.sin_addr.S_un.S_un_b.s_b3 = 1;
  my_addr.sin_addr.S_un.S_un_b.s_b4 = 199;
#endif

  // When a socket is first created, it exists in a name space (address family), but
  // it has no name assigned to it. So we use the bind() function to assign a
  // local name (local address) to the unnamed socket.  Since our application
  // does not care what local address is assigned, we use the value INADDR_ANY.
  // In our application we specify a specific port number, but if we had assigned
  // a port value of 0, it would mean to let bind() choose any unused port
  // with a value between 1024 and 5000.

  if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr))
      == SOCKET_ERROR) {
    LogMessage(VL_Error, "%s: bind() failed, address %s, port %d", __PRETTY_FUNCTION__, inet_ntoa(my_addr.sin_addr), ntohs(my_addr.sin_port));
    ProcessSocketError();
    close(sockfd);
    return -1;
  }

  /*
   // When a socket is created, the default is blocking mode (nonblocking mode is
   // disabled. Use ioctlsocket() to set the I/O mode to be nonblocking.
   bFlag = 1;      // a nonzero value enables nonblocking mode.
   if (ioctlsocket(sockfd, FIONBIO, (unsigned long *)&bFlag) == SOCKET_ERROR)
   {
   //printf("\nServer: ioctl(FIONBIO) failed with error = %d.\n", WSAGetLastError());
   ProcessSocketError();
   closesocket(sockfd);
   WSACleanup();
   return(-1);
   }
   */

  // Set the socket to broadcast mode.
  ivalue = 1;
  if (setsockopt(sockfd, SOL_SOCKET, // protocol level
                 SO_BROADCAST, (char *) &ivalue, // nonzero value enables the boolean SO_BROADCAST option
                 sizeof(ivalue)) == SOCKET_ERROR) {
    LogMessage(VL_Error, "%s: setting socket to broadcast mode failed");
    ProcessSocketError();
    close(sockfd);
    return -1;
  }

#if 0
  char str[256];

  // Diagnostic: find out the maximum size of the socket send buffer.
  int four = 4;
  if (getsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (char *)&ivalue, &four)
      == SOCKET_ERROR)
  {
    ProcessSocketError();
    closesocket(sockfd);
    return -1;
  }
  sprintf(str, "Max Send Buffer Size = %d", ivalue);
  PopupNotice(str);

  // Diagnostic: find out the maximum size of the socket receive buffer.
  if (getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (char *)&ivalue, &four)
      == SOCKET_ERROR)
  {
    ProcessSocketError();
    closesocket(sockfd);
    return -1;
  }
  sprintf(str, "Max Receive Buffer Size = %d", ivalue);
  PopupNotice(str);
#endif

  setReceiveBufferSize(sockfd, 0x100000);

  return sockfd;
}

int ConvertToIPAddress(const char *szNameOrAddress, struct in_addr *Address)
{
  struct hostent* haddr = NULL;
  //    char s[256];

  if (szNameOrAddress == NULL || szNameOrAddress[0] == 0) {
    Address->s_addr = INADDR_BROADCAST;
    return 0;
  }

  /* find Internet address of host */

  if (!isalpha(szNameOrAddress[0])) {
    /* Convert a.b.c.d address to a usable one */
    unsigned long addr = inet_addr(szNameOrAddress);
    haddr = gethostbyaddr((char *) &addr, 4, AF_INET);
    if (haddr != NULL) {
      Address->s_addr = addr;
      return 0;
    }
    return -1;
  }

  /* server address is a name */
  haddr = gethostbyname(szNameOrAddress);

  if (haddr == NULL) {
    return -1;
  }

  if (haddr->h_length != sizeof(in_addr)) {
    //        LOGMESSAGE("SOCKET ERROR: address sizes don't match?!");
    return -1;
  }

  memcpy(
      Address,
      haddr->h_addr_list[0],
      sizeof(in_addr));

  return 0;
}

/***********************************************************

 Function: int Broadcast(char *Buffer, int nBytes)

 Description: broadcast the message contained in the first
 parameter to all cameras on the network

 Parameters:
 char * Buffer	pointer to buffer containing the
 ASCII message
 int nBytes		number of bytes in the message

 Return Values:
 numbytes if successful
 -1 if unsuccessful

 *************************************************************/
int Broadcast(SOCKET sockfd, unsigned short uPort, const char *Buffer, int nBytes)
{
  struct sockaddr_in their_addr; // client connector's address information
  int numbytes;

  //if (sockfd <= 0) return 0;
  if (sockfd == -1)
    return 0;

  their_addr.sin_family = AF_INET; // host byte order
  their_addr.sin_port = htons(uPort); // short, network byte order
  their_addr.sin_addr.s_addr = INADDR_BROADCAST;

#if 0
  their_addr.sin_addr.S_un.S_un_b.s_b1 = 10;
  their_addr.sin_addr.S_un.S_un_b.s_b2 = 1;
  their_addr.sin_addr.S_un.S_un_b.s_b3 = 2;
  their_addr.sin_addr.S_un.S_un_b.s_b4 = 255;
#endif

  memset(&(their_addr.sin_zero), 0, 8); // zero the rest of the struct

  if ((numbytes = sendto(sockfd, Buffer, nBytes, 0,
                         (struct sockaddr *) &their_addr,
                         sizeof(struct sockaddr))) == SOCKET_ERROR) {
    //printf("\nServer: broadcast sendto() failed with error = %d.\n", WSAGetLastError());
    ProcessSocketError();
    return (-1);
  }

  return (numbytes);
} // end Broadcast()


//cMultiCast::cMultiCast(char *szMyAddress, int MyPort, char *szMultiCastAddress, int MultiCastPort)
SOCKET Socket_CreateLargeMultiCast(in_addr MyAddress, // not used
                                   unsigned short MyPort,
                                   in_addr MultiCastAddress // standard byte order (not network byte order)
                                   )
{
  SOCKET MySocket;
  int retval;

  // Get a datagram socket

  MySocket = socket(PF_INET, SOCK_DGRAM, 0);

  // "A socket must bind to an address before calling setsockopt"

  struct sockaddr_in my_addr;

  memset(&my_addr, 0, sizeof(my_addr));
  my_addr.sin_family = AF_INET;
  my_addr.sin_port = htons(MyPort);
  //my_addr.sin_addr.s_addr = inet_addr(szMyAddress); //INADDR_ANY;
  my_addr.sin_addr.s_addr = INADDR_ANY;//MyAddress.s_addr;

  if (bind(MySocket, (struct sockaddr *) &my_addr, sizeof(struct sockaddr))
      == SOCKET_ERROR) {
    LogMessage(VL_Error, "%s: bind() failed, address %s, port %d", __PRETTY_FUNCTION__, inet_ntoa(my_addr.sin_addr), ntohs(my_addr.sin_port));
    ProcessSocketError();
    close(MySocket);
    return -1;
  }

  // Join the multicast group
  // Note: This is NOT necessary for the host program.

  struct ip_mreqn stMreq;

  //stMreq.imr_multiaddr.s_addr = inet_addr(szMultiCastAddress);
  //stMreq.imr_interface.s_addr = inet_addr(szMyAddress);
  stMreq.imr_multiaddr.s_addr = htonl(MultiCastAddress.s_addr);
//  stMreq.imr_interface = MyAddress;
  stMreq.imr_address.s_addr = INADDR_ANY;
  stMreq.imr_ifindex = 0;
  retval = setsockopt(MySocket, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                      (char *) &stMreq, sizeof(ip_mreqn));
  if (retval == SOCKET_ERROR) {
    LogMessage(VL_Error, "%s: Joining the multicast group failed", __PRETTY_FUNCTION__);
    ProcessSocketError();
    close(MySocket);
    return -1;
  }

  // Set TTL to traverse multiple routers (test)
  // The default is 1 which means direct connections only.  No routers.

  int iTmp;

  iTmp = 2;
  retval = setsockopt(MySocket, IPPROTO_IP, IP_MULTICAST_TTL, (char *) &iTmp,
                      sizeof(iTmp));
  if (retval == SOCKET_ERROR) {
    ProcessSocketError();
  }

  setReceiveBufferSize(MySocket, 0x100000);

  // Disable loopback
#if 0
  iTmp = 0;
  retval = setsockopt(MySocket, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&iTmp, sizeof(iTmp));
  if (retval == SOCKET_ERROR)
  {
    ProcessSocketError();
  }
#endif

  // If you want the MultiCast to go out through more than one ethernet card...
#if 0
  unsigned long addr = inet_addr(IP_Address);
  retval = setsockopt(MySocket, IPPROTO_IP, IP_MULTICAST_IF, (char *)&addr, sizeof(addr));
#endif

  // Send and Receive examples

#if 0
  // Configure an address for sendto

  memset(&m_stTo, 0, sizeof(sockaddr_in));
  m_stTo.sin_family = AF_INET;
  m_stTo.sin_addr.s_addr = inet_addr(szMultiCastAddress);
  m_stTo.sin_port = htons(MultiCastPort);
#endif

#if 0
  retval = sendto(MySocket, "multicast message", 20, 0, (struct sockaddr *)&m_stTo, sizeof(m_stTo));

  // Receiving message example

  len = sizeof(stFrom);
  recvfrom(MySocket, achIn, BUFSIZE, 0, (struct sockaddr *)&stFrom, &len);

  // Exit multicast group

  //stMreq.imr_multiaddr.s_addr = inet_addr("224,1,2,4");
  //stMreq.imr_interface.s_addr = netstatic[0].n_ipaddr;

  retval = setsockopt(MySocket, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char *)&stMreq, sizeof(stMreq));

  retval = close(MySocket);
#endif

  return MySocket;
}

//////////////////////////////////////

int ProcessSocketError()
{
  LogMessage(VL_Error, "Socket error: %s", strerror(errno));
  return 0;
}

int Cortex_GetAllOfMyAddresses(unsigned long Addresses[], int nMax)
{
  struct hostent *haddr;
  char szMyName[128];
  unsigned long NameLength = 128;
  int nAddresses;

  if (gethostname(szMyName, NameLength) == -1) {
    LogMessage(VL_Error, "Failed to get my hostname");
    return -1;
  }
  if (!(haddr = gethostbyname(szMyName))) {
    LogMessage(VL_Error, "Failed to get my addresses");
    return -1;
  }

  for (nAddresses = 0; nAddresses < nMax; nAddresses++) {
    if (haddr->h_addr_list[nAddresses] == NULL) {
      break;
    }

    memcpy(&Addresses[nAddresses], haddr->h_addr_list[nAddresses], 4);
  }

  return nAddresses;
}

int GetHostByAddr(unsigned char Address[4], char *szName)
{
    hostent* pHostEnt;

    pHostEnt = gethostbyaddr((char*)Address, 4, PF_INET);

    if (pHostEnt == NULL)
    {
        LogMessage(VL_Error, "Unable to get CortexHost name information");
        return RC_NetworkError;
    }
    strcpy(szName, pHostEnt->h_name);

    return RC_Okay;
}

int setReceiveBufferSize(SOCKET sockfd, int size)
{
  // Large 1MB Buffer for receiving so we don't lose data.
  int optval = size;
  socklen_t optval_size = sizeof(int);
  if (setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (char *) &optval,
             optval_size) == SOCKET_ERROR) {
    LogMessage(VL_Error, "%s: setting receive buffer size failed");
  }
  if (getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (char *) &optval,
             &optval_size) == SOCKET_ERROR) {
    LogMessage(VL_Error, "%s: getting receive buffer size failed");
  }
  if (optval != size) {
    LogMessage(VL_Warning,
               "%s, ReceiveBuffer size is %d, but set %d", __PRETTY_FUNCTION__, optval, size);
    return RC_Unrecognized;
  }
  return RC_Okay;
}
