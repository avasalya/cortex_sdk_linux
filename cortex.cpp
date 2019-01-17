/*=========================================================
 //
 // File: Cortex.cpp  v200
 //
 // Created by Ned Phipps, Oct-2004
 //
 =============================================================================*/

/*! \file Cortex.cpp
 This file implements the API for ethernet communication of data
 between Cortex and multiple client programs.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <ctype.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/errno.h>

#include "cortex.h" // Users will include this header
#include "m3x3.h"
#include "cortex_intern.h"
#include "cortex_socket.h"
#include "cortex_unpack.h"
//#include "cmetered.h"

const unsigned char MyVersionNumber[4] = { 4, 1, 3, 1 }; // ProgramID, Major, Minor, Bugfix


LOCAL int bInitialized = 0;
LOCAL sHostInfo HostInfo;
LOCAL int User_VerbosityLevel = VL_Warning;

LOCAL int SendToCortex(sPacket *Packet);
LOCAL void GetHostName_ASYNC();
LOCAL sBodyDefs* pNewBodyDefs = NULL;
LOCAL sPacket PacketOut;
LOCAL sPacket PacketIn_Frame;
LOCAL sPacket PacketIn;

//LOCAL char *pBodyDefBuffer = NULL; // Gets allocated the same size as the received packet.
LOCAL sFrameOfData LatestFrameOfData;
LOCAL sFrameOfData Polled_FrameOfData;

//LOCAL  unsigned short wMyPort = 1600;         // My outgoing port (Cortex auto-replies to me here)
LOCAL unsigned short wMyPort = 0; // Let the socket library find an available port
LOCAL unsigned short wCortexPort = 1002; // 1002; // Cortex is listening at this port
LOCAL unsigned short wMultiCastPort = 1001; // Cortex sends frames to this port and associated address
//LOCAL  unsigned short wMultiCastPort = 1511;  // Cortex sends frames to this port and associated address

//LOCAL  in_addr MyNicCardAddress={ 10,  1,  2,199};   // My local IP address
//LOCAL  in_addr MultiCastAddress={225,  1,  1,  1};   // Cortex sends frames to this address and associated port

// LOCAL in_addr MyNicCardAddress = { (10 << 24) + (1 << 16) + (2 << 8) + 199 }; // My local IP address
// LOCAL in_addr MultiCastAddress = { (225 << 24) + (1 << 16) + (1 << 8) + 1 }; // Cortex sends frames to this address and associated port

//LOCAL  in_addr CortexNicCardAddress={0,0,0,0};
//LOCAL  in_addr CortexNicCardAddress={255,255,255,255};

//LOCAL in_addr CortexNicCardAddress = { (255 << 24) + (255 << 16) + (255 << 8) + 255 };


/* /////////////////////////////  my addresses ///////////////////////// */


LOCAL in_addr MyNicCardAddress = { (10 << 24)  + (1 << 16)   + (1 << 8)   + 200 }; // My local IP address

LOCAL in_addr MultiCastAddress = {  3774939393 };//{ (225 << 24) + (1<< 16) + (1 << 8) + 1 }; // Cortex sends frames to this address and associated port

// // convert IP address to integer;
//http://www.aboutmyip.com/AboutMyXApp/IP2Integer.jsp?ipAddress=255.255.255.1

// LOCAL in_addr CortexNicCardAddress = { (10 << 24) + (1 << 16) + (1 << 8) + 190 };
LOCAL in_addr CortexNicCardAddress = { (10 << 24) + (1 << 16) + (1 << 8) + 190 };

/* /////////////////////////////  my addresses ///////////////////////// */


LOCAL sockaddr_in CortexAddr; // This gets filled out when Cortex replies.


LOCAL SOCKET CommandSocket = -1;
LOCAL SOCKET MultiCastReaderSocket = -1;
// For use with waiting for replies from Cortex.
LOCAL sem_t EH_CommandConfirmed;
LOCAL pthread_t CortexListenThread_ID;
LOCAL void* CortexListenThread_Func(void *);
LOCAL pthread_t ReadDataThread_ID;
LOCAL void* ReadDataThread_Func(void *);
LOCAL pthread_t GetHostNameThread_ID;

// === Logging ===

void Dummy_CB_ErrorMsgHandler(int iLevel, const char *szMessage)
{
}

void
    (*CB_ErrorMsgHandler)(int iLevel, const char *szMessage) = Dummy_CB_ErrorMsgHandler;

void LogMessage(int iLevel, const char *szMsg, ...)
{
  if (iLevel <= User_VerbosityLevel) {
    // create char* from input
    char* str_tmp = NULL;
    va_list arg_list;
    va_start(arg_list, szMsg);
    (void) vasprintf(&str_tmp, szMsg, arg_list);
    CB_ErrorMsgHandler(iLevel, str_tmp);
    free(str_tmp);
  }
}

//=============================================================================

LOCAL void Dummy_CB_DataHandler(sFrameOfData *FrameOfData)
{
}
GLOBAL void (*CB_DataHandler)(sFrameOfData *FrameOfData) = Dummy_CB_DataHandler;

//=============================================================================
#if 0 // muellerj: not used
LOCAL int Broadcast(sPacket *Packet)
{
  return Broadcast(CommandSocket, wCortexPort, (const char *) Packet, Packet->nBytes + 4);
}
#endif

void FoundHost()
{
  HostInfo.bFoundHost = 1;

  LogMessage(VL_Info,
             "AutoConnected to: %s Version %d.%d.%d at %d.%d.%d.%d (%s)",
             HostInfo.szHostProgramName, HostInfo.HostProgramVersion[1],
             HostInfo.HostProgramVersion[2], HostInfo.HostProgramVersion[3],

             HostInfo.HostMachineAddress[0], HostInfo.HostMachineAddress[1],
             HostInfo.HostMachineAddress[2], HostInfo.HostMachineAddress[3],
             HostInfo.szHostMachineName);
}

