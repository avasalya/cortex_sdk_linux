// -*- C++ -*-
/*!
 * @file  RTC_Cortex_bridge.cpp
 * @brief RTC_Cortex_bridge component
 * $Date$ 15/01/2019
 * $author$ Takahiro ITO
 * $Id$
 */

#include "RTC_Cortex_bridge.h"
#include "cortex.h"

using namespace std;

// Module specification
// <rtc-template block="module_spec">
static const char* nullcomponent_spec[] =
  {
    "implementation_id", "RTC_Cortex_bridge",
    "type_name",         "RTC_Cortex_bridge",
    "description",       "data logger component",
    "version",           "1.0",
    "vendor",            "AIST",
    "category",          "example",
    "activity_type",     "DataFlowComponent",
    "max_instance",      "10",
    "language",          "C++",
    "lang_type",         "compile",
    "conf.default.numChannels", "32",
    // Configuration variables
    ""
  };
// </rtc-template>

RTC_Cortex_bridge::RTC_Cortex_bridge(RTC::Manager* _manager)
  : RTC::DataFlowComponentBase(_manager),
    // <rtc-template block="initializer">
    m_RTC_Cortex_bridgeServicePort("RTC_Cortex_bridgeService"),
    m_analogOut("analogOut", m_analog),
    numChannels(32)
    // </rtc-template>
{
  manager = _manager;
  m_service0.setMyRTC(this);
}

RTC_Cortex_bridge::~RTC_Cortex_bridge()
{
}

RTC::ReturnCode_t RTC_Cortex_bridge::onInitialize()
{
  std::cout << m_profile.instance_name << ": onInitialize()" << std::endl;
  // Registration: InPort/OutPort/Service
  // <rtc-template block="registration">
  // Set InPort buffers

  // Set OutPort buffer
  addOutPort("analogOut", m_analogOut);

  // Set service provider to Ports
  m_RTC_Cortex_bridgeServicePort.registerProvider("service0", "RTC_Cortex_bridgeService", m_service0);

  // Set service consumers to Ports

  // Set CORBA Service Ports
  addPort(m_RTC_Cortex_bridgeServicePort);

  // </rtc-template>
  // <rtc-template block="bind_config">
  // Bind variables and configuration variable
  bindParameter("numChannels", numChannels, "32");

  // </rtc-template>
  pBodyDefs = NULL;
  FrameofData = NULL;
  m_analog.data.length(numChannels);
  retval = RC_Okay;

  Cortex_SetVerbosityLevel(VL_Info);
  Cortex_SetErrorMsgHandlerFunc(MyErrorMsgHandler);
  Cortex_SetDataHandlerFunc(MyDataHandler);

  cout << "****** Cortex_Initialize ******\n" << endl;
  retval = Cortex_Initialize("", NULL);
  if (retval != RC_Okay){
    cout << "Error: Unable to initialize ethernet communication" << endl; 
    retval = Cortex_Exit();
    return RTC::RTC_ERROR;
  }

  // cortex frame rate //
  cout << "\n****** Cortex_FrameRate ******\n" << endl;
  retval = Cortex_Request("GetContextFrameRate", &pResponse, &nBytes);
  if (retval != RC_Okay){
    cout << "ERROR, GetContextFrameRate\n" << endl;
  }
  float *contextFrameRate = (float*) pResponse;
  cout << "ContextFrameRate = %3.1f Hz\n" << *contextFrameRate << endl;

  // get name of bodies being tracked and its set of markers //
  cout << "\n****** Cortex_GetBodyDefs ******\n" << endl;
  pBodyDefs = Cortex_GetBodyDefs();

  if (pBodyDefs == NULL){
    cout << "Failed to get body defs\n" << endl;
  } 
  else{
    cout << "total no. of analog channels " << pBodyDefs->nAnalogChannels << endl;
    for(int iChannel=0; iChannel<pBodyDefs->nAnalogChannels; iChannel++){
      cout << "name of analog channel " << iChannel+1 << " : " << pBodyDefs->szAnalogChannelNames[iChannel] << endl;    
    }
  }

  return RTC::RTC_OK;
}

RTC::ReturnCode_t RTC_Cortex_bridge::onActivated(RTC::UniqueId ec_id)
{
  cout << "\n*** Starting live mode ***" << endl;
  retval = Cortex_Request("LiveMode", &pResponse, &nBytes);
  del = 0;
  return RTC::RTC_OK;
}

RTC::ReturnCode_t RTC_Cortex_bridge::onDeactivated(RTC::UniqueId ec_id)
{
  cout << "\ntotal delay from camera " << del << endl;
  cout << "\n****** Paused live mode ... exiting Cortex ******\n" << endl;
  Cortex_FreeFrame(FrameofData);
  Cortex_Request("Pause", &pResponse, &nBytes);
  Cortex_Exit();
  return RTC::RTC_OK;
}

RTC::ReturnCode_t RTC_Cortex_bridge::onExecute(RTC::UniqueId ec_id)
{
  FrameofData =  Cortex_GetCurrentFrame();  // Can POLL for the current frame. 
  del+=FrameofData->fDelay;
  for(int b = 0; b<FrameofData->AnalogData.nAnalogChannels*FrameofData->AnalogData.nAnalogSamples; b++){
      m_analog.data[b] = FrameofData->AnalogData.AnalogSamples[b];
        // cout << "body1 markers == " << (m+1) << endl <<
        //               " X: " << FrameofData->BodyData[b].Markers[m][0] << endl <<
        //               " Y: " << FrameofData->BodyData[b].Markers[m][1] << endl <<
        //               " Z: " << FrameofData->BodyData[b].Markers[m][2] << endl;
        // cout << FrameofData->BodyData[b].Markers[1][0] - FrameofData->BodyData[b].Markers[2][0] << endl;
  }
  
  setTimestamp(m_analog);
  m_analogOut.write();
  
  return RTC::RTC_OK;
}

bool RTC_Cortex_bridge::test(const char *i_name)
{
  std::vector<RtcBase*> vec = manager->getComponents();
  cout << "show all compoments name" << endl;
  for(int i=0; i<(int)vec.size(); i++)
    cout << i << "\t" << vec.at(i)->getInstanceName() << endl;

  RtcBase* rtc = manager->getComponent(i_name);
  if(!rtc) {
    cout << "RTC_Cortex_bridge::cannot find "  << i_name << endl;
    return false;
  }

  return true;
}

extern "C"
{
  void RTC_Cortex_bridgeInit(RTC::Manager* manager)
  {
    RTC::Properties profile(nullcomponent_spec);
    manager->registerFactory(profile,
                             RTC::Create<RTC_Cortex_bridge>,
                             RTC::Delete<RTC_Cortex_bridge>);
  }

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

  void MyDataHandler(sFrameOfData* FrameOfData)
  {
    printf("Received multi-cast frame no %d\n", FrameOfData->iFrame);
  }

};


