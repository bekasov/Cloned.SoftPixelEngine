
# === CMake lists for "GLSL Compute Tests" - (21/05/2013) ===

add_executable(
	TestGLSLCompute
	${TestsPath}/GLSLComputeTests/main.cpp
)

target_link_libraries(TestGLSLCompute SoftPixelEngine)
