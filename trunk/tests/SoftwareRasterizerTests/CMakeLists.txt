
# === CMake lists for "SceneGraph Tests" - (07/04/2013) ===

add_executable(
	TestSoftwareRasterizer
	${TestsPath}/SoftwareRasterizerTests/main.cpp
)

target_link_libraries(TestSoftwareRasterizer SoftPixelEngine)