//===================================================================
//-------------------------------------------------------------------

LOCAL void* CortexListenThread_Func(void *)
{
  //hostent *pHostEnt;
  socklen_t addr_len;
  int nBytesReceived;
  int Count = 0;
  sockaddr_in TheirAddress;

  //memset(&CortexAddr, 0, sizeof(sockaddr_in));
  addr_len = sizeof(struct sockaddr);

  while (1) {
    // Block further processing until we receive a datagram from anyone,
    // including ourself, over the network.
    // This thread will spend most if its time asleep in this recvfrom
    // function.

    nBytesReceived = recvfrom(CommandSocket, (char *) &PacketIn,
                              sizeof(sPacket), 0,
                              (struct sockaddr *) &TheirAddress, &addr_len);
    LogMessage(VL_Debug, "Got reply of %d bytes of data", nBytesReceived);

    // We shutdown and closed the socket from Cortex_Exit()
    if (nBytesReceived == 0 || nBytesReceived == SOCKET_ERROR) {
      break;
    }

    Count++;
    if (memcmp(&TheirAddress.sin_addr.s_addr, HostInfo.HostMachineAddress, 4)
        == 0) {
#if 0
      if (!HostInfo.bFoundHost)
      {
        FoundHost();
      }
#endif
      LogMessage(VL_Debug, "Packet return address matches Cortex address");
      HostInfo.LatestConfirmationTime = clock();
    }

    LogMessage(VL_Debug,
               "CommandReplyReader received from %s: Command=%d, nBytes=%d",
               inet_ntoa(TheirAddress.sin_addr), (int) PacketIn.iCommand,
               (int) PacketIn.nBytes);

    // First send SerialCommand Confirmation

    // Now handle the data

    switch (PacketIn.iCommand) {
    case PKT2_HELLO_WORLD:
      LogMessage(VL_Debug, "HELLO_WORLD: %s, Version %d.%d.%d",
                 PacketIn.Data.Me.szName,
                 //PacketIn.Data.Me.Version[0],
                 PacketIn.Data.Me.Version[1], PacketIn.Data.Me.Version[2],
                 PacketIn.Data.Me.Version[3]);
      break;

    case PKT2_HERE_I_AM:
      if (*(unsigned long*) HostInfo.HostMachineAddress != 0
          && *(unsigned long*) HostInfo.HostMachineAddress != 0xFFFFFFFF
          && *(unsigned long*) HostInfo.HostMachineAddress
              != TheirAddress.sin_addr.s_addr) {
        LogMessage(VL_Debug, "Ignoring HERE_I_AM message from another machine");
        break;
      }
      LogMessage(VL_Debug, "HERE_I_AM message");

      CortexAddr = TheirAddress;

      strcpy(HostInfo.szHostProgramName, PacketIn.Data.Me.szName);
      memcpy(HostInfo.HostProgramVersion, PacketIn.Data.Me.Version, 4);
      memcpy(HostInfo.HostMachineAddress, &CortexAddr.sin_addr.s_addr, 4);

      GetHostName_ASYNC();

      if (!HostInfo.bFoundHost) {
        FoundHost();
      }

#if 0
      LogMessage(VL_Debug, "HERE_I_AM: %s, Version %d.%d.%d",
          PacketIn.Data.Me.szName,
          //PacketIn.Data.Me.Version[0],
          PacketIn.Data.Me.Version[1],
          PacketIn.Data.Me.Version[2],
          PacketIn.Data.Me.Version[3]);
#endif
      break;

    case PKT2_BODYDEFS:
      pNewBodyDefs = Unpack_BodyDefs(PacketIn.Data.cData, PacketIn.nBytes);
      sem_post(&EH_CommandConfirmed);
      break;

    case PKT2_FRAME_OF_DATA:
      Unpack_FrameOfData(PacketIn.Data.cData, PacketIn.nBytes,
                         &Polled_FrameOfData);
      //CB_DataHandler(&Polled_FrameOfData);
      sem_post(&EH_CommandConfirmed);
      break;

    case PKT2_GENERAL_REPLY:
      sem_post(&EH_CommandConfirmed);
      break;

    case PKT2_UNRECOGNIZED_REQUEST:
      sem_post(&EH_CommandConfirmed);
      break;

    case PKT2_UNRECOGNIZED_COMMAND:
      sem_post(&EH_CommandConfirmed);
      break;

    case PKT2_COMMENT:
      LogMessage(VL_Debug, "COMMENT: %s\n", PacketIn.Data.String);
      break;

    default:
      LogMessage(
                 VL_Warning,
                 "CommandReplyReader, unexpected value, PacketIn.iCommand== %d\n",
                 PacketIn.iCommand);
      break;
    }
  }

  // Unreachable

  return 0;
}

//===================================================================
//-------------------------------------------------------------------

