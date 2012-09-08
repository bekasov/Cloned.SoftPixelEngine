
# === CMake lists for "Deferred Renderer Tests" - (08/09/2012) ===

add_executable(
	TestDeferredRenderer
	${TestsPath}/DeferredRendererTests/main.cpp
)

target_link_libraries(TestDeferredRenderer SoftPixelEngine)
