
# === CMake lists for "Draw Text Tests" - (30/09/2012) ===

add_executable(
	TestDrawText
	${TestsPath}/DrawTextTests/main.cpp
)

target_link_libraries(TestDrawText SoftPixelEngine)
