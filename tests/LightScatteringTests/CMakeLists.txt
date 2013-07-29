
# === CMake lists for "Light Scattering Tests" - (07/25/2013) ===

add_executable(
	TestLightScattering
	${TestsPath}/LightScatteringTests/main.cpp
)

target_link_libraries(TestLightScattering SoftPixelEngine)
