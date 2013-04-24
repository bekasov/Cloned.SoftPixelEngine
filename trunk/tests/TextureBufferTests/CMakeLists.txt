
# === CMake lists for "TextureBuffer Tests" - (24/04/2013) ===

add_executable(
	TestTextureBuffer
	${TestsPath}/TextureBufferTests/main.cpp
)

target_link_libraries(TestTextureBuffer SoftPixelEngine)
