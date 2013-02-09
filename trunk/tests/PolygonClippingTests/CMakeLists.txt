
# === CMake lists for "PolygonClipping Tests" - (08/02/2013) ===

add_executable(
	TestPolygonClipping
	${TestsPath}/PolygonClippingTests/main.cpp
)

target_link_libraries(TestPolygonClipping SoftPixelEngine)
