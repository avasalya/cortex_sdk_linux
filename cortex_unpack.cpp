#include "cortex_unpack.h"

#include "cortex_intern.h"

#include <string.h>
#include <stdlib.h>

// Tagged items definitions
#define TAG_ENCODER_ANGLES 11
#define TAG_RECORDING_STATUS 12
#define TAG_ZOOM_FOCUS_ENCODER_DATA 13


int Unpack_BodyDef(char **pptr, sBodyDef *BodyDef)
{
  int nMarkers;
  int nSegments;
  int nDofs;
  int iMarker;
  int iSegment;
  int iDof;
  char *ptr = *pptr;

  // The main name

  BodyDef->szName = ptr;
  ptr += strlen(ptr) + 1;

  // Markers

  memcpy(&nMarkers, ptr, 4);
  BodyDef->nMarkers = nMarkers;
  ptr += 4;

  BodyDef->szMarkerNames = (char **) malloc(nMarkers * sizeof(char*));

  for (iMarker = 0; iMarker < nMarkers; iMarker++) {
    BodyDef->szMarkerNames[iMarker] = ptr;
    ptr += strlen(ptr) + 1;
  }

  // Segments

  memcpy(&nSegments, ptr, 4);
  BodyDef->Hierarchy.nSegments = nSegments;
  ptr += 4;

  BodyDef->Hierarchy.szSegmentNames = (char **) malloc(nSegments
      * sizeof(char*));
  BodyDef->Hierarchy.iParents = (int *) malloc(nSegments * sizeof(int));

  for (iSegment = 0; iSegment < nSegments; iSegment++) {
    BodyDef->Hierarchy.szSegmentNames[iSegment] = ptr;
    ptr += strlen(ptr) + 1;

    memcpy(&BodyDef->Hierarchy.iParents[iSegment], ptr, 4);
    ptr += 4;
  }

  // Dofs

  memcpy(&nDofs, ptr, 4);
  BodyDef->nDofs = nDofs;
  ptr += 4;

  BodyDef->szDofNames = (char **) malloc(nDofs * sizeof(char*));

  for (iDof = 0; iDof < nDofs; iDof++) {
    BodyDef->szDofNames[iDof] = ptr;
    ptr += strlen(ptr) + 1;
  }

  *pptr = ptr;
  return 0;
}

int Unpack_AnalogDefs(char** pptr, sBodyDefs* pBodyDefs)
{
  char* ptr = *pptr;
  int iChannel;
  int nChannels;
  int nForcePlates;

  memcpy(&nChannels, ptr, 4);
  ptr += 4;

  pBodyDefs->nAnalogChannels = nChannels;
  pBodyDefs->szAnalogChannelNames = (char **) malloc(nChannels * sizeof(char*));

  for (iChannel = 0; iChannel < nChannels; iChannel++) {
    pBodyDefs->szAnalogChannelNames[iChannel] = ptr;
    ptr += strlen(ptr) + 1;
  }

  memcpy(&nForcePlates, ptr, 4);
  ptr += 4;
  pBodyDefs->nForcePlates = nForcePlates;

  *pptr = ptr;
  return 0;
}

sBodyDefs* Unpack_BodyDefs(char *Data, int nBytes)
{
  char *ptr = Data;
  char *MyBuffer;
  int iBody;
  int nBodies;
  sBodyDefs *pBodyDefs;

  pBodyDefs = (sBodyDefs *) malloc(sizeof(sBodyDefs));
  if (pBodyDefs == NULL) {
    return NULL;
  }

  memset(pBodyDefs, 0, sizeof(sBodyDefs));

  MyBuffer = (char *) malloc(nBytes);
  if (MyBuffer == NULL) {
    free(pBodyDefs);
    return NULL;
  }
  pBodyDefs->AllocatedSpace = MyBuffer;

  memcpy(MyBuffer, Data, nBytes);
  ptr = MyBuffer;

  memcpy(&nBodies, ptr, 4);
  pBodyDefs->nBodyDefs = nBodies;
  ptr += 4;

  for (iBody = 0; iBody < nBodies; iBody++) {
    Unpack_BodyDef(&ptr, &pBodyDefs->BodyDefs[iBody]);
  }

  Unpack_AnalogDefs(&ptr, pBodyDefs);

  return pBodyDefs;
}

