
# === CMake lists for "SceneGraph Tests" - (28/01/2013) ===

add_executable(
	TestSceneGraph
	${TestsPath}/SceneGraphTests/main.cpp
)

target_link_libraries(TestSceneGraph SoftPixelEngine)
