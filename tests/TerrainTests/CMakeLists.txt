
# === CMake lists for "Terrain Tests" - (11/02/2013) ===

add_executable(
	TestTerrain
	${TestsPath}/TerrainTests/main.cpp
)

target_link_libraries(TestTerrain SoftPixelEngine)
