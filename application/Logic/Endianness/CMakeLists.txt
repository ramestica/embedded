emb_module(
    NAME  Endianness
    TITLE "Endianness detection/byte-swap helpers"
)

# Header-only — emb_library() assumes a compiled target (PUBLIC isn't valid
# on target_include_directories for a pure INTERFACE library), so this is
# wired up directly instead.
add_library(Endianness INTERFACE)
target_include_directories(Endianness INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(emb::Endianness ALIAS Endianness)

#___oOo___
