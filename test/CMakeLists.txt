cmake_minimum_required(VERSION 3.9)

find_package(Qt5Test REQUIRED)

macro(create_qtest name)
    add_executable(${name} ${name}.cpp)
    add_test(${name} ${name})
    target_include_directories (${name} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(${name} Qt5::Core Qt5::Widgets Qt5::Test)
    target_link_libraries(${name} ripes_lib)
endmacro()

# =============================================================================
# RISC-V Tests
# =============================================================================
option(VSRTL_ENABLE_RISCV_TESTS "Enable RISC-V unit tests" OFF)
if(VSRTL_ENABLE_RISCV_TESTS)
    # Ensure required RISC-V toolchain tools are available
    find_program(RV_AS riscv64-linux-gnu-as)
    if(NOT RV_AS)
        message(FATAL_ERROR "riscv64-linux-gnu-as not found!")
    endif()

    find_program(RV_OBJCOPY riscv64-linux-gnu-objcopy)
    if(NOT RV_OBJCOPY)
        message(FATAL_ERROR "riscv64-linux-gnu-objcopy not found!")
    endif()

    # Point to bundled tests within source directory
    set(VSRTL_RISCV_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/riscv-tests)
    add_definitions(-DVSRTL_RISCV_TEST_DIR="${VSRTL_RISCV_TEST_DIR}")
    create_qtest(tst_riscv)
endif()
