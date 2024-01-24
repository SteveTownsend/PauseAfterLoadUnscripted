include(FetchContent)

FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt
  GIT_TAG        10.2.1
  OVERRIDE_FIND_PACKAGE
)
if (NOT fmt_POPULATED)
        FetchContent_Populate(fmt)
        set(FMT_INSTALL ON CACHE INTERNAL "Install SPDLOG for CommonLibSSE")
        add_subdirectory(${fmt_SOURCE_DIR} ${fmt_BINARY_DIR})
endif()

FetchContent_Declare(
 spdlog
 GIT_REPOSITORY https://github.com/gabime/spdlog
 GIT_TAG        v1.13.0
 OVERRIDE_FIND_PACKAGE
)
FetchContent_GetProperties(spdlog)
if (NOT spdlog_POPULATED)
        FetchContent_Populate(spdlog)
        set(SPDLOG_INSTALL ON CACHE INTERNAL "Install SPDLOG for CommonLibSSE")
# pending update to spdlog v1.13.0 etc        
#        set(SPDLOG_USE_STD_FORMAT ON CACHE INTERNAL "Use std::format in SPDLOG, not fmt")
        add_subdirectory(${spdlog_SOURCE_DIR} ${spdlog_BINARY_DIR})
endif()

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2
  GIT_TAG        v3.5.2
  OVERRIDE_FIND_PACKAGE
  )
FetchContent_MakeAvailable(Catch2)

FetchContent_Declare(
  rapidcsv
  GIT_REPOSITORY https://github.com/d99kris/rapidcsv
  GIT_TAG        v8.64
  OVERRIDE_FIND_PACKAGE
  )
FetchContent_MakeAvailable(rapidcsv)
set(RAPIDCSV_INCLUDE_DIRS ${rapidcsv_SOURCE_DIR}/src)

# VR supported, but don't worry about tests
set(ENABLE_SKYRIM_VR ON)
set(BUILD_TESTS OFF)
FetchContent_Declare(
  CommonLibSSE
  GIT_REPOSITORY https://github.com/CharmedBaryon/CommonLibSSE-NG
  GIT_TAG        v3.7.0
  OVERRIDE_FIND_PACKAGE
)
FetchContent_MakeAvailable(CommonLibSSE)

include_directories(${fmt_SOURCE_DIR}/include)
target_compile_definitions(spdlog PUBLIC SPDLOG_FMT_EXTERNAL)
target_compile_options(spdlog PUBLIC "/I${fmt_SOURCE_DIR}/include")
target_compile_options(CommonLibSSE PUBLIC "/I${rapidcsv_SOURCE_DIR}/src")

find_package(spdlog CONFIG REQUIRED)
find_package(CommonLibSSE CONFIG REQUIRED)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_NO_BOOST_CMAKE ON)
find_package(Boost 1.84)
message(STATUS "Boost version: ${Boost_VERSION}")
