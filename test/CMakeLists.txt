cmake_minimum_required(VERSION 3.9)

file(GLOB TST_SOURCES *.cpp)

find_package(Qt5Test REQUIRED)

macro(create_qtest name)
    add_executable(${name} ${name}.cpp)
    add_test(${name} ${name})
    target_include_directories (${name} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(${name} Qt5::Core Qt5::Widgets Qt5::Test)
    target_link_libraries(${name} ripes_lib)
    target_link_libraries(${name} ${ICONS_SRC} ${EXAMPLES_SRC})
endmacro()

create_qtest(testall)
