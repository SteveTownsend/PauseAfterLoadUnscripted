cmake_minimum_required(VERSION 3.27)

# #######################################################################################################################
# # Define project
# #######################################################################################################################
set(NAME "PauseAfterLoadUnscripted")
set(PRETTY_NAME "Pause After Load Unscripted SE")
set(VERSION 1.2.0)

message("${CMAKE_SIZEOF_VOID_P}")
set(spdlog_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

project(
        ${NAME}
        VERSION ${VERSION}
        DESCRIPTION "SKSE/SKSEVR plugin pauses the game after load screen to avoid entirely predictable swift death."
        LANGUAGES CXX)
set(PROJECT_FRIENDLY_NAME "PauseAfterLoadUnscripted NG")
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")

set(BUILD_SHARED_LIBS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/version.rc
        @ONLY)

set(sources
        src/Data/SettingsCache.cpp
        src/Data/SettingsCache.h
        src/Data/SimpleIni.cpp
        src/Data/SimpleIni.h
        src/Pausing/InputListener.h
        src/Pausing/PauseHandler.h
        src/Relocation/Hooks.cpp
        src/Relocation/Hooks.h
        src/Utilities/LogStackWalker.cpp
        src/Utilities/LogStackWalker.h
        src/Utilities/LogWrapper.h
        src/Utilities/RecursiveLock.cpp
        src/Utilities/RecursiveLock.h
        src/Utilities/StackWalker.cpp
        src/Utilities/StackWalker.h
        src/Utilities/utils.cpp
        src/Utilities/utils.h
        src/Utilities/version.cpp
        src/Utilities/version.h
        src/main.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/version.rc
)

source_group(
        TREE ${CMAKE_CURRENT_SOURCE_DIR}
        FILES
        ${sources}
)

# #######################################################################################################################
# # Add CMake features
# #######################################################################################################################
include(CheckIPOSupported)
include(GNUInstallDirs)

# #######################################################################################################################
# # Find dependencies
# #######################################################################################################################
include(paluDependencies.cmake)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# #######################################################################################################################
# # Configuration for all targets
# #######################################################################################################################
if(WIN32)
        add_compile_definitions(
                UNICODE
                _UNICODE
                NOMINMAX
                _AMD64_
                WIN32_LEAN_AND_MEAN)
        if(${CMAKE_BUILD_TYPE} STREQUAL RelWithDebInfo)
                add_compile_definitions(
                        _FULL_LOGGING
                )
        endif()
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL Clang)
        add_compile_definitions(
                __cpp_lib_char8_t
                __cpp_consteval
                __cpp_lib_format)
endif()

check_ipo_supported(RESULT USE_IPO OUTPUT IPO_OUTPUT)

if(USE_IPO)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
else()
        message(WARNING "Interprocedural optimizations are not supported.")
endif()

# #######################################################################################################################
# # Configure target DLL
# #######################################################################################################################
include(${commonlibsse_SOURCE_DIR}/cmake/CommonLibSSE.cmake)
add_commonlibsse_plugin(${PROJECT_NAME}
    NAME ${PROJECT_NAME}
    AUTHOR "Steve Townsend, wskeever, bobbyclue"
    EMAIL "SteveTownsend0@gmail.com"
    USE_ADDRESS_LIBRARY
    USE_SIGNATURE_SCANNING
    SOURCES ${sources}
)

add_library("${PROJECT_NAME}::${PROJECT_NAME}" ALIAS "${PROJECT_NAME}")

target_include_directories(${PROJECT_NAME}
        PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/src>
        $<INSTALL_INTERFACE:src>)

target_include_directories(${PROJECT_NAME}
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

target_include_directories(${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        CommonLibSSE
        Dbghelp.lib
        spdlog
)
target_precompile_headers(${PROJECT_NAME}
        PRIVATE
        src/PrecompiledHeaders.h)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/Sample"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

install(TARGETS ${PROJECT_NAME}
        DESTINATION "${CMAKE_INSTALL_LIBDIR}")

# #######################################################################################################################
# # Build plugin archive for Nexus, for Release only
# #######################################################################################################################
if (${CMAKE_BUILD_TYPE} STREQUAL Release)
        include(paluBuildArchive.cmake)
endif()
