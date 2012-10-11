
# === CMake lists for "Script Tests" - (11/10/2012) ===

add_executable(
	TestScript
	${TestsPath}/ScriptTests/main.cpp
)

target_link_libraries(TestScript SoftPixelEngine)
