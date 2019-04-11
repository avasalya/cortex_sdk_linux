#include "RTC_Cortex_bridgeService_impl.h"
#include "RTC_Cortex_bridge.h"
#include <iostream>

RTC_Cortex_bridgeService_impl::RTC_Cortex_bridgeService_impl() : m_rtc(NULL)
{
}

RTC_Cortex_bridgeService_impl::~RTC_Cortex_bridgeService_impl()
{
}

CORBA::Boolean RTC_Cortex_bridgeService_impl::test1(const char *name)
{
  std::cout << "RTC_Cortex_bridgeService_impl::test1::" << name << std::endl;
  return m_rtc->test(name);
}

CORBA::Boolean RTC_Cortex_bridgeService_impl::test2()
{
  std::cout << "RTC_Cortex_bridgeService_impl::test2" << std::endl;
}

void RTC_Cortex_bridgeService_impl::test3(CORBA::ULong len)
{
  std::cout << "RTC_Cortex_bridgeService_impl::test3::" << len << std::endl;
}


