
# === CMake lists for the PhysicsSimulation Tutorial - (28/03/2013) ===

add_executable(
	TutorialPhysicsSimulation
	${TutorialPath}/PhysicsSimulation/sources/main.cpp
)

target_link_libraries(TutorialPhysicsSimulation SoftPixelEngine)
