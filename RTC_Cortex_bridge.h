#ifndef RTC_TEST_H
#define RTC_TEST_H

#include <deque>
#include <iomanip>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <vector>

#include <rtm/Manager.h>
#include <rtm/DataFlowComponentBase.h>
#include <rtm/CorbaPort.h>
#include <rtm/DataInPort.h>
#include <rtm/DataOutPort.h>
#include <rtm/idl/BasicDataTypeSkel.h>
#include <rtm/idl/ExtendedDataTypesSkel.h>

#include "cortex.h"

// Service implementation headers
// <rtc-template block="service_impl_h">
#include "RTC_Cortex_bridgeService_impl.h"

// </rtc-template>

// Service Consumer stub headers
// <rtc-template block="consumer_stub_h">

// </rtc-template>

using namespace RTC;

/**
   \brief sample RT component which has one data input port and one data output port
 */
class RTC_Cortex_bridge
  : public RTC::DataFlowComponentBase
{
 public:
  /**
     \brief Constructor
     \param manager pointer to the Manager
  */
  RTC_Cortex_bridge(RTC::Manager* _manager);
  /**
     \brief Destructor
  */
  virtual ~RTC_Cortex_bridge();

  // The initialize action (on CREATED->ALIVE transition)
  // formaer rtc_init_entry()
  virtual RTC::ReturnCode_t onInitialize();

  // The finalize action (on ALIVE->END transition)
  // formaer rtc_exiting_entry()
  // virtual RTC::ReturnCode_t onFinalize();

  // The startup action when ExecutionContext startup
  // former rtc_starting_entry()
  // virtual RTC::ReturnCode_t onStartup(RTC::UniqueId ec_id);

  // The shutdown action when ExecutionContext stop
  // former rtc_stopping_entry()
  // virtual RTC::ReturnCode_t onShutdown(RTC::UniqueId ec_id);

  // The activated action (Active state entry action)
  // former rtc_active_entry()
  virtual RTC::ReturnCode_t onActivated(RTC::UniqueId ec_id);

  // The deactivated action (Active state exit action)
  // former rtc_active_exit()
  virtual RTC::ReturnCode_t onDeactivated(RTC::UniqueId ec_id);

  // The execution action that is invoked periodically
  // former rtc_active_do()
  virtual RTC::ReturnCode_t onExecute(RTC::UniqueId ec_id);

  // The aborting action when main logic error occurred.
  // former rtc_aborting_entry()
  // virtual RTC::ReturnCode_t onAborting(RTC::UniqueId ec_id);

  // The error action in ERROR state
  // former rtc_error_do()
  // virtual RTC::ReturnCode_t onError(RTC::UniqueId ec_id);

  // The reset action that is invoked resetting
  // This is same but different the former rtc_init_entry()
  // virtual RTC::ReturnCode_t onReset(RTC::UniqueId ec_id);

  // The state update action that is invoked after onExecute() action
  // no corresponding operation exists in OpenRTm-aist-0.2.0
  // virtual RTC::ReturnCode_t onStateUpdate(RTC::UniqueId ec_id);

  // The action that is invoked when execution context's rate is changed
  // no corresponding operation exists in OpenRTm-aist-0.2.0
  // virtual RTC::ReturnCode_t onRateChanged(RTC::UniqueId ec_id);

  bool test(const char *i_name);
  //void MyErrorMsgHandler(int iLevel, const char *szMsg);
  //void MyDataHandler(sFrameOfData* FrameOfData);
 protected:
  // Configuration variable declaration
  // <rtc-template block="config_declare">

  // </rtc-template>


  // DataInPort declaration
  // <rtc-template block="inport_declare">

  // </rtc-template>

  // DataOutPort declaration
  // <rtc-template block="outport_declare">
  TimedDoubleSeq m_analog;
  OutPort<TimedDoubleSeq> m_analogOut;

  // </rtc-template>

  // CORBA Port declaration
  // <rtc-template block="corbaport_declare">
  RTC::CorbaPort m_RTC_Cortex_bridgeServicePort;

  // </rtc-template>

  // Service declaration
  // <rtc-template block="service_declare">
  RTC_Cortex_bridgeService_impl m_service0;

  // </rtc-template>

  // Consumer declaration
  // <rtc-template block="consumer_declare">

  // </rtc-template>

 private:
  RTC::Manager* manager;
  sBodyDefs* pBodyDefs;
  sFrameOfData* FrameofData;
  std::vector<int> bodyMarkers;
  std::vector<double> analogData;
  int retval;
  void *pResponse;
  int nBytes;
  double del;
  int numChannels;
};


extern "C"
{
  void RTC_Cortex_bridgeInit(RTC::Manager* manager);
  void MyErrorMsgHandler(int iLevel, const char *szMsg);
  void MyDataHandler(sFrameOfData* FrameOfData);
};

#endif // RTC_TEST_TRG_H
