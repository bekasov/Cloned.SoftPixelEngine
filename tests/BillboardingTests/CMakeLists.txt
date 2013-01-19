
# === CMake lists for the "Billboarding Tests" - (19/01/2013) ===

add_executable(
	TestBillboarding
	${TestsPath}/BillboardingTests/main.cpp
)

target_link_libraries(TestBillboarding SoftPixelEngine)
