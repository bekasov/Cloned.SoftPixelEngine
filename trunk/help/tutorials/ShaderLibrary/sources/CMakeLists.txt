
# === CMake lists for the GettingStarted ShaderLibrary - (20/07/2012) ===

set(TutorialSourcePath ${TutorialPath}/ShaderLibrary/sources)

add_executable(
	TutorialShaderLibrary
	${TutorialSourcePath}/Library.cpp
	${TutorialSourcePath}/Library.hpp
	${TutorialSourcePath}/main.cpp
)

target_link_libraries(TutorialShaderLibrary SoftPixelEngine)
