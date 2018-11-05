cmake_minimum_required(VERSION 3.7)
project(chip8)

# FOR TESTING
add_subdirectory (googletest-release-1.7.0)
enable_testing()
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

set(CMAKE_CXX_STANDARD 14)

# for YouCompleteMe
SET( CMAKE_EXPORT_COMPILE_COMMANDS ON )
IF( EXISTS "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json" )
          EXECUTE_PROCESS( COMMAND ${CMAKE_COMMAND} -E copy_if_different
                      ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json
                      ${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json
         )
ENDIF()

enable_testing()
add_subdirectory(src)
add_subdirectory(test)
