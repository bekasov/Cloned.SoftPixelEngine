
# Custom CMake module for finding "NVIDIA Cg Toolkit" files

if(APPLE)
	include(${CMAKE_ROOT}/Modules/CMakeFindFrameworks.cmake)
	set(CG_FRAMEWORK_INCLUDES)
	CMAKE_FIND_FRAMEWORKS(Cg)
	
	if(Cg_FRAMEWORKS)
		foreach(dir ${Cg_FRAMEWORKS})
			set(CG_FRAMEWORK_INCLUDES ${CG_FRAMEWORK_INCLUDES}
			${dir}/Headers ${dir}/PrivateHeaders)
		endforeach(dir)
		
		# Find the include  dir
		find_path(CG_INCLUDE_PATH cg.h ${CG_FRAMEWORK_INCLUDES})
		
		# Since we are using Cg framework, we must link to it.
		# Note, we use weak linking, so that it works even when Cg is not available.
		set(CG_LIBRARY "-weak_framework Cg" CACHE STRING "Cg library")
		set(CG_GL_LIBRARY "-weak_framework Cg" CACHE STRING "Cg GL library")
	endif(Cg_FRAMEWORKS)
	
	find_program(CG_COMPILER cgc
		/usr/bin
		/usr/local/bin
		DOC "The Cg compiler"
	)
else(APPLE)
	if(WIN32)
		find_program(CG_COMPILER cgc
			$ENV{CG_BIN_PATH}
			$ENV{PROGRAMFILES}/NVIDIA\ Corporation/Cg/bin
			$ENV{PROGRAMFILES}/Cg
			${PROJECT_SOURCE_DIR}/../Cg
			DOC "The Cg Compiler"
		)
		
		if(CG_COMPILER)
			get_filename_component(CG_COMPILER_DIR ${CG_COMPILER} PATH)
			get_filename_component(CG_COMPILER_SUPER_DIR ${CG_COMPILER_DIR} PATH)
		else(CG_COMPILER)
			set(CG_COMPILER_DIR .)
			set(CG_COMPILER_SUPER_DIR ..)
		endif(CG_COMPILER)
		
		set(
			CG_LIB_SEARCH_PATHS
			$ENV{PROGRAMFILES}/NVIDIA\ Corporation/Cg/lib
			$ENV{PROGRAMFILES}/Cg
			${PROJECT_SOURCE_DIR}/../Cg
			${CG_COMPILER_SUPER_DIR}/lib
			${CG_COMPILER_DIR}
		)
		
		find_path(CG_INCLUDE_PATH Cg/cg.h
			$ENV{CG_INC_PATH}
			$ENV{PROGRAMFILES}/NVIDIA\ Corporation/Cg/include
			$ENV{PROGRAMFILES}/Cg
			${PROJECT_SOURCE_DIR}/../Cg
			${CG_COMPILER_SUPER_DIR}/include
			${CG_COMPILER_DIR}
			DOC "The directory where Cg/cg.h resides"
		)
		find_library(CG_LIBRARY
			NAMES Cg
			PATHS
			$ENV{CG_LIB_PATH}
			$ENV{PROGRAMFILES}/NVIDIA\ Corporation/Cg/lib
			$ENV{PROGRAMFILES}/Cg
			${PROJECT_SOURCE_DIR}/../Cg
			${CG_COMPILER_SUPER_DIR}/lib
			${CG_COMPILER_DIR}
			DOC "The Cg runtime library"
		)
		find_library(CG_GL_LIBRARY NAMES CgGL PATHS ${CG_LIB_SEARCH_PATHS} DOC "The Cg runtime library for OpenGL")
		find_library(CG_D3D9_LIBRARY CgD3D9 PATHS ${CG_LIB_SEARCH_PATHS} DOC "The Cg runtime library for Direct3D9")
		find_library(CG_D3D11_LIBRARY CgD3D11 PATHS ${CG_LIB_SEARCH_PATHS} DOC "The Cg runtime library for Direct3D11")
	else(WIN32)
		find_program(CG_COMPILER cgc
			/usr/bin
			/usr/local/bin
			DOC "The Cg Compiler"
		)
		
		get_filename_component(CG_COMPILER_DIR "${CG_COMPILER}" PATH)
		get_filename_component(CG_COMPILER_SUPER_DIR "${CG_COMPILER_DIR}" PATH)
		
		set(
			CG_LIB_SEARCH_PATHS
			/usr/lib64
			/usr/lib
			/usr/local/lib64
			/usr/local/lib
			${CG_COMPILER_SUPER_DIR}/lib64
			${CG_COMPILER_SUPER_DIR}/lib
		)
		
		find_path(CG_INCLUDE_PATH Cg/cg.h
			/usr/include
			/usr/local/include
			${CG_COMPILER_SUPER_DIR}/include
			DOC "The directory where Cg/cg.h resides"
		)
		find_library(CG_LIBRARY Cg PATHS ${CG_LIB_SEARCH_PATHS} DOC "The Cg runtime library")
		set(CG_LIBRARY ${CG_LIBRARY} -lpthread)
		
		find_library(CG_GL_LIBRARY CgGL PATHS ${CG_LIB_SEARCH_PATHS} DOC "The Cg runtime library")
	endif(WIN32)
endif(APPLE)
