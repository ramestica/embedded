emb_module(
    NAME  Host
    TITLE "Host communication protocol (commands, data streaming)"
)

emb_library(
    NAME    Host
    SOURCES
        src/Host.cpp
        src/DataStream.cpp
        src/CommandReply.cpp
)

target_link_libraries(Host
    PUBLIC
        emb::Utils
        emb::Queue
    PRIVATE
        emb::ErrorCodes
)

#___oOo___
