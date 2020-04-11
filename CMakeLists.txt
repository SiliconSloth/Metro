cmake_minimum_required (VERSION 3.10)
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
include(CheckCXXSymbolExists)
project (Metro)

# Module seatch path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

# Use environmental variable to find OpenSSL
set(OPENSSL_ROOT_DIR $ENV{OPENSSL_ROOT_DIR})

IF (WIN32)
    set(EXT ".lib")
ENDIF()
IF (UNIX)
    set(EXT ".a")
ENDIF()

# LibGit2 Options
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build Static Library" FORCE)      # Force static library
set(USE_SSH OFF CACHE BOOL "Manual SSH entry" FORCE)                    # Enter SSH manually
set(LIBSSH2_FOUND 1)                                                    # Manual SSH directory settings
set(LIBSSH2_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/deps/libssh2/include")
set(LIBSSH2_LIBRARIES "${CMAKE_BINARY_DIR}/libssh2/src/libssh2${EXT}")
set(LIBSSH2_LDFLAGS "")
set(USE_BUNDLED_ZLIB ON CACHE BOOL "Use Bundled ZLib" FORCE)            # Want to ensure bundled zlib is used
set(BUILD_CLAR OFF CACHE BOOL "Disables Tests" FORCE)                   # Disable tests - no python required

# Build dependencies
add_subdirectory("deps/libssh2" libssh2)
add_subdirectory("deps/libgit2" libgit2)
include_directories("deps/libgit2/include")

# Include directory and start file for metro
include_directories("include")
add_executable(metro src/main.cpp)

IF (WIN32)
    # On Windows SSL must be installed/build manually
    IF (DEFINED OPENSSL_ROOT_DIR)
        add_library(ssl STATIC IMPORTED)
        set_target_properties(ssl PROPERTIES IMPORTED_LOCATION "${OPENSSL_ROOT_DIR}/libssl.lib")
        add_library(crypto STATIC IMPORTED)
        set_target_properties(crypto PROPERTIES IMPORTED_LOCATION "${OPENSSL_ROOT_DIR}/libcrypto.lib")
    ENDIF()
    target_link_libraries(metro git2 libssh2 ssl crypto winhttp Rpcrt4 crypt32)
ENDIF()
IF (UNIX)
    IF (APPLE)
        target_link_libraries(metro git2 pthread crypto libssh2 iconv "-framework CoreFoundation" "-framework Security")
    ELSE()
        target_link_libraries(metro git2 pthread ssl crypto libssh2 stdc++fs)
        IF (HTTP_PARSER_FOUND)
            target_link_libraries(metro http_parser)
        ENDIF()
    ENDIF()
    IF (PCRE2_FOUND)
        target_link_libraries(metro pcre)
    ENDIF()
ENDIF()
