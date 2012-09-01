
# === CMake lists for "PhysX Tests" - (31/08/2012) ===

add_executable(
	TestsPhysX
	${TestsPath}/PhysXTests/main.cpp
)

target_link_libraries(TestsPhysX SoftPixelEngine)
