
# === CMake lists for the Tessellation Tutorial - (03/08/2012) ===

add_executable(
	TutorialTessellation
	${TutorialPath}/Tessellation/sources/main.cpp
)

target_link_libraries(TutorialTessellation SoftPixelEngine)
