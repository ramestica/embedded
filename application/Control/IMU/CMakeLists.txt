emb_module(
    NAME  IMU
    TITLE "IMU orientation device using complementary/first-order filters"
)

emb_library(
    NAME    IMU
    SOURCES src/Device.cpp
)

target_link_libraries(IMU PUBLIC emb::Filter)

#___oOo___
