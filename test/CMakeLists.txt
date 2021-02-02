cmake_minimum_required(VERSION 3.9)

find_package(Qt5Test REQUIRED)

# Point to bundled tests within source directory
set(VSRTL_RISCV_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/riscv-tests)
add_definitions(-DVSRTL_RISCV_TEST_DIR="${VSRTL_RISCV_TEST_DIR}")

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
option(RIPES_ENABLE_RISCV_TESTS "Enable RISC-V unit tests" OFF)
if(RIPES_ENABLE_RISCV_TESTS)
    message(STATUS "RISC-V Tests enabled")
    message(STATUS "Locating required toolchain tools...")
    list (APPEND RV_TOOLS
        riscv64-unknown-elf-as
        riscv64-unknown-elf-objdump
        riscv64-unknown-elf-gcc
    )

    foreach(tool ${RV_TOOLS})
        # Ensure required RISC-V toolchain tools are available
        unset(RV_TOOL CACHE)
        find_program(RV_TOOL ${tool})
        if(NOT RV_TOOL)
            message(FATAL_ERROR "'${tool}' not found! A RISC-V toolchain must be available to build and run the RISC-V tests.")
        endif()
        message(STATUS "Found tool '${RV_TOOL}'")
    endforeach()

    create_qtest(tst_riscv)
    message(STATUS "RISC-V tests configured successfully")
endif()

create_qtest(tst_assembler)
create_qtest(tst_expreval)
