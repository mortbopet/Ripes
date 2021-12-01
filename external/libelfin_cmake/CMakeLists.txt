cmake_minimum_required(VERSION 3.1)
project(libelfin CXX)

if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 11)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PythonInterp 3 REQUIRED)
set(source_subfolder "${CMAKE_CURRENT_SOURCE_DIR}/libelfin")

file(GLOB_RECURSE elf_sources ${source_subfolder}/elf/*.cc)
set(elf_headers
    ${source_subfolder}/elf/common.hh
    ${source_subfolder}/elf/data.hh
    ${source_subfolder}/elf/elf++.hh)
file(GLOB_RECURSE dwarf_sources ${source_subfolder}/dwarf/*.cc)
set(dwarf_headers
    ${source_subfolder}/dwarf/data.hh
    ${source_subfolder}/dwarf/dwarf++.hh
    ${source_subfolder}/dwarf/small_vector.hh)

add_custom_command(
    OUTPUT ${source_subfolder}/elf/to_string.cc
    COMMAND ${CMAKE_COMMAND} -E echo '// Automatically generated' > to_string.cc
    COMMAND ${CMAKE_COMMAND} -E echo '// DO NOT EDIT' >> to_string.cc
    COMMAND ${CMAKE_COMMAND} -E echo >> to_string.cc
    COMMAND ${CMAKE_COMMAND} -E echo '\#include \"data.hh\"' >> to_string.cc
    COMMAND ${CMAKE_COMMAND} -E echo '\#include \"to_hex.hh\"' >> to_string.cc
    COMMAND ${CMAKE_COMMAND} -E echo >> to_string.cc
    COMMAND ${CMAKE_COMMAND} -E echo 'ELFPP_BEGIN_NAMESPACE' >> to_string.cc
    COMMAND ${CMAKE_COMMAND} -E echo >> to_string.cc
    COMMAND ${PYTHON_EXECUTABLE} enum-print.py -u --hex --no-type --mask shf --mask pf -x loos -x hios -x loproc -x hiproc < data.hh >> to_string.cc
    COMMAND ${CMAKE_COMMAND} -E echo 'ELFPP_END_NAMESPACE' >> to_string.cc
    DEPENDS ${source_subfolder}/elf/enum-print.py ${source_subfolder}/elf/data.hh
    WORKING_DIRECTORY ${source_subfolder}/elf)

add_custom_command(
    OUTPUT ${source_subfolder}/dwarf/to_string.cc
    COMMAND ${CMAKE_COMMAND} -E echo '// Automatically generated' > to_string.cc
    COMMAND ${CMAKE_COMMAND} -E echo '// DO NOT EDIT' >> to_string.cc
    COMMAND ${CMAKE_COMMAND} -E echo >> to_string.cc
    COMMAND ${CMAKE_COMMAND} -E echo '\#include \"internal.hh\"' >> to_string.cc
    COMMAND ${CMAKE_COMMAND} -E echo >> to_string.cc
    COMMAND ${CMAKE_COMMAND} -E echo 'DWARFPP_BEGIN_NAMESPACE' >> to_string.cc
    COMMAND ${CMAKE_COMMAND} -E echo >> to_string.cc
    COMMAND ${PYTHON_EXECUTABLE} ../elf/enum-print.py < dwarf++.hh >> to_string.cc
    COMMAND ${PYTHON_EXECUTABLE} ../elf/enum-print.py -s _ -u --hex -x hi_user -x lo_user < data.hh >> to_string.cc
    COMMAND ${CMAKE_COMMAND} -E echo 'DWARFPP_END_NAMESPACE' >> to_string.cc
    DEPENDS ${source_subfolder}/elf/enum-print.py ${source_subfolder}/dwarf/data.hh
    WORKING_DIRECTORY ${source_subfolder}/dwarf)

# Uncomment this if/when switching to elf++ instead of libelf.
# add_library(elf++ STATIC ${elf_sources} ${source_subfolder}/elf/to_string.cc)
# set_target_properties(elf++ PROPERTIES
#     PUBLIC_HEADER "${elf_headers}")

add_library(dwarf++ STATIC ${dwarf_sources} ${source_subfolder}/dwarf/to_string.cc)
set_target_properties(dwarf++ PROPERTIES
    PUBLIC_HEADER "${dwarf_headers}")

# Make autoMOC and autoUIC happy
set_property(SOURCE ${source_subfolder}/elf/to_string.cc PROPERTY SKIP_AUTOGEN ON)
set_property(SOURCE ${source_subfolder}/dwarf/to_string.cc PROPERTY SKIP_AUTOGEN ON)
