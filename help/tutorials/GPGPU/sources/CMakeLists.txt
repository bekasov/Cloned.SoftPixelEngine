
# === CMake lists for the GPGPU Tutorial - (23/08/2012) ===

add_executable(
	TutorialGPGPU
	${TutorialPath}/GPGPU/sources/main.cpp
)

target_link_libraries(TutorialGPGPU SoftPixelEngine)
