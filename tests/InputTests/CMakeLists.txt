
# === CMake lists for "Input Tests" - (09/10/2012) ===

add_executable(
	TestInput
	${TestsPath}/InputTests/main.cpp
)

target_link_libraries(TestInput SoftPixelEngine)
