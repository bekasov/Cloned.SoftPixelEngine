
# Custom CMake module for finding "SoftPixel Engine SDK" files
# Written by Lukas Hermanns on 25/07/2012

set(SoftPixelEngine_ROOT_DIR ${PROJECT_SOURCE_DIR}/../SoftPixelEngine)
set(SoftPixelEngine_LIB_ROOT_DIR ${SoftPixelEngine_ROOT_DIR}/cmake/build)

# Macros

macro(_SOFTPIXELENGINE_APPEND_LIBRARIES _list _release)
	set(_debug ${_release}_DEBUG)
	if(${_debug})
		set(${_list} ${${_list}} optimized ${${_release}} debug ${${_debug}})
	else()
		set(${_list} ${${_list}} ${${_release}})
	endif()
endmacro()

# Find library

find_path(
	SoftPixelEngine_INCLUDE_DIR
	NAMES
		SoftPixelEngine.hpp
	HINTS
		${SoftPixelEngine_ROOT_DIR}/repository/sources
		${SoftPixelEngine_ROOT_DIR}/sources
)

find_library(
	SoftPixelEngine_LIBRARY
	NAMES
		SoftPixelEngine
	HINTS
		${SoftPixelEngine_LIB_ROOT_DIR}/lib/Release
)

find_library(
	SoftPixelEngine_LIBRARY_DEBUG
	NAMES
		SoftPixelEngine
	HINTS
		${SoftPixelEngine_LIB_ROOT_DIR}/lib/Debug
)

# Setup package handle

include(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(
	SoftPixelEngine
	DEFAULT_MSG
	SoftPixelEngine_LIBRARY
	SoftPixelEngine_LIBRARY_DEBUG
	SoftPixelEngine_INCLUDE_DIR
)

if(SOFTPIXELENGINE_FOUND)
	set(SoftPixelEngine_FOUND TRUE)
	_SOFTPIXELENGINE_APPEND_LIBRARIES(SoftPixelEngine_LIBRARIES SoftPixelEngine_LIBRARY)
endif(SOFTPIXELENGINE_FOUND)
