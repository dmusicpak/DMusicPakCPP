# Examples CMakeLists.txt

# Write example
add_executable(example_write example_write.cpp)
target_link_libraries(example_write PRIVATE dmusicpak)

# Read example
add_executable(example_read example_read.cpp)
target_link_libraries(example_read PRIVATE dmusicpak)

# Stream example
add_executable(example_stream example_stream.cpp)
target_link_libraries(example_stream PRIVATE dmusicpak)

# Network example (only if network support is enabled)
if(ENABLE_NETWORK)
    add_executable(example_network example_network.cpp)
    target_link_libraries(example_network PRIVATE dmusicpak)
    if(CURL_FOUND)
        if(TARGET CURL::libcurl)
            # Modern CMake: use imported target
            target_link_libraries(example_network PRIVATE CURL::libcurl)
        else()
            # Fallback: use variables
            target_link_libraries(example_network PRIVATE ${CURL_LIBRARIES})
        endif()
    endif()
endif()

# Install examples (optional)
set(EXAMPLE_TARGETS example_write example_read example_stream)
if(ENABLE_NETWORK)
    list(APPEND EXAMPLE_TARGETS example_network)
endif()
install(TARGETS ${EXAMPLE_TARGETS}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/examples
)