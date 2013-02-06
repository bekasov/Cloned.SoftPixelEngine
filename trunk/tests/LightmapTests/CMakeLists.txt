
# === CMake lists for "Lightmap Tests" - (05/02/2013) ===

add_executable(
	TestLightmap
	${TestsPath}/LightmapTests/main.cpp
)

target_link_libraries(TestLightmap SoftPixelEngine)
