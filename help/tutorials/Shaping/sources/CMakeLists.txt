
# === CMake lists for the GettingStarted Tutorial - (03/08/2012) ===

add_executable(
	TutorialShaping
	${TutorialPath}/Shaping/sources/main.cpp
)

target_link_libraries(TutorialShaping SoftPixelEngine)