int Unpack_BodyData(char **pptr, sBodyData *pBody)
{
  char *ptr = *pptr;
  int nBytes = 0;
  int nTotalBytes = 0;
  int nMarkers;
  int nSegments;
  int nDofs;

  // Name of the object

  strcpy(pBody->szName, ptr);
  nBytes = (int) strlen(pBody->szName) + 1;
  ptr += nBytes;
  nTotalBytes += nBytes;

  // The Markers

  memcpy(&nMarkers, ptr, 4);
  ptr += 4;
  nTotalBytes += 4;

  nBytes = nMarkers * sizeof(tMarkerData);

  if (nMarkers != pBody->nMarkers) {
    pBody->nMarkers = nMarkers;
    pBody->Markers = (tMarkerData*) realloc(pBody->Markers, nBytes);
  }

  memcpy(pBody->Markers, ptr, nBytes);
  ptr += nBytes;
  nTotalBytes += nBytes;

  // The Segments

  memcpy(&nSegments, ptr, 4);
  ptr += 4;
  nTotalBytes += 4;

  nBytes = nSegments * sizeof(tSegmentData);

  if (nSegments != pBody->nSegments) {
    pBody->nSegments = nSegments;
    pBody->Segments = (tSegmentData*) realloc(pBody->Segments, nBytes);
  }

  memcpy(pBody->Segments, ptr, nBytes);
  ptr += nBytes;
  nTotalBytes += nBytes;

  // The Dofs

  memcpy(&nDofs, ptr, 4);
  ptr += 4;
  nTotalBytes += 4;

  nBytes = nDofs * sizeof(tDofData);

  if (nDofs != pBody->nDofs) {
    pBody->nDofs = nDofs;
    pBody->Dofs = (tDofData*) realloc(pBody->Dofs, nBytes);
  }

  memcpy(pBody->Dofs, ptr, nBytes);
  ptr += nBytes;
  nTotalBytes += nBytes;

  // That's all folks

  *pptr = ptr;
  return nTotalBytes;
}

int Unpack_AnalogData(char** pptr, sAnalogData* pUnpacked)
{
  char *ptr = *pptr;
  int nBytes;
  int nTotalBytes = 0;
  int nSamples;
  int nChannels;
  int nForceSamples;
  int nForcePlates;

  // Raw analog samples

  memcpy(&nChannels, ptr, 4);
  ptr += 4;
  nTotalBytes += 4;

  memcpy(&nSamples, ptr, 4);
  ptr += 4;
  nTotalBytes += 4;

  if (nSamples != pUnpacked->nAnalogSamples || nChannels
      != pUnpacked->nAnalogChannels) {
    pUnpacked->nAnalogSamples = nSamples;
    pUnpacked->nAnalogChannels = nChannels;
    pUnpacked->AnalogSamples = (short*) realloc(pUnpacked->AnalogSamples,
                                                nSamples * nChannels
                                                    * sizeof(short));
  }

  nBytes = nSamples * nChannels * sizeof(short);
  memcpy(pUnpacked->AnalogSamples, ptr, nBytes);
  ptr += nBytes;
  nTotalBytes += nBytes;

  // Force data

  memcpy(&nForcePlates, ptr, 4);
  ptr += 4;
  nTotalBytes += 4;

  memcpy(&nForceSamples, ptr, 4);
  ptr += 4;
  nTotalBytes += 4;

  if (nForceSamples != pUnpacked->nForceSamples || nForcePlates
      != pUnpacked->nForcePlates) {
    pUnpacked->nForceSamples = nForceSamples;
    pUnpacked->nForcePlates = nForcePlates;
    pUnpacked->Forces = (tForceData*) realloc(pUnpacked->Forces, nForceSamples
        * nForcePlates * sizeof(tForceData));
  }

  nBytes = nForceSamples * nForcePlates * sizeof(tForceData);
  memcpy(pUnpacked->Forces, ptr, nBytes);
  ptr += nBytes;
  nTotalBytes += nBytes;

  *pptr = ptr;
  return nTotalBytes;
}

int Unpack_ExtraStuff(char** pptr, sFrameOfData* Frame)
{
  char *ptr = *pptr;
  int nBytes = 0;

  // Skip the tag
  ptr += 4;
  nBytes += 4;

  // Floating point delay value.

  memcpy(&Frame->fDelay, ptr, 4);
  ptr += 4;
  nBytes += 4;

  *pptr = ptr;
  return nBytes;
}

int Unpack_RecordingStatus(char* ptr, int nBytes,
                                 sRecordingStatus* pUnpacked)
{
  memcpy(&pUnpacked->bRecording, ptr, sizeof(int));
  ptr += 4;

  memcpy(&pUnpacked->iFirstFrame, ptr, sizeof(int));
  ptr += 4;

  memcpy(&pUnpacked->iLastFrame, ptr, sizeof(int));
  ptr += 4;

  strcpy(pUnpacked->szFilename, ptr);

  return OK;
}

