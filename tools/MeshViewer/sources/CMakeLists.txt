
# === CMake lists for the MeshViewer - (28/07/2012) ===

add_executable(
	ToolMeshViewer
	${ToolPath}/MeshViewer/sources/main.cpp
)

target_link_libraries(ToolMeshViewer SoftPixelEngine)
