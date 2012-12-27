
# Custom CMake module for finding "DirectX SDK" files

if(WIN32)
	find_path(DirectX_INCLUDE_DIRS d3d9.h PATHS
		"$ENV{DXSDK_DIR}/Include"
		"$ENV{PROGRAMFILES}/Microsoft DirectX SDK*/Include"
	)
	get_filename_component(DirectX_ROOT_DIR "${DirectX_INCLUDE_DIRS}/.." ABSOLUTE)
	
	if(CMAKE_CL_64)
		set(DirectX_LIBRARY_PATHS "${DirectX_ROOT_DIR}/Lib/x64")
	else(CMAKE_CL_64)
		set(DirectX_LIBRARY_PATHS "${DirectX_ROOT_DIR}/Lib/x86" "${DirectX_ROOT_DIR}/Lib")
	endif(CMAKE_CL_64)
	
	find_library(DirectX_D3D9_LIBRARY d3d9 ${DirectX_LIBRARY_PATHS} NO_DEFAULT_PATH)
	find_library(DirectX_D3D11_LIBRARY d3d11 ${DirectX_LIBRARY_PATHS} NO_DEFAULT_PATH)
	find_library(DirectX_D2D1_LIBRARY d2d1 ${DirectX_LIBRARY_PATHS} NO_DEFAULT_PATH)
	find_library(DirectX_DWRITE_LIBRARY dwrite ${DirectX_LIBRARY_PATHS} NO_DEFAULT_PATH)
	find_library(DirectX_DXGUID_LIBRARY dxguid ${DirectX_LIBRARY_PATHS} NO_DEFAULT_PATH)
	find_library(DirectX_D3DCOMPILER_LIBRARY d3dcompiler ${DirectX_LIBRARY_PATHS} NO_DEFAULT_PATH)
	find_library(DirectX_XINPUT_LIBRARY XInput ${DirectX_LIBRARY_PATHS} NO_DEFAULT_PATH)
	
	set(DirectX_LIBRARIES
		${DirectX_D3D9_LIBRARY}
		${DirectX_D3D11_LIBRARY}
		${DirectX_D2D1_LIBRARY}
		${DirectX_DWRITE_LIBRARY}
		${DirectX_DXGUID_LIBRARY}
		${DirectX_D3DCOMPILER_LIBRARY}
		${DirectX_XINPUT_LIBRARY}
	)
	
	include(FindPackageHandleStandardArgs)
	FIND_PACKAGE_HANDLE_STANDARD_ARGS(DirectX DEFAULT_MSG DirectX_ROOT_DIR DirectX_LIBRARIES DirectX_INCLUDE_DIRS)
	
	mark_as_advanced(
		DirectX_INCLUDE_DIRS
		DirectX_D3D9_LIBRARY
		DirectX_D3D11_LIBRARY
		DirectX_D2D1_LIBRARY
		DirectX_DWRITE_LIBRARY
		DirectX_DXGUID_LIBRARY
		DirectX_D3DCOMPILER_LIBRARY
		DirectX_XINPUT_LIBRARY
	)
endif(WIN32)
