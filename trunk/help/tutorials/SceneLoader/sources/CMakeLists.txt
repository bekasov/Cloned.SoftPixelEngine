
# === CMake lists for the SceneLoader Tutorial - (21/07/2012) ===

add_executable(
	TutorialSceneLoader
	${TutorialPath}/SceneLoader/sources/main.cpp
)

target_link_libraries(TutorialSceneLoader SoftPixelEngine)
