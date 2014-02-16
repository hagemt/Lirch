# For the minimal 'envelope' library:
add_subdirectory(envelope)

# For the core library:
add_library(lirch-core core.cpp plugin_loader.cpp)
target_link_libraries(lirch-core envelope dl pthread)

# TODO: re-evaluate this?
add_subdirectory(qt)
