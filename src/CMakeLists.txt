cmake_minimum_required(VERSION 3.9)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB LIB_SOURCES *.cpp graphics/*.cpp)
file(GLOB LIB_HEADERS *.h graphics/*.h)
file(GLOB LIB_UIS *.ui)

set(RIPES_LIB ripes_lib)
add_library(${RIPES_LIB} ${LIB_SOURCES} ${LIB_HEADERS} ${LIB_UIS})
target_include_directories (${RIPES_LIB} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
# External project includes
target_include_directories (${RIPES_LIB} PUBLIC ../external)

target_compile_features(${RIPES_LIB} PRIVATE cxx_std_17)

# Link external libraries
target_link_libraries(${RIPES_LIB} fancytabbar_lib)

add_subdirectory(processors)
