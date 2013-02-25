
# === CMake lists for "RayTracing Tests" - (10/02/2013) ===

add_executable(
	TestRayTracing
	${TestsPath}/RayTracingTests/main.cpp
)

target_link_libraries(TestRayTracing SoftPixelEngine)
