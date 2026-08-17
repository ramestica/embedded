# CMake target named "LogicI2C" (header stays Logic/I2C.h) — same reasoning
# as Logic/UART: Target/I2C already claimed the plain "I2C" target name.
if(NOT EMB_HOST)

emb_module(
    NAME  LogicI2C
    TITLE "Host-facing I2C protocol layer"
)

emb_library(
    NAME    LogicI2C
    SOURCES src/I2C.cpp
)

# Logic/I2C.h itself includes ErrorCodes.h + Host.h; Target/I2C.h is only
# used in I2C.cpp.
target_link_libraries(LogicI2C
    PUBLIC
        emb::ErrorCodes
        emb::Host
    PRIVATE
        emb::I2C
)

endif()

#___oOo___
