if(NOT EMB_HOST)

emb_module(
    NAME  PWM
    TITLE "TM4C123 PWM driver"
)

emb_library(
    NAME    PWM
    SOURCES src/PWM.cpp
)

target_link_libraries(PWM PUBLIC emb::GPIO)

endif()

#___oOo___
