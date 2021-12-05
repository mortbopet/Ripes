function(check_nonempty_string STR)
    if ("${STR}" STREQUAL "")
        message(FATAL_ERROR "Empty string")
    endif()
endfunction()

# Function to create sub-libraries for the Ripes library. A library is
# built based on the *.h,*.cpp and *.ui within the immediate directory of
# the CMakeLists.txt file. If LINK_TO_RIPES_LIB is set, the ${RIPES_LIB}
# will be linked to the newly defined library.
function(create_ripes_lib NAME)
    cmake_parse_arguments(OPTIONS
        "LINK_TO_RIPES_LIB;FIXED_NAME;EXCLUDE_SRC_INC" # options
        ""                             # 1-valued keywords
        ""                             # multi-valued keywords
        ${ARGN})

    file(GLOB LIB_SOURCES *.cpp)
    file(GLOB LIB_HEADERS *.h)
    file(GLOB LIB_UI *.ui)
    if(NOT OPTIONS_FIXED_NAME)
        check_nonempty_string(NAME)
        set(LIB_NAME ${NAME}_lib)
    else()
        check_nonempty_string(OPTIONS_FIXED_NAME)
        set(LIB_NAME ${NAME})
    endif()

    add_library(${LIB_NAME} STATIC ${LIB_SOURCES} ${LIB_HEADERS} ${LIB_UI})
    target_compile_features(${LIB_NAME} PRIVATE cxx_std_17)
    target_include_directories (${LIB_NAME} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        )

    target_link_libraries(${LIB_NAME} PUBLIC Qt5::Gui)
    if(NOT OPTIONS_EXCLUDE_SRC_INC)
        target_include_directories(${LIB_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/src)
    endif()

    if(OPTIONS_LINK_TO_RIPES_LIB)
        target_link_libraries(${RIPES_LIB} PUBLIC ${LIB_NAME})
    endif()
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
create_ripes_lib(${RIPES_LIB} FIXED_NAME EXCLUDE_SRC_INC)

# All of the following subdirectories will create separate libraries and link them into
# ripes_lib
add_subdirectory(isa)
add_subdirectory(cachesim)
add_subdirectory(editor)
add_subdirectory(syscall)
add_subdirectory(assembler)
add_subdirectory(io)
add_subdirectory(utilities)
add_subdirectory(processors)
add_subdirectory(version)

# Also link Qt and VSRTL libraries.
target_link_libraries(${RIPES_LIB} PUBLIC
    fancytabbar_lib
    ${VSRTL_GRAPHICS_LIB}
    Qt5::Charts
    dwarf++)

