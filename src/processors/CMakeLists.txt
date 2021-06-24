cmake_minimum_required(VERSION 3.9)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Allow linking parent libraries with libraries defined in subdirectories
cmake_policy(SET CMP0079 NEW)

macro(create_isa_lib isa)
    file(GLOB LIB_SOURCES  ${isa}/*.cpp)
    file(GLOB LIB_HEADERS
        RipesVSRTLProcessor.h
        ripesvsrtlprocessor.h
        ${isa}/*.h)
    add_library(${isa}_lib ${LIB_SOURCES} ${LIB_HEADERS})
endmacro()

macro(create_vsrtl_processor isa name)
    file(GLOB LIB_SOURCES  ${isa}/${name}/*.cpp)
    file(GLOB LIB_HEADERS RipesVSRTLProcessor.h ${isa}/${name}/*.h)
    add_library(${name} ${LIB_SOURCES} ${LIB_HEADERS})
    target_link_libraries(${name} ${VSRTL_CORE_LIB} ${isa}_lib)
    target_include_directories(${name} PUBLIC ${isa})
    target_include_directories(${name} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    target_include_directories(${name} PUBLIC ${CMAKE_SOURCE_DIR}/external)
    target_compile_features(${name} PRIVATE cxx_std_17)
endmacro()

macro(create_verilator_processor name verilatorDir)
    file(GLOB LIB_SOURCES
        ${name}/*.cpp
        ${name}/${verilatorDir}/*.cpp
    )
    file(GLOB LIB_HEADERS
        ${name}/*.h
        ${name}/${subfolder}/*.h
    )

    add_library(${name} ${LIB_SOURCES} ${LIB_HEADERS})
    target_compile_features(${name} PRIVATE cxx_std_17)

    # Link processor against Verilator runtime library
    target_link_libraries(${name} ${VERILATOR_LIB})
    # Link Ripes lib against the processor library (Verilator processors are not header-only)
    target_link_libraries(${RIPES_LIB} ${name})

endmacro()

# RISC-V Processors
create_isa_lib(RISC-V)
create_vsrtl_processor(RISC-V rvss)
create_vsrtl_processor(RISC-V rv5s)
create_vsrtl_processor(RISC-V rv5s_no_fw_hz)
create_vsrtl_processor(RISC-V rv5s_no_hz)
create_vsrtl_processor(RISC-V rv5s_no_fw)
create_vsrtl_processor(RISC-V rv6s_dual)
