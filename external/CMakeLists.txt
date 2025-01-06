######################################################################
## External project subdirectories
######################################################################

include(FetchContent)

# Fancy tab bar
add_subdirectory(fancytabbar)

# VSRTL
set(VSRTL_BUILD_APP OFF)
set(VSRTL_BUILD_TESTS OFF)
FetchContent_Declare(
  VSRTL
  GIT_REPOSITORY https://github.com/mortbopet/VSRTL.git
  GIT_TAG        master
)

FetchContent_MakeAvailable(VSRTL)

# libelfin
FetchContent_Declare(
    libelfin
    GIT_REPOSITORY https://github.com/mortbopet/libelfin.git
    GIT_TAG        93a424940dc6e721818c5634b1876c8800e3f980
)
FetchContent_MakeAvailable(libelfin)

# Make autoMOC and autoUIC happy about the generated libelfin files
set(libelfin_src "${CMAKE_CURRENT_BINARY_DIR}/libelfin")
set_property(SOURCE "${libelfin_src}/elf_to_string.cc" PROPERTY SKIP_AUTOGEN ON)
set_property(SOURCE "${libelfin_src}/dwarf_to_string.cc" PROPERTY SKIP_AUTOGEN ON)

# ELFIO
FetchContent_Declare(
    ELFIO
    GIT_REPOSITORY https://github.com/serge1/ELFIO.git
    GIT_TAG 79fcd11595d0793dec593e9072e36c7923c57c1a
)
FetchContent_MakeAvailable(ELFIO)
