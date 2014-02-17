# Base plugins:
add_subdirectory(antenna)
add_subdirectory(logger)
add_subdirectory(masseuse)
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
