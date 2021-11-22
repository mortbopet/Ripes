cmake_minimum_required(VERSION 3.9)

find_package(Qt5Test REQUIRED)

# Point to bundled tests within source directory
set(RISCV32_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/riscv-tests)
set(RISCV64_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/riscv-tests-64)
set(RISCV32_C_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/riscv-tests-c)
set(RISCV64_C_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/riscv-tests-c-64)
add_definitions(-DRISCV32_TEST_DIR="${RISCV32_TEST_DIR}")
add_definitions(-DRISCV64_TEST_DIR="${RISCV64_TEST_DIR}")
add_definitions(-DRISCV32_C_TEST_DIR="${RISCV32_C_TEST_DIR}")
add_definitions(-DRISCV64_C_TEST_DIR="${RISCV64_C_TEST_DIR}")

macro(create_qtest name)
    add_executable(${name} ${name}.cpp programloader.h)
    add_test(${name} ${name})
    target_include_directories (${name} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(${name} Qt5::Core Qt5::Widgets Qt5::Test)
    target_link_libraries(${name} ripes_lib)
endmacro()

create_qtest(tst_riscv)
create_qtest(tst_assembler)
create_qtest(tst_expreval)
create_qtest(tst_cosimulate)
create_qtest(tst_reverse)
