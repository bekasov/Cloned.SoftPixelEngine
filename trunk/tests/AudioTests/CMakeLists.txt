
# === CMake lists for "Animation Tests" - (28/02/2013) ===

add_executable(
	TestAudio
	${TestsPath}/AudioTests/main.cpp
)

target_link_libraries(TestAudio SoftPixelEngine)
