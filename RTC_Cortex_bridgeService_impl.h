// -*-C++-*-
#ifndef __RTC_TEST_SERVICE_IMPL_H__
#define __RTC_TEST_SERVICE_IMPL_H__

#include "RTC_Cortex_bridgeService.hh"

class RTC_Cortex_bridge;

class RTC_Cortex_bridgeService_impl
  : public virtual POA_OpenHRP::RTC_Cortex_bridgeService,
    public virtual PortableServer::RefCountServantBase
{
public:
  RTC_Cortex_bridgeService_impl();
  virtual ~RTC_Cortex_bridgeService_impl();

  void setMyRTC(RTC_Cortex_bridge *i_rtc) { m_rtc = i_rtc; }

  CORBA::Boolean test1(const char *name);
  CORBA::Boolean test2();
  void test3(CORBA::ULong len);
private:
  RTC_Cortex_bridge *m_rtc;
};

#endif
