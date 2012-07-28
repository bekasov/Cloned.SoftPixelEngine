
# === CMake lists for the MeshViewer - (28/07/2012) ===

if(WIN32)
	add_executable(
		ToolMeshViewer
		WIN32
		${ToolPath}/MeshViewer/sources/main.cpp
	)
else(WIN32)
	add_executable(
		ToolMeshViewer
		${ToolPath}/MeshViewer/sources/main.cpp
	)
endif(WIN32)

target_link_libraries(ToolMeshViewer SoftPixelEngine)
