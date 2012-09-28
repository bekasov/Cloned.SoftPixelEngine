
# === CMake lists for "Storyboard Tests" - (17/09/2012) ===

add_executable(
	TestStoryboard
	${TestsPath}/StoryboardTests/main.cpp
)

target_link_libraries(TestStoryboard SoftPixelEngine)
