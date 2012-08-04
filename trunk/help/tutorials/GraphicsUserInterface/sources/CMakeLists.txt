
# === CMake lists for the GraphicsUserInterface Tutorial - (03/08/2012) ===

add_executable(
	TutorialGraphicsUserInterface
	${TutorialPath}/GraphicsUserInterface/sources/main.cpp
)

target_link_libraries(TutorialGraphicsUserInterface SoftPixelEngine)
