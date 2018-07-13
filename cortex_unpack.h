#ifndef CORTEX_UNPACK_H
#define CORTEX_UNPACK_H

#include "cortex.h"

int Unpack_BodyDef(char **pptr, sBodyDef *BodyDef);

int Unpack_AnalogDefs(char** pptr, sBodyDefs* pBodyDefs);

sBodyDefs* Unpack_BodyDefs(char *Data, int nBytes);

int Unpack_BodyData(char **pptr, sBodyData *pBody);

int Unpack_AnalogData(char** pptr, sAnalogData* pUnpacked);

int Unpack_ExtraStuff(char** pptr, sFrameOfData* Frame);

int Unpack_RecordingStatus(char* ptr, int nBytes,
                                 sRecordingStatus* pUnpacked);

int Unpack_EncoderAngles(char* ptr, int nBytes, sAnalogData* pUnpacked);

int Unpack_ZoomFocusEncoderData(char* ptr, int nBytes,
                                      sFrameOfData* Frame);

int Unpack_UnnamedMarkers(char **pptr, sFrameOfData *Frame);

int Unpack_FrameOfData(char *Data, int nTotalBytes, sFrameOfData *Frame);


#endif
