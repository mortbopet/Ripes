cmake_minimum_required(VERSION 3.9)

# Find Qt6 Core and Widgets
find_package(Qt6 COMPONENTS Core Widgets REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB LIB_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB LIB_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/fancytabbar/*.h)

add_library(fancytabbar STATIC ${LIB_SOURCES} ${LIB_HEADERS})
add_library(fancytabbar::fancytabbar ALIAS fancytabbar)
target_link_libraries(fancytabbar Qt6::Core Qt6::Widgets)
target_include_directories(fancytabbar PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
