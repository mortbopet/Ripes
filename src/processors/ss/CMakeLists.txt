file(GLOB LIB_SOURCES *.cpp)
file(GLOB LIB_HEADERS *.h)

add_library(RISC-V ${LIB_SOURCES} ${LIB_HEADERS})
target_link_libraries(RISC-V ${VSRTL_CORE})
