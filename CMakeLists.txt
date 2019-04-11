set(comp_sources RTC_Cortex_bridge.cpp RTC_Cortex_bridgeService_impl.cpp)
set(libs ${OPENHRP_LIBRARIES} hrpsysBaseStub myHrpsysStub)
include_directories(.)
include_directories(./Cortex_sdk)
add_library(RTC_Cortex_bridge SHARED ${comp_sources})
add_library(cortex_sdk ./Cortex_sdk/m3x3.cpp ./Cortex_sdk/cortex_socket.cpp ./Cortex_sdk/cortex_unpack.cpp ./Cortex_sdk/cortex.cpp)
target_link_libraries(cortex_sdk pthread m)

target_link_libraries(RTC_Cortex_bridge ${libs})
set_target_properties(RTC_Cortex_bridge PROPERTIES PREFIX "")

add_executable(RTC_Cortex_bridgeComp RTC_Cortex_bridgeComp.cpp ${comp_sources})
target_link_libraries(RTC_Cortex_bridgeComp ${libs} cortex_sdk)

set(target RTC_Cortex_bridge RTC_Cortex_bridgeComp)

install(TARGETS ${target}
  RUNTIME DESTINATION bin CONFIGURATIONS Release Debug
  LIBRARY DESTINATION lib CONFIGURATIONS Release Debug
)


