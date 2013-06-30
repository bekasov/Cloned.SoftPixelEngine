
# === CMake lists for "Script Tests" - (30/06/2013) ===

add_executable(
	TestScript
	${TestsPath}/ScriptTests/main.cpp
)

target_link_libraries(TestScript SoftPixelEngine)
