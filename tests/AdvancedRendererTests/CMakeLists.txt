
# === CMake lists for "Advanced Renderer Tests" - (08/09/2012) ===

add_executable(
	TestAdvancedRenderer
	${TestsPath}/AdvancedRendererTests/main.cpp
)

target_link_libraries(TestAdvancedRenderer SoftPixelEngine)