LOCAL void* ReadDataThread_Func(void *)
{
  sockaddr_in TheirAddress;
  socklen_t addr_len;
  int nBytesReceived;
  int Count = 0;

  if (MultiCastReaderSocket != -1) {
    close(MultiCastReaderSocket);
    MultiCastReaderSocket = -1;
  }

  MultiCastReaderSocket = Socket_CreateLargeMultiCast(MyNicCardAddress,
                                                      wMultiCastPort,
                                                      MultiCastAddress);

  if (MultiCastReaderSocket == -1) {
    LogMessage(VL_Error, "Unable to initialize FrameReader");
    return 0;
  } else {
    LogMessage(VL_Info, "Initialized multi-cast frame reader");
  }

  // TestRead(MultiCastReaderSocket);

  memset(&TheirAddress, 0, sizeof(sockaddr));
  memset(&LatestFrameOfData, 0, sizeof(sFrameOfData));

  while (1) {
    addr_len = sizeof(struct sockaddr);

    // Block further processing until we receive a datagram from anyone,
    // including ourself, over the network.
    // This thread should spend most if its time asleep in this recvfrom
    // function.

    nBytesReceived = recvfrom(MultiCastReaderSocket, (char *) &PacketIn_Frame,
                              sizeof(sPacket), 0, (sockaddr *) &TheirAddress,
                              &addr_len);
    LogMessage(VL_Debug, "Multicast: received %d bytes of data\n",
               nBytesReceived);

    // We shutdown and closed the socket from Cortex_Exit()
    if (nBytesReceived == 0 || nBytesReceived == SOCKET_ERROR) {
      break;
    }

    Count++;

    if (memcmp(&TheirAddress.sin_addr.s_addr, HostInfo.HostMachineAddress, 4)
        == 0) {
      // Officially? found on the OTHER socket, since that's where we make the requests.
      if (!HostInfo.bFoundHost) {
        FoundHost();
      }
      //HostInfo.bFoundHost = 1;
      LogMessage(VL_Debug,
                 "MultiCast packet return address matches Cortex host address");
      HostInfo.LatestConfirmationTime = clock();
    } else {
      LogMessage(VL_Debug, "MultiCastReader Ignoring packet from %s",
                 inet_ntoa(TheirAddress.sin_addr));
      continue;
    }

    LogMessage(VL_Debug,
               "MultiCastReader Received From %s: Command=%d, nBytes=%d",
               inet_ntoa(TheirAddress.sin_addr), (int) PacketIn_Frame.iCommand,
               (int) PacketIn_Frame.nBytes);

    // First send SerialCommand Confirmation

    // Now handle the data

    switch (PacketIn_Frame.iCommand) {
    case PKT2_FRAME_OF_DATA:
      Unpack_FrameOfData(PacketIn_Frame.Data.cData, PacketIn_Frame.nBytes,
                         &LatestFrameOfData);
      //      if (g_Meterer.IsActive()) {
      //        g_Meterer.StoreFrame(&LatestFrameOfData);
      //      } else {
      CB_DataHandler(&LatestFrameOfData);
      //      }
      break;

    case PKT2_HELLO_WORLD:
    case PKT2_HERE_I_AM:
      if (PacketIn_Frame.Data.Me.Version[0] != 1) // Not from Cortex: Ignore it
        break;

      //if (HostInfo.HostMachineAddress[0] == 0)
      if (*(unsigned long*) HostInfo.HostMachineAddress == 0
          || *(unsigned long*) HostInfo.HostMachineAddress == 0xFFFFFFFF
          || *(unsigned long*) HostInfo.HostMachineAddress
              == TheirAddress.sin_addr.s_addr) {
        CortexAddr.sin_addr.s_addr = TheirAddress.sin_addr.s_addr;

        memcpy(&HostInfo.HostMachineAddress, &TheirAddress.sin_addr.s_addr, 4);
        strcpy(HostInfo.szHostProgramName, PacketIn_Frame.Data.Me.szName);
        memcpy(&HostInfo.HostProgramVersion, PacketIn_Frame.Data.Me.Version, 4);
        GetHostName_ASYNC();
        if (!HostInfo.bFoundHost) {
          FoundHost();
        }
#if 0
        HostInfo.bFoundHost = 1;

        LogMessage(VL_Info, "AutoConnected to: %s Version %d.%d.%d at %d.%d.%d.%d (%s)",
            HostInfo.szHostProgramName,
            HostInfo.HostProgramVersion[1],
            HostInfo.HostProgramVersion[2],
            HostInfo.HostProgramVersion[3],
            HostInfo.HostMachineAddress[0],
            HostInfo.HostMachineAddress[1],
            HostInfo.HostMachineAddress[2],
            HostInfo.HostMachineAddress[3],
            HostInfo.szHostMachineName);

#endif
      }
      break;

    case PKT2_COMMENT:
      LogMessage(VL_Debug, "DataStream Comment: %s\n",
                 PacketIn_Frame.Data.String);
      break;
    }
  }

  // Unreachable

  return 0;
}

//===================================================================
//-------------------------------------------------------------------

LOCAL int Initialize_ListenForReplies()
{
  if (CommandSocket != -1) {
    return OK;
  }
  LogMessage(VL_Debug, "Creating Socket for commands");
  CommandSocket
      = Socket_CreateForBroadcasting(MyNicCardAddress.s_addr, wMyPort);

  if (CommandSocket == -1) {
    LogMessage(VL_Error, "Unable to initialize SDK Sockets.");
    return RC_NetworkError;
  }

  int status = pthread_create(&CortexListenThread_ID, NULL,
                              CortexListenThread_Func, NULL);
  if (status != 0) {
    LogMessage(VL_Error,
               "%s: pthread_create error starting CortexListenThread thread",
               __PRETTY_FUNCTION__);
  }

  return OK;
}

#if 0
int TestRead(SOCKET socket)
{
  sockaddr TheirAddress;
  int addr_len;
  int nBytesReceived;
  int Count=0;

  memset(&TheirAddress, 0, sizeof(sockaddr));

  addr_len = sizeof(struct sockaddr);

  // Block further processing until we receive a datagram from anyone,

  nBytesReceived = recvfrom(
      socket,
      (char *)&PacketIn,
      sizeof(sPacket),
      0,
      &TheirAddress,
      &addr_len);

  printf("Received MultiCast of %d bytes.\n", nBytesReceived);

  return nBytesReceived;
}
#endif

