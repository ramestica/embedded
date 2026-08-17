emb_module(
    NAME  ErrorCodes
    TITLE "Common error codes and longjmp-based error signaling"
)

emb_library(
    NAME    ErrorCodes
    SOURCES src/ErrorCodes.cpp
)

#___oOo___