int Unpack_EncoderAngles(char* ptr, int nBytes, sAnalogData* pUnpacked)
{
  int nCounters;
  int nSamples;
  int size;

  memcpy(&nCounters, ptr, sizeof(int));
  ptr += 4;

  memcpy(&nSamples, ptr, sizeof(int));
  ptr += 4;

  size = nCounters * nSamples * sizeof(double);

  if (nCounters > 2)
    return ERRFLAG;
  if (nBytes != 4 + 4 + 8 * nCounters * nSamples)
    return ERRFLAG;

  if (pUnpacked->nAngleEncoders != nCounters || pUnpacked->nAngleEncoderSamples
      != nSamples) {
    pUnpacked->nAngleEncoders = nCounters;
    pUnpacked->nAngleEncoderSamples = nSamples;
    pUnpacked->AngleEncoderSamples
        = (double*) realloc(pUnpacked->AngleEncoderSamples, size);
  }

  memcpy(pUnpacked->AngleEncoderSamples, ptr, size);

  return OK;
}

int Unpack_ZoomFocusEncoderData(char* ptr, int nBytes,
                                      sFrameOfData* Frame)
{
  for (int i = 0; i < Frame->nBodies; i++) {
    memcpy(&Frame->BodyData[i].ZoomEncoderValue, ptr, 4);
    ptr += 4;

    memcpy(&Frame->BodyData[i].FocusEncoderValue, ptr, 4);
    ptr += 4;
  }
  return OK;
}

int Unpack_UnnamedMarkers(char **pptr, sFrameOfData *Frame)
{
  char *ptr = *pptr;
  int nBytes = 0;
  int nTotalBytes = 0;
  int nMarkers;

  memcpy(&nMarkers, ptr, 4);
  ptr += 4;
  nTotalBytes += 4;

  nBytes = nMarkers * sizeof(tMarkerData);

  if (nMarkers != Frame->nUnidentifiedMarkers) {
    Frame->nUnidentifiedMarkers = nMarkers;
    Frame->UnidentifiedMarkers
        = (tMarkerData*) realloc(Frame->UnidentifiedMarkers, nBytes);
  }

  memcpy(Frame->UnidentifiedMarkers, ptr, nBytes);
  ptr += nBytes;
  nTotalBytes += nBytes;

  *pptr = ptr;
  return nTotalBytes;
}

int Unpack_FrameOfData(char *Data, int nTotalBytes, sFrameOfData *Frame)
{
  char *ptr = Data;
  int nBytes = 0;
  int iBody;
  int nBodies;

  if (Data == NULL || Frame == NULL) {
    return RC_ApiError;
  }

  memcpy(&Frame->iFrame, ptr, 4);
  ptr += 4;
  nBytes += 4;

  memcpy(&nBodies, ptr, 4);
  ptr += 4;
  nBytes += 4;

  if (nBodies < 0 || nBodies > MAX_N_BODIES) {
    LogMessage(VL_Error, "nBodies parameter is out of range");
    return RC_GeneralError;
  }

  Frame->nBodies = nBodies;

  for (iBody = 0; iBody < nBodies; iBody++) {
    nBytes += Unpack_BodyData(&ptr, &Frame->BodyData[iBody]);
  }

  // Unnamed markers

  nBytes += Unpack_UnnamedMarkers(&ptr, Frame);

  // Analog

  nBytes += Unpack_AnalogData(&ptr, &Frame->AnalogData);

  // Extra stuff

  if (nTotalBytes > nBytes + 4) {
    nBytes += Unpack_ExtraStuff(&ptr, Frame);
  }

  // These items are optional

  while (nTotalBytes > nBytes + 4) {
    int Tag;
    memcpy(&Tag, ptr, 4);
    ptr += 4;
    nBytes += 4;

    int nTheseBytes;
    memcpy(&nTheseBytes, ptr, 4);
    ptr += 4;
    nBytes += 4;

    switch (Tag) {
    case TAG_ENCODER_ANGLES:
      Unpack_EncoderAngles(ptr, nTheseBytes, &Frame->AnalogData);
      break;

    case TAG_RECORDING_STATUS:
      Unpack_RecordingStatus(ptr, nTheseBytes, &Frame->RecordingStatus);
      break;

    case TAG_ZOOM_FOCUS_ENCODER_DATA:
      Unpack_ZoomFocusEncoderData(ptr, nTheseBytes, Frame);
      break;
    }

    ptr += nTheseBytes;
    nBytes += nTheseBytes;
  }

  return nBytes;
}
