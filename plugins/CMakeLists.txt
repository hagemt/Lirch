# Facilitate plug-in building TODO(teh): clean
macro(add_lirch_plugin NAME)
	set(QT_DONT_USE_QTCORE FALSE)
	set(QT_DONT_USE_QTGUI TRUE)
	if(BUILD_TESTING)
		set(QT_USE_QTTEST TRUE)
	endif(BUILD_TESTING)
	# Include all the correct headers:
	include(${QT_USE_FILE})
	include_directories(${QT_INCLUDE_DIR})
	if(QT_USE_QTNETWORK)
		include_directories(${QT_QTNETWORK_INCLUDE_DIR})
	endif(QT_USE_QTNETWORK)
	# Specify a shared library target for the plug-in:
	add_library(${NAME} SHARED ${ARGN})
	# Link with envelope and Qt libraries (as necessary)
	target_link_libraries(${NAME} envelope)
	target_link_libraries(${NAME} ${QT_LIBRARIES})
	if(QT_USE_QTNETWORK)
		target_link_libraries(${NAME} ${QT_QTNETWORK_LIBRARIES})
	endif(QT_USE_QTNETWORK)
	# Specify install (packaging) protocol
	install(TARGETS ${NAME} DESTINATION lib/lirch COMPONENT plugins)
	# TODO(teh): allow plugins to specify their own install?
endmacro(add_lirch_plugin)

# Base plugins:
add_subdirectory(antenna)
add_subdirectory(logger)
add_subdirectory(masseuse
add_subdirectory(meatgrinder)
add_subdirectory(quip)
add_subdirectory(userlist)

# Optional plugins:
if(BUILD_PLUGINS_EXTRA OR BUILD_TESTING)
	add_subdirectory(extra)
endif(BUILD_PLUGINS_EXTRA OR BUILD_TESTING)

# Future plugins:
if(BUILD_PLUGINS_USER)
	# Add your plugin's subdirectory here!
endif(BUILD_PLUGINS_USER)