//===================================================================
//-------------------------------------------------------------------

LOCAL int Initialize_ListenForFramesOfData()
{
  int status = pthread_create(&ReadDataThread_ID, NULL, ReadDataThread_Func,
                              NULL);
  if (status != 0) {
    LogMessage(
               VL_Error,
               "Initialize_ListenForFramesOfData(), pthread_create error starting ReadDataThread thread");
  }

  return OK;
}

//==================================================================

/**   This function defines the connection routes to talk to Cortex.
 *
 *    Machines can have more than one ethernet interface.  This function
 *    is used to either set the ethernet interface to use, or to let
 *    the SDK auto-select the local interface, and/or the Cortex host.
 *    This function should only be called once at startup.
 *
 *  \param szMyNicCardAddress - "a.b.c.d" or HostName.  "" and NULL mean AutoSelect
 *
 *  \param szCortexNicCardAddress - "a.b.c.d" or HostName.  "" and NULL mean AutoSelect
 *
 *  \return maReturnCode - RC_Okay, RC_ApiError, RC_NetworkError, RC_GeneralError
 */
int Cortex_Initialize(const char* szMyNicCardAddress,
                      const char* szCortexNicCardAddress)
{
  int retval;
  in_addr MyAddresses[10];
  int nAddresses;

  // Initialization only happens once.

  if (bInitialized) {
    LogMessage(VL_Warning, "Already Initialized");
    return RC_GeneralError;
  }

  nAddresses = Cortex_GetAllOfMyAddresses((unsigned long *) MyAddresses, 10);
  if (nAddresses < 0) {
    LogMessage(VL_Error, "Unable to find my own machine");
    return RC_NetworkError;
  } else if (nAddresses == 0) {
    LogMessage(VL_Error, "This machine has no ethernet interfaces");
    return RC_NetworkError;
  }

  if (szMyNicCardAddress == NULL || szMyNicCardAddress[0] == 0) {
    if (nAddresses > 1) {
      LogMessage(
                 VL_Warning,
                 "The local machine has more than one ethernet interface.  Using the first one found.");
    }
    MyNicCardAddress = MyAddresses[0];

    LogMessage(VL_Info, "Initializing using my default ethernet address: %s",
               inet_ntoa(MyNicCardAddress));
  } else {
    retval = ConvertToIPAddress(szMyNicCardAddress, &MyNicCardAddress);
    if (retval != OK) {
      LogMessage(VL_Error, "Unable to find MyNicCardAddress \"%s\"",
                 szMyNicCardAddress);
      Cortex_Exit();
      return RC_NetworkError;
    }

    LogMessage(VL_Info, "Initializing using my address: %s",
               inet_ntoa(MyNicCardAddress));
  }

  if (ConvertToIPAddress(szCortexNicCardAddress, &CortexNicCardAddress) != OK) {
    LogMessage(VL_Error, "Unable to convert \"%s\" to IP Address for Cortex",
               szCortexNicCardAddress);
    return RC_NetworkError;
  }

  memset(&CortexAddr, 0, sizeof(CortexAddr));
  CortexAddr.sin_family = AF_INET; // host byte order
  CortexAddr.sin_port = htons(wCortexPort); // short, network byte order
  CortexAddr.sin_addr = CortexNicCardAddress;
  LogMessage(VL_Info, "Initializing using Cortex host address: %s",
             inet_ntoa(CortexNicCardAddress));

  memset(&HostInfo, 0, sizeof(sHostInfo));
  memcpy(HostInfo.HostMachineAddress, &CortexNicCardAddress, 4);

  memset(&Polled_FrameOfData, 0, sizeof(sFrameOfData));
  memset(&LatestFrameOfData, 0, sizeof(sFrameOfData));

  // Get the semaphore technique initialized (CL)
  if (sem_init(&EH_CommandConfirmed, 0, 0) != 0) {
    LogMessage(VL_Error, "Unable to initialize semaphore: %d", errno);
  }

  Initialize_ListenForReplies();
  usleep(10000);

  Initialize_ListenForFramesOfData();
  usleep(10000); // Give the ListenThread time to be ready for an answer to the startup packet

  // Let the world know we are here.
  PacketOut.iCommand = PKT2_HELLO_WORLD;
  PacketOut.nBytes = sizeof(sMe);
  strcpy(PacketOut.Data.Me.szName, "ClientTest");
  memcpy(PacketOut.Data.Me.Version, MyVersionNumber, 4);

  //Broadcast(&PacketOut);

  //Broadcast(CommandSocket, wCortexPort, (char *)&PacketOut, PacketOut.nBytes + 4);

  // At this point, if szCortexNicCardAddress was not specified,
  // then CortexAddr should be the broadcast address.
  // The ListenForReplies thread will hear the response and get the actual address

  SendToCortex(&PacketOut);

  usleep(100000); // sleep until threads are running

  bInitialized = 1;

  return RC_Okay;
}

//==================================================================
/**   The user supplied function will be called whenever a frame of data arrives.
 *
 *    The ethernet servicing is done via a thread created
 *    when the connection to Cortex is made.  This function is
 *    called from that thread.  Some tasks are not sharable
 *    directly across threads.  Window redrawing, for example,
 *    should be done via events or messages.
 *
 *  \param MyFunction - This user supply callback function handles the streaming data
 *
 *  \return maReturnCode - RC_Okay
 *
 *    Notes: The data parameter points to "hot" data. That frame of data
 *           will be overwritten with the next call to the callback function.
 */
int Cortex_SetDataHandlerFunc(void(*MyFunction)(sFrameOfData *FrameOfData))
{
  CB_DataHandler = MyFunction;
  return RC_Okay;
}

