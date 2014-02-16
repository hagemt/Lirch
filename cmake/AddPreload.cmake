macro(add_preload PLUGIN_NAME PRELOADS)
	# FIXME this needs to use find_library or something for portability
	set(LIBRARY_NAME "${LIBRARY_OUTPUT_PATH}/lib${PLUGIN_NAME}.so")
	set(${PRELOADS} "${${PRELOADS}}
	{ \"${PLUGIN_NAME}\", \"${LIBRARY_NAME}\" },")
	# FIXME hard-coding these paths also makes it impossible to relocate
endmacro(add_preload)

# Handle preload option additions (advanced)
macro(add_preload_option PLUGIN_NAME OPTION_STATE)
	string(TOUPPER "LIRCH_PRELOAD_${PLUGIN_NAME}" OPTION_NAME)
	set(OPTION_DOC "Preload the ${PLUGIN_NAME} plugin")
	# Preload defaults to OFF
	option(${OPTION_NAME} "${OPTION_DOC}" OFF)
	mark_as_advanced(${OPTION_NAME} FORCE)
	# Disabling the second argument triggers disabling the preload
	if (NOT ${OPTION_STATE})
		set(${OPTION_NAME} OFF CACHE BOOL ${OPTION_DOC} FORCE)
	endif(NOT ${OPTION_STATE})
	# Preloads might modify the configuration
	if(${OPTION_NAME})
		string(TOUPPER "LIRCH_PREPATH_${PLUGIN_NAME}" LIBRARY_NAME)
		# TODO add paths to facilitate finding the correct lib
		# FIXME this needs to use find_library or something for portability
		# set(${LIBRARY_NAME} "${LIBRARY_OUTPUT_PATH}/lib${PLUGIN_NAME}.so")
		# FIXME don't use LIBRARY_OUTPUT_PATH or don't use find_library
		find_library(${LIBRARY_NAME} ${PLUGIN_NAME} "${LIBRARY_OUTPUT_PATH}")
		mark_as_advanced(${LIBRARY_NAME} FORCE)
		# Provided this preload is found, add it to LIRCH_PRELOADS
		if(${LIBRARY_NAME})
			set(LIRCH_PRELOADS "${LIRCH_PRELOADS}
	{ \"${PLUGIN_NAME}\", \"${${LIBRARY_NAME}}\" },")
		endif(${LIBRARY_NAME})
	endif(${OPTION_NAME})
endmacro(add_preload_option)
