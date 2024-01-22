cmake_minimum_required(VERSION 3.21)

# #######################################################################################################################
# # Define project
# #######################################################################################################################
set(NAME "PauseAfterLoadUnscripted")
set(VERSION 1.1.0)

project(
        ${NAME}
        VERSION ${VERSION}
        DESCRIPTION "SKSE/SKSEVR plugin that generates crash logs when the game Just Works™."
        LANGUAGES CXX)
set(PROJECT_FRIENDLY_NAME "PauseAfterLoadUnscripted NG")
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")

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
        src/PrecompiledHeaders.cpp
        src/PrecompiledHeaders.h
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
find_package(CommonLibSSE CONFIG REQUIRED)
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
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
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

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        Dbghelp.lib
)
target_precompile_headers(${PROJECT_NAME}
        PRIVATE
        src/PrecompiledHeaders.h)

#set_source_files_properties(
#    src/Utilities/utils.cpp
#    src/Utilities/version.cpp
#    PROPERTIES SKIP_PRECOMPILE_HEADERS ON)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/Sample"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

install(TARGETS ${PROJECT_NAME}
        DESTINATION "${CMAKE_INSTALL_LIBDIR}")

# #######################################################################################################################
# # Automatic plugin deployment
# #######################################################################################################################
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(BUILD_NAME "Debug")
elseif(CMAKE_BUILD_TYPE STREQUAL "Logging")
        set(BUILD_NAME "Logging")
else()
        set(BUILD_NAME "Release")
endif()

install(DIRECTORY DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/Papyrus${BUILD_NAME}/")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/Plugin${BUILD_NAME}/")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/Plugin${BUILD_NAME}/")

if(${CMAKE_BUILD_TYPE} STREQUAL Debug OR ${CMAKE_BUILD_TYPE} STREQUAL Logging OR ${CMAKE_BUILD_TYPE} STREQUAL Release)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_PDB_FILE:${PROJECT_NAME}> "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/Plugin${BUILD_NAME}/")
endif()

file(GLOB_RECURSE CONFIG_FILES "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/Config/*.yaml")
file(GLOB_RECURSE OUTPUT_DLLS "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/**/*.dll")
file(GLOB_RECURSE OUTPUT_PDBS "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/**/*.pdb")
file(GLOB_RECURSE OUTPUT_SCRIPTS "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/**/*.pex")
set_property(TARGET ${PROJECT_NAME}
        APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${OUTPUT_DLLS}")
set_property(TARGET ${PROJECT_NAME}
        APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${OUTPUT_PDBS}")
set_property(TARGET ${PROJECT_NAME}
        APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${OUTPUT_SCRIPTS}")

# Automatic deployment to Mod Organizer 2 mod directory.
foreach(DEPLOY_TARGET $ENV{SkyrimPluginTargets})
        message("Adding deployment target ${DEPLOY_TARGET}.")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> "${DEPLOY_TARGET}/SKSE/Plugins/")

        if(${CMAKE_BUILD_TYPE} STREQUAL Debug OR ${CMAKE_BUILD_TYPE} STREQUAL RelWithDebInfo OR ${CMAKE_BUILD_TYPE} STREQUAL Release)
                add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_PDB_FILE:${PROJECT_NAME}> "${DEPLOY_TARGET}/SKSE/Plugins/")
        endif()
endforeach()

set(ZIP_DIR "${CMAKE_CURRENT_BINARY_DIR}/zip")
add_custom_target(build-time-make-directory ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory "${ZIP_DIR}"
        "${ZIP_DIR}/SKSE/Plugins/"
)

message("Copying mod into ${ZIP_DIR}.")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> "${ZIP_DIR}/SKSE/Plugins/"
)

if(CONFIG_FILES)
        message("Copying default config files ${CONFIG_FILES} into ${ZIP_DIR}.")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy ${CONFIG_FILES} "${ZIP_DIR}/SKSE/Plugins/")
endif()

if(${CMAKE_BUILD_TYPE} STREQUAL Debug OR ${CMAKE_BUILD_TYPE} STREQUAL Logging OR ${CMAKE_BUILD_TYPE} STREQUAL Release)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_PDB_FILE:${PROJECT_NAME}> "${ZIP_DIR}/SKSE/Plugins/")
endif()

set(TARGET_ZIP "${PROJECT_NAME}_${PROJECT_VERSION}.7z")
message("Zipping ${ZIP_DIR} to ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_ZIP}.")
ADD_CUSTOM_COMMAND(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E tar cf ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_ZIP} --format=7zip -- .
        WORKING_DIRECTORY ${ZIP_DIR}
)
