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
