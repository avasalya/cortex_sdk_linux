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

#include <unistd.h>

#include "cortex.h"

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
  printf("  %s: %s\n", szLevel, szMsg);
}


int main()
{

  sBodyDefs* pBodyDefs = NULL;
  sFrameOfData* FrameofData = NULL;

  int retval = RC_Okay;

  Cortex_SetVerbosityLevel(VL_Info);
  Cortex_SetErrorMsgHandlerFunc(MyErrorMsgHandler);
  
  retval = Cortex_Initialize("10.1.1.180", "10.1.1.100");

  if (retval != RC_Okay)
  {
    printf("Error: Unable to initialize ethernet communication\n");
    retval = Cortex_Exit();
    return 1;
  }
  
  printf("\n****** Cortex_GetBodyDefs ******\n");

  pBodyDefs = Cortex_GetBodyDefs();
  if (pBodyDefs == NULL) 
  {
    printf("Failed to get body defs\n");
  } 
  else 
  {
    printf("number of markers defined in body1: %d\n",pBodyDefs->BodyDefs[0].nMarkers);
    printf("number of markers defined in body2:  %d\n",pBodyDefs->BodyDefs[1].nMarkers);
  }

  void *pResponse;
  int nBytes;
  retval = Cortex_Request("GetContextFrameRate", &pResponse, &nBytes);
  if (retval != RC_Okay)
    printf("ERROR, GetContextFrameRate\n");
  float *contextFrameRate = (float*) pResponse;
  printf("ContextFrameRate = %3.1f Hz\n", *contextFrameRate);


  printf("\n*** Starting live mode ***\n");
  
  retval = Cortex_Request("LiveMode", &pResponse, &nBytes);
  int bodyStickMarkers =  pBodyDefs->BodyDefs[0].nMarkers;
  Cortex_FreeBodyDefs(pBodyDefs);
  
  while(1)
  {
    FrameofData =  Cortex_GetCurrentFrame();  // Can POLL for the current frame. 

    printf("time delay from camera %f  cortex frame %d\n", FrameofData->fDelay, FrameofData->iFrame);
    for(int m = 0; m<bodyStickMarkers; m++)
    {
      // usleep(100000);  
      printf("body1 markers %d == x: %f y: %f z: %f\n",                       
                      (m+1),
                      FrameofData->BodyData[0].Markers[m][0],
                      FrameofData->BodyData[0].Markers[m][1],
                      FrameofData->BodyData[0].Markers[m][2]);
    }
    // usleep(1000000);

    if(FrameofData->iFrame == 1000)
    {
      printf("\n****** Paused live mode ... exiting Cortex ******\n");
      retval = Cortex_Request("Pause", &pResponse, &nBytes);
      retval = Cortex_Exit();
    }
  } 
  return 0;
}