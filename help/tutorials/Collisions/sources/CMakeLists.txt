
# === CMake lists for the Collisions Tutorial - (20/07/2012) ===

add_executable(
	TutorialCollisions
	${TutorialPath}/Collisions/sources/main.cpp
)

target_link_libraries(TutorialCollisions SoftPixelEngine)
