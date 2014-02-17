# TODO(teh): write some of this into a macro? (needs DRYing up)
set(QT_DONT_USE_QTCORE FALSE)
set(QT_DONT_USE_QTGUI TRUE)
set(QT_USE_QTNETWORK FALSE)
set(QT_USE_QTTEST FALSE)
if(BUILD_TESTING)
	set(QT_USE_QTTEST TRUE)
endif(BUILD_TESTING)
include("${QT_USE_FILE}")
include_directories("${QT_INCLUDE_DIR}")

# This component is a mediator between Qt and the core:
qt4_wrap_cpp(LIRCH_QTCORE_MOC lirch_qtcore.h)

# Every UI uses Templatized C++ Command Line Parser Library (header-only)
include_directories("${CMAKE_SOURCE_DIR}/tclap/include")

# Build the core executable with QtCore and no UI
add_executable(alirch "${CMAKE_BINARY_DIR}/alirch.cpp" "${LIRCH_QTCORE_MOC}")
target_link_libraries(alirch lirch-core ${QT_LIBRARIES})
install(TARGETS alirch DESTINATION bin COMPONENT Binaries)