//==================================================================
/**   The user supplied function handles text messages posted from within the SDK.
 *
 *    Logging messages is done as a utility to help code and/or run using the SDK.
 *    Various messages get posted for help with error conditions or events that happen.
 *    Each message has a Log-Level assigned to it so the user can.
 *  \sa Cortex_SetVerbosityLevel
 *
 *
 *  \param  MyFunction - This user defined function handles messages from the SDK.
 *
 *  \return maReturnCode - RC_Okay
 */
int Cortex_SetErrorMsgHandlerFunc(void(*MyFunction)(int iLogLevel,
                                                    const char *szLogMessage))
{
  CB_ErrorMsgHandler = MyFunction;
  return RC_Okay;
}

//==================================================================
/**   This function stops all activity of the SDK.
 *
 *    This function should be called once before exiting.
 */
int Cortex_Exit()
{
  if (!bInitialized) {
    return -1;
  }

  // Close sockets first, this will cause the threads to exit their functions
  if (CommandSocket != -1) {
    close(CommandSocket);
    CommandSocket = -1;
  }

  if (MultiCastReaderSocket != -1) {
    close(MultiCastReaderSocket);
    MultiCastReaderSocket = -1;
  }

  //TODO: kill threads

  bInitialized = 0;

  return 0;
}

//==================================================================
/**   This function queries Cortex for its set of tracking objects.
 *
 *  \return sBodyDefs* - This is a pointer to the internal storage of
 *                       the results of the latest call to this function.
 *
 *  \sa Cortex_FreeBodyDefs
 */
sBodyDefs* Cortex_GetBodyDefs()
{
  int nTries = 3;

  PacketOut.iCommand = PKT2_REQUEST_BODYDEFS;
  PacketOut.nBytes = 0;

  // Sleep for a long enough time to expect a response
  // Currently set to 20 ms.

  // Set semaphore to wait for response.

  while (nTries--) {
    // In Linux, POSIX semaphores are of the auto-reset type
    // ResetEvent(EH_CommandConfirmed);

    SendToCortex(&PacketOut);

    // Sleep for a long enough time to expect a response
    int count = 10000;
    while (count--) {

      int retCode = sem_trywait(&EH_CommandConfirmed);
      if (!retCode) {

        /* Event is signaled */
        return pNewBodyDefs;

      } else {

        /* check whether somebody else has the semaphore locked */
        if (errno == EAGAIN) {
          usleep(10); /* sleep for 10us */
        } else {
          LogMessage(VL_Debug,
                     "Error in semaphore timeout in GetBodyDefs (error %d)",
                     errno);
        }

      }

    }
  }

  LogMessage(VL_Error, "No response from Cortex");
  return NULL;
}

//==================================================================
/**   This function frees the memory allocated by Cortex_GetBodyDefs
 *
 *  The data within the structure is freed and also the structure itself.

 * \param pBodyDefs - The item to free.
 *
 * \return RC_Okay
 */
int Cortex_FreeBodyDefs(sBodyDefs* pBodyDefs)
{
  int nBodies = pBodyDefs->nBodyDefs;
  int iBody;

  for (iBody = 0; iBody < nBodies; iBody++) {
    sBodyDef *pBody = &pBodyDefs->BodyDefs[iBody];

    // Free each array of pointers to the names
    if (pBody->szMarkerNames != NULL)
      free(pBody->szMarkerNames);
    if (pBody->Hierarchy.szSegmentNames != NULL)
      free(pBody->Hierarchy.szSegmentNames);
    if (pBody->Hierarchy.iParents != NULL)
      free(pBody->Hierarchy.iParents);
    if (pBody->szDofNames != NULL)
      free(pBody->szDofNames);
  }

  if (pBodyDefs->szAnalogChannelNames != NULL)
    free(pBodyDefs->szAnalogChannelNames);

  // Free the big space that contains all the names
  if (pBodyDefs->AllocatedSpace != NULL)
    free(pBodyDefs->AllocatedSpace);
  memset(pBodyDefs, 0, sizeof(sBodyDef)); // not needed anymore
  free(pBodyDefs);

  return RC_Okay;
}

//==================================================================
/**   This function returns a 4-byte version number.
 *
 * \param Version - An array of four bytes: ModuleID, Major, Minor, Bugfix
 *
 * \return RC_Okay
 */
int Cortex_GetSdkVersion(unsigned char Version[4])
{
  memcpy(Version, MyVersionNumber, 4);
  return RC_Okay;
}

//==================================================================
/**   This function sends commands to Cortex and returns a response.
 *
 *    This function is an extendable interface between the Client programs
 *    and the Host (Cortex) program.  The commands are sent as readable text strings.
 *    The response is returned unaltered.
 *
 * \param szCommand - The request to send the Cortex
 * \param Response - The reply
 * \param pnBytes - The number of bytes in the response
 *
 \verbatim
 Example:
 void *pResponse=NULL;
 Cortex_Request("GetFrameRate", &pResponse, sizeof(void*));
 fFrameRate = *(float*)pResponse;
 \endverbatim
 *
 * \return RC_Okay, RC_TimeOut, RC_NotRecognized, RC_GeneralError
 */
