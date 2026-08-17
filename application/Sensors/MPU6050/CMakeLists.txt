if(NOT EMB_HOST)

emb_module(
    NAME  MPU6050
    TITLE "MPU6050 accelerometer/gyroscope driver"
)

emb_library(
    NAME    MPU6050
    SOURCES
        src/MPU6050Base.cpp
        src/MPU6050.cpp
)

target_link_libraries(MPU6050
    PUBLIC
        emb::LogicI2C
        emb::MainLoop
    PRIVATE
        emb::Timer
        emb::Endianness
        emb::Host
)

endif()

#___oOo___
