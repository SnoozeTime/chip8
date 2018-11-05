include_directories(${PROJECT_SOURCE_DIR}/src)

function (add_chip8_test test_class)
    add_executable(${test_class} ${test_class}.cc)
    target_link_libraries(${test_class} chip8 gtest gtest_main)
    add_test(${test_class} ${test_class})
endfunction()

add_chip8_test(opcode_test)