int Cortex_Request(const char* szCommand, void** Response, int *pnBytes)
{
  const char* FRAME_QUERY = "GetFrameOfData";

  int nTries = 10;
  int expectingFrame = 0;

  *pnBytes = 0;

  LogMessage(VL_Debug, "Requesting: %s", szCommand);

  PacketOut.iCommand = PKT2_GENERAL_REQUEST;
  PacketOut.nBytes = (int) strlen(szCommand) + 1;
  strcpy(PacketOut.Data.String, szCommand);

  // Is this a request for a frame of data
  if (strncmp(szCommand, FRAME_QUERY, strlen(FRAME_QUERY)) == 0) {
    expectingFrame = 1;
  }

  while (nTries--) {
    SendToCortex(&PacketOut);

    int count = 10000;
    while (count--) {

      int retCode = sem_trywait(&EH_CommandConfirmed);
      if (!retCode) {
        if (PacketIn.iCommand == PKT2_GENERAL_REPLY) {
          *Response = PacketIn.Data.cData;
          *pnBytes = PacketIn.nBytes;
          return RC_Okay;
        } else if (PacketIn.iCommand == PKT2_FRAME_OF_DATA && expectingFrame) {
          *Response = &Polled_FrameOfData;
          *pnBytes = PacketIn.nBytes;
          return RC_Okay;
        } else if (PacketIn.iCommand == PKT2_UNRECOGNIZED_REQUEST) {
          *Response = NULL;
          return RC_Unrecognized;
        } else {
          return RC_GeneralError;
        }
      }
      usleep(10); // wait because response not yet available
    }
  }

  LogMessage(VL_Warning, "Cortex_Request, Request Timeout");
  *Response = NULL;
  return RC_TimeOut;
}

//==================================================================
/** This function sets the filter level of the LogMessages.
 *
 *  The default verbosity level is VL_Warning.
 *
 * \param iLevel - one of the maVerbosityLevel enum values.
 *
 * \return RC_Okay
 */
int Cortex_SetVerbosityLevel(int iLevel)
{
  User_VerbosityLevel = iLevel;
  return RC_Okay;
}

//==================================================================
/** This function gets information about the connection to Cortex
 *
 *  This function returns IP-Address information and Cortex version information.
 *  The version info can be used to handle incompatible changes in either our code
 *  or your code.
 *
 * \param pHostInfo - Structure containing connection information
 *
 * \return RC_Okay, RC_NetworkError
 */
int Cortex_GetHostInfo(sHostInfo *pHostInfo)
{
  if (HostInfo.HostMachineAddress[0] == 0) {
    return RC_NetworkError;
  }

  memcpy(pHostInfo, &HostInfo, sizeof(sHostInfo));
  return RC_Okay;
}

//==================================================================
/** This function polls Cortex for the current frame
 *
 *  The SDK user has the streaming data available via the callback function.
 *  In addition, this function is available to get a frame directly.
 *
 *  Note: Cortex considers the current frame to be the latest LiveMode frame completed or,
 *        if not in LiveMode, the current frame is the one that is displayed on the screen.
 *
 * \return sFrameOfData
 */
sFrameOfData* Cortex_GetCurrentFrame()
{
  PacketOut.iCommand = PKT2_REQUEST_FRAME;
  PacketOut.nBytes = 0;

  int bytesSent = SendToCortex(&PacketOut);
  LogMessage(VL_Debug, "Cortex_GetCurrentFrame(), SendToCortex sent %d bytes",
             bytesSent);

  int nTries = 2000;
  while (nTries--) {
    int retCode = sem_trywait(&EH_CommandConfirmed);
    if (!retCode) {
      if (PacketIn.iCommand == PKT2_FRAME_OF_DATA) {
        LogMessage(VL_Debug, "FRAME_OF_DATA message");
        return &Polled_FrameOfData;
      } else if (PacketIn.iCommand == PKT2_GENERAL_REPLY) {
        LogMessage(VL_Debug, "GENERAL_REPLY message");
      }
    } else {
      if (errno == EAGAIN) { // semaphore still locked
        usleep(10);
      } else {
        LogMessage(VL_Debug,
                   "Error in semaphore timeout in GetCurrentFrame: %s",
                   strerror(errno));
      }

    }
  }

  return NULL;
}

//==================================================================
/** This function copies a frame of data.
 *
 *  The Destination frame should start initialized to all zeros.  The CopyFrame
 *  and FreeFrame functions will handle the memory allocations necessary to fill
 *  out the data.
 *
 * \param pSrc - The frame to copy FROM.
 * \param pDst - The frame to copy TO
 *
 * \return RC_Okay, RC_MemoryError
 */
