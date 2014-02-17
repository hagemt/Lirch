if(BUILD_CLIENT_BASIC)
	# Build the core executable with QtCore and basic UI
	add_executable(blirch "${CMAKE_BINARY_DIR}/blirch.cpp")
	target_link_libraries(blirch lirch-core lirch-core-qt ${QT_LIBRARIES})
	install(TARGETS blirch DESTINATION bin)
endif(BUILD_CLIENT_BASIC)

if(BUILD_CLIENT_CURSES)
	# Build the core executable with QtCore and curses UI
	add_executable(clirch "${CMAKE_BINARY_DIR}/clirch.cpp")
	target_link_libraries(clirch lirch-core lirch-core-qt ${QT_LIBRARIES})
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
	add_executable(qlirch "${CMAKE_BINARY_DIR}/qlirch.cpp")
	target_link_libraries(qlirch lirch-core lirch-qt-interface)
	install(TARGETS qlirch DESTINATION bin)
endif(BUILD_CLIENT_QTGUI)
