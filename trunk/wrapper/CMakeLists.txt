
# === CMake lists for the "SoftPixel Engine Wrapper" - (28/09/2012) ===

file(GLOB FilesWrapper wrapper/spw*)

add_library(SoftPixelEngineWrapper SHARED ${FilesWrapper})

target_link_libraries(SoftPixelEngineWrapper SoftPixelEngine)