int Cortex_CopyFrame(const sFrameOfData* pSrc, sFrameOfData* pDst)
{
  int iBody;
  int nBodies = pSrc->nBodies;
  const sBodyData* SrcBody;
  sBodyData* DstBody;

  int n;
  void *ptr;
  int size;

  pDst->iFrame = pSrc->iFrame;

  pDst->nBodies = nBodies;

  for (iBody = 0; iBody < nBodies; iBody++) {
    SrcBody = &pSrc->BodyData[iBody];
    DstBody = &pDst->BodyData[iBody];

    // Copy Markers

    n = SrcBody->nMarkers;
    size = n * sizeof(tMarkerData);

    if (DstBody->nMarkers != n) {
      ptr = realloc(DstBody->Markers, size);
      if (size > 0 && ptr == NULL) {
        Cortex_FreeFrame(pDst);
        return RC_MemoryError;
      }
      DstBody->nMarkers = n;
      DstBody->Markers = (tMarkerData*) ptr;
    }

    memcpy(DstBody->Markers, SrcBody->Markers, size);

    // Copy Segments

    n = SrcBody->nSegments;
    size = n * sizeof(tSegmentData);

    if (DstBody->nSegments != n) {
      ptr = realloc(DstBody->Segments, size);
      if (size > 0 && ptr == NULL) {
        Cortex_FreeFrame(pDst);
        return RC_MemoryError;
      }
      DstBody->nSegments = n;
      DstBody->Segments = (tSegmentData*) ptr;
    }

    memcpy(DstBody->Segments, SrcBody->Segments, size);

    // Copy DOFs

    n = SrcBody->nDofs;
    size = n * sizeof(tDofData);

    if (DstBody->nDofs != n) {
      ptr = realloc(DstBody->Dofs, size);
      if (size > 0 && ptr == NULL) {
        Cortex_FreeFrame(pDst);
        return RC_MemoryError;
      }
      DstBody->nDofs = n;
      DstBody->Dofs = (tDofData*) ptr;
    }

    memcpy(DstBody->Dofs, SrcBody->Dofs, size);

    // Copy extra items
    DstBody->fAvgDofResidual = SrcBody->fAvgDofResidual;
    DstBody->nIterations = SrcBody->nIterations;
    DstBody->ZoomEncoderValue = SrcBody->ZoomEncoderValue;
    DstBody->FocusEncoderValue = SrcBody->FocusEncoderValue;
  }

  // Copy Unidentified Markers

  n = pSrc->nUnidentifiedMarkers;
  size = n * sizeof(tMarkerData);

  if (pDst->nUnidentifiedMarkers != n) {
    ptr = realloc(pDst->UnidentifiedMarkers, size);
    if (size > 0 && ptr == NULL) {
      Cortex_FreeFrame(pDst);
      return RC_MemoryError;
    }
    pDst->nUnidentifiedMarkers = n;
    pDst->UnidentifiedMarkers = (tMarkerData*) ptr;
  }

  memcpy(pDst->UnidentifiedMarkers, pSrc->UnidentifiedMarkers, size);

  // Copy Analog

  const sAnalogData* SrcAnalog = &pSrc->AnalogData;
  sAnalogData* DstAnalog = &pDst->AnalogData;

  // Analog Channels

  int nChannels = SrcAnalog->nAnalogChannels;
  int nSamples = SrcAnalog->nAnalogSamples;

  size = nChannels * nSamples * sizeof(short);

  if (DstAnalog->nAnalogChannels != nChannels || DstAnalog->nAnalogSamples
      != nSamples) {
    ptr = realloc(DstAnalog->AnalogSamples, size);
    if (size > 0 && ptr == NULL) {
      Cortex_FreeFrame(pDst);
      return RC_MemoryError;
    }
    DstAnalog->nAnalogChannels = nChannels;
    DstAnalog->nAnalogSamples = nSamples;
    DstAnalog->AnalogSamples = (short*) ptr;
  }

  memcpy(DstAnalog->AnalogSamples, SrcAnalog->AnalogSamples, size);

  // Forces Data

  int nForcePlates = SrcAnalog->nForcePlates;
  int nForceSamples = SrcAnalog->nForceSamples;

  size = nForcePlates * nForceSamples * sizeof(tForceData);

  if (DstAnalog->nForcePlates != nForcePlates || DstAnalog->nForceSamples
      != nForceSamples) {
    ptr = realloc(DstAnalog->Forces, size);
    if (size > 0 && ptr == NULL) {
      Cortex_FreeFrame(pDst);
      return RC_MemoryError;
    }
    DstAnalog->nForcePlates = nForcePlates;
    DstAnalog->nForceSamples = nForceSamples;
    DstAnalog->Forces = (tForceData*) ptr;
  }

  memcpy(DstAnalog->Forces, SrcAnalog->Forces, size);

  int nAngleEncoders = SrcAnalog->nAngleEncoders;
  int nAngleEncoderSamples = SrcAnalog->nAngleEncoderSamples;

  size = nAngleEncoders * nAngleEncoderSamples * sizeof(double);

  if (DstAnalog->nAngleEncoders != nAngleEncoders
      || DstAnalog->nAngleEncoderSamples != nAngleEncoderSamples) {
    ptr = realloc(DstAnalog->AngleEncoderSamples, size);
    if (size > 0 && ptr == NULL) {
      Cortex_FreeFrame(pDst);
      return RC_MemoryError;
    }
    DstAnalog->nAngleEncoders = nAngleEncoders;
    DstAnalog->nAngleEncoderSamples = nAngleEncoderSamples;
    DstAnalog->AngleEncoderSamples = (double*) ptr;
  }

  memcpy(DstAnalog->AngleEncoderSamples, SrcAnalog->AngleEncoderSamples, size);

  return RC_Okay;
}

//==================================================================
/** This function frees memory within the structure.
 *
 *  The sFrameOfData structure includes pointers to various pieces of data.
 *  That data is dynamically allocated or reallocated to be consistent with
 *  the data that has arrived from Cortex.  To properly use the sFrameOfData
 *  structure, you should use the utility functions supplied.  It is possible
 *  to reuse sFrameOfData variables without ever freeing them.  The SDK will
 *  reallocate the components for you.
 *
 * \param pFrame - The frame of data to free.
 *
 * \return RC_Okay
 */
int Cortex_FreeFrame(sFrameOfData* pFrame)
{
  int iBody;

  for (iBody = 0; iBody < MAX_N_BODIES; iBody++) {
    sBodyData* Body = &pFrame->BodyData[iBody];

    if (Body->Markers != NULL)
      free(Body->Markers);
    if (Body->Segments != NULL)
      free(Body->Segments);
    if (Body->Dofs != NULL)
      free(Body->Dofs);
  }
  if (pFrame->UnidentifiedMarkers != NULL)
    free(pFrame->UnidentifiedMarkers);

  if (pFrame->AnalogData.AnalogSamples != NULL)
    free(pFrame->AnalogData.AnalogSamples);
  if (pFrame->AnalogData.Forces != NULL)
    free(pFrame->AnalogData.Forces);
  if (pFrame->AnalogData.AngleEncoderSamples != NULL)
    free(pFrame->AnalogData.AngleEncoderSamples);

  memset(pFrame, 0, sizeof(sFrameOfData));

  return RC_Okay;
}

