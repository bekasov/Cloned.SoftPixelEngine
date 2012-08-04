
# === CMake lists for the Primitives Tutorial - (03/08/2012) ===

add_executable(
	TutorialPrimitives
	${TutorialPath}/Primitives/sources/main.cpp
)

target_link_libraries(TutorialPrimitives SoftPixelEngine)
