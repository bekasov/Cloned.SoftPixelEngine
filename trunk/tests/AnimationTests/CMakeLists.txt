
# === CMake lists for "Animation Tests" - (19/01/2013) ===

add_executable(
	TestAnimation
	${TestsPath}/AnimationTests/main.cpp
)

target_link_libraries(TestAnimation SoftPixelEngine)
