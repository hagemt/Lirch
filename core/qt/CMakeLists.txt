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
add_executable(lirch alirch_qtcore.cpp ${LIRCH_QTCORE_MOC})
target_link_libraries(lirch lirch-core ${QT_LIBRARIES})
install(TARGETS lirch DESTINATION bin)

if(BUILD_CLIENT_BASIC)
	# Build the core executable with QtCore and basic UI
	add_executable(blirch blirch_qtcore.cpp ${LIRCH_QTCORE_MOC})
	target_link_libraries(blirch lirch-core ${QT_LIBRARIES})
	install(TARGETS blirch DESTINATION bin)
endif(BUILD_CLIENT_BASIC)

if(BUILD_CLIENT_CURSES)
	# Build the core executable with QtCore and curses UI
	add_executable(clirch clirch_qtcore.cpp ${LIRCH_QTCORE_MOC})
	target_link_libraries(clirch lirch-core ${QT_LIBRARIES})
	install(TARGETS clirch DESTINATION bin)
endif(BUILD_CLIENT_CURSES)

if(BUILD_CLIENT_QTGUI)
	# Use QtCore, QtGui, and QtNetwork
	set(QT_DONT_USE_QTGUI FALSE)
	set(QT_USE_QTNETWORK TRUE)
	include("${QT_USE_FILE}")
	include_directories("${QT_INCLUDE_DIR}")
	include_directories("${QT_QTNETWORK_INCLUDE_DIR}")
	# Build the core executable with QtGUI and qt UI
	add_executable(qlirch qlirch_qtgui.cpp)
	target_link_libraries(qlirch lirch-core lirch-qt-interface)
	install(TARGETS qlirch DESTINATION bin)
endif(BUILD_CLIENT_QTGUI)