LOCAL int SendToCortex(sPacket *Packet)
{
  // Here's my response back to the requestor
  LogMessage(VL_Debug, "Sending request %u", Packet->iCommand);
  return sendto(CommandSocket, (char *) Packet, 4 + Packet->nBytes, 0,
                (sockaddr *) &CortexAddr, sizeof(CortexAddr));
}

LOCAL void* GetHostByAddrThread_Func(void* _HostInfo)
{
  sHostInfo *HostInfo = (sHostInfo *) _HostInfo;

  GetHostByAddr(HostInfo->HostMachineAddress, HostInfo->szHostMachineName);

  return 0;
}

LOCAL void GetHostName_ASYNC()
{
  int status = pthread_create(&GetHostNameThread_ID, NULL,
                              GetHostByAddrThread_Func, (void*) &HostInfo);
  if (status != 0) {
    LogMessage(VL_Error,
               "pthread_create error starting GetHostByAddrThread_Func thread");
  }
}

//=============================================================================

LOCAL int PackHierarchy(sHierarchy *pHierarchy, void *Buf, int nBytesBuf)
{
  int nBytes = 0;
  char *Ptr = (char *) Buf;
  int iSegment;
  int nSegments = pHierarchy->nSegments;
  int len;

#if 0
  if (nSegments > MAX_SEGMENTS)
  {
    nSegments = 0;
  }
#endif

  memcpy(Ptr, &pHierarchy->nSegments, sizeof(int));
  nBytes += 4;
  Ptr += 4;

  for (iSegment = 0; iSegment < nSegments; iSegment++) {
    memcpy(Ptr, &pHierarchy->iParents[iSegment], sizeof(int));
    nBytes += 4;
    Ptr += 4;

    len = (int) strlen(pHierarchy->szSegmentNames[iSegment]);
    strcpy(Ptr, pHierarchy->szSegmentNames[iSegment]);
    nBytes += len + 1;
    Ptr += len + 1;
  }

  return nBytes;
}

//==================================================================
/** This function pushes a skeleton definition to Cortex.
 *
 *  A skeleton, defined in an animation package can be used to start
 *  a skeleton model definition in Cortex.  The hierarchy and starting
 *  pose can come from the animation package.  The rest of the details
 *  of the skeleton get filled out in the Cortex interface.  The parameters
 *  to this function match the parameters defining the HTR data that
 *  normally gets sent through the SDK2.
 *
 * \param pHierarchy - The number of segments, their names and parent child
 relationships.
 * \param pFrame - One frame of HTR data dimensioned according to the number
 *                 of segments defined in the pHierarchy parameter.
 *
 * \return - RC_Okay, RC_NetworkError
 */
int Cortex_SendHtr(sHierarchy *pHierarchy, tSegmentData *pFrame)
{
  static char BigArray[0x20000];
  unsigned short *Header = &((unsigned short *) BigArray)[0];
  char *Ptr = &BigArray[8];
  int nBytesHierarchy = 0;
  int nBytesFrame = 0;

  // Packet ID        - 2 bytes
  // Packet DataSize  - 2 bytes
  // nBytesHierachy   - 2 bytes
  // nBytesDataFrame  - 2 bytes

#if 1
  Ptr = &BigArray[8];
  nBytesHierarchy = PackHierarchy(pHierarchy, Ptr, 0x20000);
  Ptr += nBytesHierarchy;
#endif

  nBytesFrame = sizeof(int) + pHierarchy->nSegments * 7 * sizeof(double);
  memcpy(Ptr, pFrame, nBytesFrame);

  Header[0] = PKT2_PUSH_BASEPOSITION;
  Header[1] = 4 + nBytesHierarchy + nBytesFrame;
  Header[2] = nBytesHierarchy;
  Header[3] = nBytesFrame;

  if (SendToCortex((sPacket *) BigArray) != 0) {
    return RC_NetworkError;
  }

  return 0;
}

//==================================================================
/** This function decodes a rotation matrix into three Euler angles.
 *
 *  This function and its inverse are utility functions for processing
 *  the HTR rotations we send in each frame of data. We send Euler angles
 *  in ZYX format (some interpretations would call it XYZ). Using these
 *  conversion utilities should protect against any misinterpretations.
 *
 * \param matrix - 3x3 rotation matrix.
 * \param iRotationOrder - one of:
 *
 *        ZYX_ORDER
 *        XYZ_ORDER
 *        YXZ_ORDER
 *        YZX_ORDER
 *        ZXY_ORDER
 *        XZY_ORDER
 *
 * \param angles - the angles in degrees.
 *
 */
void Cortex_ExtractEulerAngles(double matrix[3][3], int iRotationOrder,
                               double angles[3])
{
  M3x3_ExtractEulerAngles(matrix, iRotationOrder, angles);
}

//==================================================================
/** This function consructs a rotation matrix from three Euler angles.
 *
 *  This function and its inverse are utility functions for processing
 *  the HTR rotations we send in each frame of data. We send Euler angles
 *  in ZYX format (some interpretations would call it XYZ). Using these
 *  conversion utilities should protect against any misinterpretations.
 *
 * \param matrix - 3x3 rotation matrix.
 * \param iRotationOrder - one of:
 *
 *        ZYX_ORDER
 *        XYZ_ORDER
 *        YXZ_ORDER
 *        YZX_ORDER
 *        ZXY_ORDER
 *        XZY_ORDER
 *
 * \param angles - the angles in degrees.
 *
 */
void Cortex_ConstructRotationMatrix(double angles[3], int iRotationOrder,
                                    double matrix[3][3])
{
  M3x3_ConstructRotationMatrix(angles[0], angles[1], angles[2], iRotationOrder,
                               matrix);
}
