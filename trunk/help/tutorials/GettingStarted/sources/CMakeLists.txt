
# === CMake lists for the GettingStarted Tutorial - (20/07/2012) ===

add_executable(
	TutorialGettingStarted
	${TutorialPath}/GettingStarted/sources/main.cpp
)

target_link_libraries(TutorialGettingStarted SoftPixelEngine)
