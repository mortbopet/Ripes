cmake_minimum_required(VERSION 3.9)

######################################################################
## External project subdirectories
######################################################################

# Fancy tab bar
add_subdirectory(fancytabbar)

# VSRTL
add_subdirectory(VSRTL)
set(VSRTL_BUILD_APP OFF)
set(VSRTL_BUILD_TESTS OFF)

add_subdirectory(libelfin)
# Make autoMOC and autoUIC happy about the generated libelfin files
set(libelfin_src "${CMAKE_CURRENT_BINARY_DIR}/libelfin")
set_property(SOURCE "${libelfin_src}/elf_to_string.cc" PROPERTY SKIP_AUTOGEN ON)
set_property(SOURCE "${libelfin_src}/dwarf_to_string.cc" PROPERTY SKIP_AUTOGEN ON)
