cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
project(Lirch C CXX)
set(PROJECT_VERSION "0.1.1.0")
include(cmake/VersionInfo.cmake)
include(cmake/Directories.cmake)
include(cmake/BuildType.cmake)
include(cmake/BuildSlug.cmake)
include(cmake/AddPreload.cmake)
include(cmake/AddPlugin.cmake)

## CONFIGURATION

# Configure some string literals and a lirch w/o preloads
set(LIRCH_LOG_DIRECTORY "${CMAKE_BINARY_DIR}/log" CACHE STRING
	"Logging is configurable, but when enabled, default here")
file(MAKE_DIRECTORY "${LIRCH_LOG_DIRECTORY}")
configure_file(include/lirch/config.h.in   "lirch_constants.h")
configure_file(core/qt/lirch_qtcore.cpp.in "alirch.cpp")

# Configurable build options TODO(teh): figure out requirements for testing
option(LIRCH_CLIENT_BASIC  "Build the basic (raw input) UI" OFF)
option(LIRCH_CLIENT_CURSES "Build the terminal (curses) UI" OFF)
option(LIRCH_CLIENT_QTGUI  "Build the graphical (QtGui) UI" ON )
option(LIRCH_PLUGINS_EXTRA "Build any non-standard plugins" OFF)
mark_as_advanced(LIRCH_PLUGINS_EXTRA FORCE)

if(BUILD_TESTING)
	set(LIRCH_PLUGINS_EXTRA ON)
endif(BUILD_TESTING)

# Optionally pre-load these plug-ins, mostly for testing purposes:
add_preload_option("timer"          LIRCH_PLUGINS_EXTRA)
add_preload_option("linuxtester"    LIRCH_PLUGINS_EXTRA)
add_preload_option("settingstester" LIRCH_PLUIGNS_EXTRA)

# Each client requires certain preloads, configure these:
if(LIRCH_CLIENT_BASIC OR LIRCH_CLIENT_CURSES OR LIRCH_CLIENT_QTGUI)
	# Load all standard plugins into every client
	set(CLIENT_PRELOADS)
	add_preload("antenna"     CLIENT_PRELOADS)
	add_preload("logger"      CLIENT_PRELOADS)
	add_preload("meatgrinder" CLIENT_PRELOADS)
	add_preload("masseuse"    CLIENT_PRELOADS)
	add_preload("quip"        CLIENT_PRELOADS)
	add_preload("userlist"    CLIENT_PRELOADS)
	# For each UI (reset preloads -> add_preload -> configure_file)
	if(LIRCH_CLIENT_BASIC)
	set(LIRCH_CLIENT_PRELOADS "${CLIENT_PRELOADS}")
		add_preload("lirch-basic" LIRCH_CLIENT_PRELOADS)
		configure_file(core/qt/lirch_qtcore.cpp.in blirch.cpp)
	endif(LIRCH_CLIENT_BASIC)
	if(LIRCH_CLIENT_CURSES)
		set(LIRCH_CLIENT_PRELOADS "${CLIENT_PRELOADS}")
		add_preload("lirch-curses" LIRCH_CLIENT_PRELOADS)
		configure_file(core/qt/lirch_qtcore.cpp.in clirch.cpp)
	endif(LIRCH_CLIENT_CURSES)
	if(LIRCH_CLIENT_QTGUI)
		set(LIRCH_CLIENT_PRELOADS "${CLIENT_PRELOADS}")
		add_preload("lirch-gui" LIRCH_CLIENT_PRELOADS)
		configure_file(core/qt/lirch_qtgui.cpp.in qlirch.cpp)
	endif(LIRCH_CLIENT_QTGUI)
endif(LIRCH_CLIENT_BASIC OR LIRCH_CLIENT_CURSES OR LIRCH_CLIENT_QTGUI)

## DEPENDENCIES (LIBRARIES)

# At some point maybe we'll consider supporting compilers that aren't gcc
if(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -Wextra")
	# TODO when does this become unnecessary?
	add_definitions(-D_GLIBCXX_USE_NANOSLEEP)
else(CMAKE_COMPILER_IS_GNUCXX)
	message(WARNING "You need to use gcc 4.6.0+ to compile lirch.")
endif(CMAKE_COMPILER_IS_GNUCXX)
# Find the libraries we'll be needing (for core)
find_library(PTHREAD_LIBRARY_PATH pthread REQUIRED)
if(UNIX)
	# FIXME(teh): this is *nix only
	find_library(DL_LIBRARY_PATH dl REQUIRED)
endif(UNIX)

# (Almost) everything (except core) needs QtCore
list(APPEND QT_COMPONENTS "QtCore")
# The antenna (+ anything else?) needs QtNetwork
list(APPEND QT_COMPONENTS "QtNetwork")
# Only the Qt GUI needs QtGui
if(LIRCH_CLIENT_QTGUI)
	list(APPEND QT_COMPONENTS "QtGui")
endif(LIRCH_CLIENT_QTGUI)
# Only need QtTest for testing
if(BUILD_TESTING)
	list(APPEND QT_COMPONENTS "QtTest")
endif(BUILD_TESTING)
# Figure out which Qt components to load
if(QT_COMPONENTS)
	set(QT_MIN_VERSION 4.8.0)
	find_package(Qt4 ${QT_MIN_VERSION} COMPONENTS ${QT_COMPONENTS} REQUIRED)
endif(QT_COMPONENTS)

## COMPILATION

add_subdirectory(core)
add_subdirectory(plugins)

if(LIRCH_CLIENT_BASIC)
	add_subdirectory(ui/basic)
endif(LIRCH_CLIENT_BASIC)

if(LIRCH_CLIENT_CURSES)
	add_subdirectory(ui/curses)
endif(LIRCH_CLIENT_CURSES)

if(LIRCH_CLIENT_QTGUI)
	add_subdirectory(ui/qt)
endif(LIRCH_CLIENT_QTGUI)

## TESTING

include(CTest)
if(BUILD_TESTING)
	set(CTEST_PROJECT_NAME "${CMAKE_PROJECT_NAME}")
	add_subdirectory(test)
endif(BUILD_TESTING)

## PACKAGING

# TODO how to get `make package` and `cpack` both to work?
set(FROM_CMAKE_BUILD ON CACHE INTERNAL "Tag for make package" FORCE)
# Initialize a global configuration
set(CPACK_GENERATOR "STGZ;TGZ;TBZ2;TZ") # FIXME why is ZIP broken?
set(CPACK_PROJECT_CONFIG_FILE "${CMAKE_SOURCE_DIR}/CPackConfig.cmake")
if(UNIX)
	list(APPEND CPACK_GENERATOR "DEB") # TODO also support RPM/PKGBUILD?
endif(UNIX)
if(WIN32 AND NOT UNIX)
	list(APPEND CPACK_GENERATOR "NSIS") # TODO after Windows build support
endif(WIN32 AND NOT UNIX)
include("${CPACK_PROJECT_CONFIG_FILE}")

# Initialize source packaging (for future use)
#set(CPACK_SOURCE_PROJECT_CONFIG_FILE
#	"${CMAKE_SOURCE_DIR}/CPackSourceConfig.cmake")
#include("${CPACK_SOURCE_PROJECT_CONFIG_FILE}")
# Include the proper modules for packaging
include(CPack)
