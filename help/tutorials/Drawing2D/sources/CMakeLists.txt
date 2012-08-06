
# === CMake lists for the Drawing2D Tutorial - (20/07/2012) ===

add_executable(
	TutorialDrawing2D
	${TutorialPath}/Drawing2D/sources/main.cpp
)

target_link_libraries(TutorialDrawing2D SoftPixelEngine)
