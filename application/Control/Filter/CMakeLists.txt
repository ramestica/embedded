# Also exposes Control/FirstOrder.h (header-only, no .cpp of its own).
emb_module(
    NAME  Filter
    TITLE "Complementary and first-order filters"
)

emb_library(
    NAME    Filter
    SOURCES src/Complementary.cpp
)

target_link_libraries(Filter
    PRIVATE
        emb::ErrorCodes
        emb::Utils
)

#___oOo___
