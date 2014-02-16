# Include some directories by default:
include_directories("${CMAKE_BINARY_DIR}")
include_directories("${CMAKE_SOURCE_DIR}")

# CMake picks up on {EXECUTABLE,LIBRARY}_OUTPUT_PATH:
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin"    )
set(LIBRARY_OUTPUT_PATH    "${CMAKE_BINARY_DIR}/lib"    )
set(BASE_INCLUDE_PATH      "${CMAKE_SOURCE_DIR}/include")

# FIXME(teh): is any of this a good idea?
if(EXISTS "${BASE_INCLUDE_PATH}")
	include_directories("${BASE_INCLUDE_PATH}")
else(EXISTS "${BASE_INCLUDE_PATH}")
	message(WARNING "BASE_INCLUDE_PATH: ${BASE_INCLUDE_PATH} does not exist!")
endif(EXISTS "${BASE_INCLUDE_PATH}")

#if(EXISTS "${BASE_INCLUDE_PATH}/${CMAKE_PROJECT_NAME}")
#	include_directories("${BASE_INCLUDE_PATH}/${CMAKE_PROJECT_NAME}")
#endif(EXISTS "${BASE_INCLUDE_PATH}/${CMAKE_PROJECT_NAME}")
