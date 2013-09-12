
# === CMake lists for "Multi Context Tests" - (11/09/2013) ===

add_executable(
	TestMultiContext
	${TestsPath}/MultiContextTests/main.cpp
)

target_link_libraries(TestMultiContext SoftPixelEngine)
