/*=========================================================
 //
 // File: ClientTest.cpp
 //
 // Created by Ned Phipps, Oct-2004
 //
 =============================================================================*/

#include <string.h>
#include <stdio.h>
#include <iostream>
#include <vector>

#include <unistd.h>

#include "cortex.h"

using namespace std;

void MyErrorMsgHandler(int iLevel, const char *szMsg)
{
  const char *szLevel = NULL;

  if (iLevel == VL_Debug) {
    szLevel = "Debug";
  } else if (iLevel == VL_Info) {
    szLevel = "Info";
  } else if (iLevel == VL_Warning) {
    szLevel = "Warning";
  } else if (iLevel == VL_Error) {
    szLevel = "Error";
  }

  printf("    %s: %s\n", szLevel, szMsg);
}


void MyDataHandler(sFrameOfData* FrameOfData)
{
  printf("Received multi-cast frame no %d\n", FrameOfData->iFrame);
}

int main(int argc, char* argv[])
{
  int retval = RC_Okay;
  unsigned char SDK_Version[4];
  sBodyDefs* pBodyDefs = NULL;

  sFrameOfData* FrameofData = NULL;
  sBodyDef* pBody{NULL};
  std::vector<int> bodyMarkers;
  //int nBytes;
  //void *pResponse;

  printf("Usage: ClientTest <Me> <Cortex>\n");
  printf("	Me = My machine name or its IP address\n");
  printf("	Cortex = Cortex' machine name or its IP Address\n");

  Cortex_SetVerbosityLevel(VL_Info);

  Cortex_GetSdkVersion(SDK_Version);
  printf("Using SDK Version %d.%d.%d\n", SDK_Version[1], SDK_Version[2],
         SDK_Version[3]);

  Cortex_SetErrorMsgHandlerFunc(MyErrorMsgHandler);
  Cortex_SetDataHandlerFunc(MyDataHandler);

  printf("****** Cortex_Initialize ******\n");
  retval = Cortex_Initialize("10.1.1.200", "10.1.1.190");
  if (argc == 1) {
    retval = Cortex_Initialize("", NULL);
  } else if (argc == 2) {
    retval = Cortex_Initialize(argv[1], NULL);
  } else if (argc == 3) {
    retval = Cortex_Initialize(argv[1], argv[2]);
  }

  if (retval != RC_Okay) {
    //retval = Cortex_Initialize("", NULL);
    printf("Error: Unable to initialize ethernet communication\n");
    retval = Cortex_Exit();
    return 1;
  }

  printf("****** Cortex_GetBodyDefs ******\n");
  pBodyDefs = Cortex_GetBodyDefs();

  if (pBodyDefs == NULL) {
    printf("Failed to get body defs\n");
  } else {
    printf("Got body defs\n");
    Cortex_FreeBodyDefs(pBodyDefs);
    pBodyDefs = NULL;
  }

  void *pResponse;
  int nBytes;
  retval = Cortex_Request("GetContextFrameRate", &pResponse, &nBytes);
  if (retval != RC_Okay)
    printf("ERROR, GetContextFrameRate\n");

  float *contextFrameRate = (float*) pResponse;

  printf("ContextFrameRate = %3.1f Hz\n", *contextFrameRate);

  printf("\n****** Cortex_GetBodyDefs ******\n");
  pBodyDefs = Cortex_GetBodyDefs();
  if (pBodyDefs == NULL)
    { printf("Failed to get body defs\n"); } 
  else
  {
    int totalBodies = pBodyDefs->nBodyDefs;
    cout << "total no of bodies tracked " << totalBodies << endl;
    for(int iBody=0; iBody<totalBodies; iBody++)
    {
      bodyMarkers.push_back(pBodyDefs->BodyDefs[iBody].nMarkers);
      pBody = &pBodyDefs->BodyDefs[iBody];
      cout << "number of markers defined in body " << iBody+1 << " (\"" << pBody->szName << "\") : " << bodyMarkers.at(iBody) << endl;    

      for (int iMarker=0 ; iMarker<pBody->nMarkers; iMarker++)
        { cout << iMarker << " " << pBody->szMarkerNames[iMarker] << endl; }
    }
  }

  printf("*** Starting live mode ***\n");
  retval = Cortex_Request("LiveMode", &pResponse, &nBytes);
  usleep(1000000);
  retval = Cortex_Request("Pause", &pResponse, &nBytes);
  printf("*** Paused live mode ***\n");

  printf("****** Cortex_Exit ******\n");
  retval = Cortex_Exit();

  return 0;
}
