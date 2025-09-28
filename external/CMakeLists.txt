######################################################################
## External project subdirectories
######################################################################

include(FetchContent)

# Fancy tab bar
add_subdirectory(fancytabbar)

# VSRTL
set(VSRTL_BUILD_APP OFF)
set(VSRTL_BUILD_TESTS OFF)
add_subdirectory(VSRTL SYSTEM)

# libelfin
FetchContent_Declare(
    libelfin
    SYSTEM
    GIT_REPOSITORY https://github.com/mortbopet/libelfin.git
    GIT_TAG        c06ecb09c0d2145091119d15b911c54d2d378ea7
)
FetchContent_MakeAvailable(libelfin)

# Make autoMOC and autoUIC happy about the generated libelfin files
set(libelfin_src "${CMAKE_CURRENT_BINARY_DIR}/libelfin")
set_property(SOURCE "${libelfin_src}/elf_to_string.cc" PROPERTY SKIP_AUTOGEN ON)
set_property(SOURCE "${libelfin_src}/dwarf_to_string.cc" PROPERTY SKIP_AUTOGEN ON)

# ELFIO
FetchContent_Declare(
    ELFIO
    SYSTEM
    GIT_REPOSITORY https://github.com/serge1/ELFIO.git
    GIT_TAG 79fcd11595d0793dec593e9072e36c7923c57c1a
)
FetchContent_MakeAvailable(ELFIO)
