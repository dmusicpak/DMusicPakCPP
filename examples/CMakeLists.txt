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

# Install examples (optional)
install(TARGETS example_write example_read example_stream
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/examples
)