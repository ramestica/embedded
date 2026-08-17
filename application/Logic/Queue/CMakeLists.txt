emb_module(
    NAME  Queue
    TITLE "Ring buffer / queue"
)

emb_library(
    NAME    Queue
    SOURCES src/Queue.cpp
)

target_link_libraries(Queue PUBLIC emb::Utils)

#___oOo___
