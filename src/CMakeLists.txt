function(check_nonempty_string STR)
    if ("${STR}" STREQUAL "")
        message(FATAL_ERROR "Empty string")
    endif()
endfunction()

# Appends the .cpp and .ui files to the ripes library.
function(add_ripes_sources NAME)
    file(GLOB DIR_SOURCES *.cpp)
    file(GLOB DIR_UI *.ui)

    # Also pull in the includes from the correspoding directory in
    # ${CMAKE_SOURCE_DIR}/include. This is a bit antithetical to CMake
    # but is needed for Qt AUTOMOC to run.
    file(RELATIVE_PATH CURRENT_DIR_PATH_RELATIVE ${CMAKE_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR})
    file(GLOB DIR_INCLUDES ${CMAKE_SOURCE_DIR}/include/${CURRENT_DIR_PATH_RELATIVE}/*.h)
    target_sources(${RIPES_LIB} PUBLIC ${DIR_SOURCES} ${DIR_UI} ${DIR_INCLUDES})
endfunction()

# Error flags on everything but MSVC
if(NOT MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra -Wall \
        -Werror=switch -Werror=return-type \
        -Werror=unreachable-code")
elseif(MINGW)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wa")
endif()

# Create the parent library. This will include everything in the current directory.
add_library(${RIPES_LIB} STATIC)
target_include_directories(${RIPES_LIB} PUBLIC ${CMAKE_SOURCE_DIR}/include)
add_ripes_sources(${RIPES_LIB})

target_link_libraries(${RIPES_LIB} PUBLIC
    fancytabbar_lib
    ${VSRTL_GRAPHICS_LIB}
    Qt5::Charts
    dwarf++
    Qt5::Gui)

# Header-only library includes
target_include_directories(${RIPES_LIB} PUBLIC
    ${CMAKE_SOURCE_DIR}/external/VSRTL/external/cereal/include
    ${CMAKE_SOURCE_DIR}/external/VSRTL/external
    ${CMAKE_SOURCE_DIR}/external/ELFIO
    ${CMAKE_SOURCE_DIR}/external/VSRTL/external/Signals
    ${CMAKE_SOURCE_DIR}/external/libelfin_cmake
    ${CMAKE_SOURCE_DIR}/external
)

add_subdirectory(isa)
add_subdirectory(cachesim)
add_subdirectory(editor)
add_subdirectory(syscall)
add_subdirectory(assembler)
add_subdirectory(io)
add_subdirectory(utilities)
add_subdirectory(processors)
add_subdirectory(version)
add_subdirectory(cli)
