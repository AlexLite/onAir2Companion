cmake_minimum_required(VERSION 3.10)
project(onAir2Companion)

# Optional vcpkg support:
# pass -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake
# or define VCPKG_ROOT in environment.
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{VCPKG_ROOT})
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
endif()

find_package(nlohmann_json CONFIG QUIET)
if(NOT nlohmann_json_FOUND)
  include(FetchContent)
  FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
  )
  FetchContent_MakeAvailable(nlohmann_json)
endif()

add_executable(onAir2Companion src/server.cpp)
target_link_libraries(onAir2Companion PRIVATE nlohmann_json::nlohmann_json)

# Copy local runtime dependencies and default config next to the built exe.
# This keeps the server self-contained inside dev/server.
set(SOFTLAB_DIR "${CMAKE_SOURCE_DIR}/softlab")

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(SOFTLAB_ARCH_DIR "${SOFTLAB_DIR}/x64")
else()
  set(SOFTLAB_ARCH_DIR "${SOFTLAB_DIR}/x86")
endif()

if(EXISTS "${SOFTLAB_ARCH_DIR}/SLMessageQueue2.dll")
  add_custom_command(TARGET onAir2Companion POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${SOFTLAB_ARCH_DIR}/SLMessageQueue2.dll"
      "$<TARGET_FILE_DIR:onAir2Companion>/SLMessageQueue2.dll")
endif()

if(EXISTS "${SOFTLAB_ARCH_DIR}/SLMessageRemote2.dll")
  add_custom_command(TARGET onAir2Companion POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${SOFTLAB_ARCH_DIR}/SLMessageRemote2.dll"
      "$<TARGET_FILE_DIR:onAir2Companion>/SLMessageRemote2.dll")
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/config.example.json")
  add_custom_command(TARGET onAir2Companion POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${CMAKE_SOURCE_DIR}/config.example.json"
      "$<TARGET_FILE_DIR:onAir2Companion>/config.json")
endif()
