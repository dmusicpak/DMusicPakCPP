cmake_minimum_required(VERSION 3.15)
project(DMusicPak VERSION 1.0.1 LANGUAGES C CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Options
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(BUILD_EXAMPLES "Build example programs" ON)
option(BUILD_TESTS "Build test programs" OFF)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Library sources
set(DMUSICPAK_SOURCES
        src/dmusicpak.cpp
        src/io.cpp
)

set(DMUSICPAK_HEADERS
        include/dmusicpak/dmusicpak.h
)

# Create library
if(BUILD_SHARED_LIBS)
    add_library(dmusicpak SHARED ${DMUSICPAK_SOURCES} ${DMUSICPAK_HEADERS})
    target_compile_definitions(dmusicpak PRIVATE DMUSICPAK_EXPORTS)

    # Set version information
    set_target_properties(dmusicpak PROPERTIES
            VERSION ${PROJECT_VERSION}
            SOVERSION ${PROJECT_VERSION_MAJOR}
    )

    # Windows DLL naming
    if(WIN32)
        set_target_properties(dmusicpak PROPERTIES
                OUTPUT_NAME "dmusicpak"
                PREFIX ""
        )
    endif()
else()
    add_library(dmusicpak STATIC ${DMUSICPAK_SOURCES} ${DMUSICPAK_HEADERS})
endif()

# Include directories
target_include_directories(dmusicpak
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Compiler warnings
if(MSVC)
    target_compile_options(dmusicpak PRIVATE /W4)
else()
    target_compile_options(dmusicpak PRIVATE -Wall -Wextra -pedantic)
endif()

# Platform-specific settings
if(WIN32)
    target_compile_definitions(dmusicpak PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# Build examples
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Build tests
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Installation
include(GNUInstallDirs)

install(DIRECTORY include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(TARGETS dmusicpak
        EXPORT DMusicPakTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Export configuration
install(EXPORT DMusicPakTargets
        FILE DMusicPakTargets.cmake
        NAMESPACE DMusicPak::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/DMusicPak
)

# Create config file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/DMusicPakConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/DMusicPakConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/DMusicPakConfig.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/DMusicPak
)

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/DMusicPakConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/DMusicPakConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/DMusicPak
)

# Print configuration summary
message(STATUS "")
message(STATUS "DMusicPak ${PROJECT_VERSION} Configuration:")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Shared libs: ${BUILD_SHARED_LIBS}")
message(STATUS "  Examples: ${BUILD_EXAMPLES}")
message(STATUS "  Tests: ${BUILD_TESTS}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")