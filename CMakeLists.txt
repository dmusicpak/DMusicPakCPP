cmake_minimum_required(VERSION 3.15)

# Set vcpkg toolchain file (must be before project() for proper integration)
# Can be overridden via command line: -DCMAKE_TOOLCHAIN_FILE=path/to/vcpkg.cmake
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "E:/Sources/open_library/vcpkg/scripts/buildsystems/vcpkg.cmake")
endif()

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
option(ENABLE_NETWORK "Enable network streaming support (requires libcurl)" OFF)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Library sources
set(DMUSICPAK_SOURCES
        src/dmusicpak.cpp
        src/io.cpp
)

# Network streaming support (optional)
if(ENABLE_NETWORK)
    # Try to find CURL using modern CMake (vcpkg or system package)
    # With vcpkg toolchain file, this should automatically find curl if installed
    find_package(CURL REQUIRED)
    
    # If we get here, CURL was found (REQUIRED ensures this)
    list(APPEND DMUSICPAK_SOURCES src/network.cpp)
    add_definitions(-DDMUSICPAK_ENABLE_NETWORK)
    message(STATUS "Network streaming enabled (libcurl found)")
    
    # CURL_FOUND is guaranteed to be TRUE when using REQUIRED
    set(CURL_FOUND TRUE)
endif()

set(DMUSICPAK_HEADERS
        include/dmusicpak/dmusicpak.h
)

# Windows DLL version resource
if(WIN32 AND BUILD_SHARED_LIBS)
    # Configure version information
    set(DMUSICPAK_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
    set(DMUSICPAK_VERSION_MINOR ${PROJECT_VERSION_MINOR})
    set(DMUSICPAK_VERSION_PATCH ${PROJECT_VERSION_PATCH})
    set(DMUSICPAK_VERSION_STRING "${PROJECT_VERSION}")
    set(DMUSICPAK_COMPANY_NAME "DMusicPak Project")
    set(DMUSICPAK_PRODUCT_NAME "DMusicPak")
    set(DMUSICPAK_FILE_DESCRIPTION "DMusicPak - Cross-platform Music Package Library")
    set(DMUSICPAK_COPYRIGHT "Copyright (C) 2025 DMusicPak Project")
    
    # Configure resource file
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/src/dmusicpak.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/dmusicpak.rc
        @ONLY
    )
    
    # Add resource file to sources
    list(APPEND DMUSICPAK_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/dmusicpak.rc)
endif()

# Create library
if(BUILD_SHARED_LIBS)
    add_library(dmusicpak SHARED ${DMUSICPAK_SOURCES} ${DMUSICPAK_HEADERS})
    target_compile_definitions(dmusicpak PRIVATE DMUSICPAK_EXPORTS)
    
    # Link libcurl if network support is enabled
    if(ENABLE_NETWORK AND CURL_FOUND)
        if(TARGET CURL::libcurl)
            # Modern CMake: use imported target
            target_link_libraries(dmusicpak PRIVATE CURL::libcurl)
        else()
            # Fallback: use variables
            target_link_libraries(dmusicpak PRIVATE ${CURL_LIBRARIES})
            target_include_directories(dmusicpak PRIVATE ${CURL_INCLUDE_DIRS})
        endif()
    endif()

    # Set version information
    set_target_properties(dmusicpak PROPERTIES
            VERSION ${PROJECT_VERSION}
            SOVERSION ${PROJECT_VERSION_MAJOR}
    )

    # Windows DLL naming and import library
    if(WIN32)
        set_target_properties(dmusicpak PROPERTIES
                OUTPUT_NAME "dmusicpak"
                PREFIX ""
        )
        # Explicitly set import library (.lib) output directory
        # The import library is automatically generated when building a DLL with MSVC
        set_target_properties(dmusicpak PROPERTIES
                ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
                ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/lib/Debug"
                ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/lib/Release"
                ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/lib/RelWithDebInfo"
                ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_BINARY_DIR}/lib/MinSizeRel"
        )
    endif()
else()
    add_library(dmusicpak STATIC ${DMUSICPAK_SOURCES} ${DMUSICPAK_HEADERS})
    
    # Link libcurl if network support is enabled
    if(ENABLE_NETWORK AND CURL_FOUND)
        if(TARGET CURL::libcurl)
            # Modern CMake: use imported target
            target_link_libraries(dmusicpak PRIVATE CURL::libcurl)
        else()
            # Fallback: use variables
            target_link_libraries(dmusicpak PRIVATE ${CURL_LIBRARIES})
            target_include_directories(dmusicpak PRIVATE ${CURL_INCLUDE_DIRS})
        endif()
    endif()
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
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/examples/CMakeLists.txt)
        add_subdirectory(examples)
    else()
        message(WARNING "Examples directory not found, disabling examples")
        set(BUILD_EXAMPLES OFF)
    endif()
endif()

# Build tests
if(BUILD_TESTS)
    enable_testing()
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt)
        add_subdirectory(tests)
    else()
        message(WARNING "Tests directory not found, disabling tests")
        set(BUILD_TESTS OFF)
    endif()
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
message(STATUS "  Network streaming: ${ENABLE_NETWORK}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")