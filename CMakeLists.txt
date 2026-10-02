cmake_minimum_required(VERSION 3.16)

project(otlut VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(otlut_core
  src/lut_writer.cpp
)

target_include_directories(otlut_core PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_executable(otlut
  src/main.cpp
)

target_link_libraries(otlut PRIVATE otlut_core)

include(CTest)
if(BUILD_TESTING)
  add_executable(otlut_tests
    tests/test_lut_writer.cpp
  )
  target_link_libraries(otlut_tests PRIVATE otlut_core)
  add_test(NAME otlut_tests COMMAND otlut_tests)
endif()

install(TARGETS otlut RUNTIME DESTINATION bin)
