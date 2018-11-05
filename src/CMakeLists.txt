set(CMAKE_CXX_FLAGS "-Wall -Werror")
add_library(chip8 chip_8.cc decoder.cc)
add_executable(main main.cpp)
target_link_libraries(main chip8 sfml-graphics sfml-window sfml-system)


add_executable(debug debug.cc)
target_link_libraries(debug chip8)
