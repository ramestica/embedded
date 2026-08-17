emb_module(
    NAME  SystemView
    TITLE "SEGGER SystemView tracing hooks"
)

# Only the no-op variant is ported so far — the real SEGGER RTT tracing
# implementation (SEGGER_SYSVIEW.c/SEGGER_RTT.c/...) is deferred until
# something actually needs it.
emb_library(
    NAME    SystemViewOff
    SOURCES src/SystemViewOff.cpp
)

#___oOo___
