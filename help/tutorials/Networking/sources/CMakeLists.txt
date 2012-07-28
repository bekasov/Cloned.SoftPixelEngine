
# === CMake lists for the Networking Tutorial - (28/07/2012) ===

add_executable(
	TutorialNetworking
	${TutorialPath}/Networking/sources/main.cpp
)

target_link_libraries(TutorialNetworking SoftPixelEngine)
