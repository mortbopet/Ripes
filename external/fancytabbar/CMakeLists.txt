cmake_minimum_required(VERSION 3.9)

# Find Qt6 Core and Widgets
find_package(Qt6 COMPONENTS Core Widgets REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB LIB_SOURCES *.cpp graphics/*.cpp)
file(GLOB LIB_HEADERS *.h graphics/*.h)
file(GLOB LIB_UIS *.ui)

set(FANCYTABBAR_LIB fancytabbar_lib)
add_library(${FANCYTABBAR_LIB} STATIC ${LIB_SOURCES} ${LIB_HEADERS} ${LIB_UIS})
target_link_libraries(${FANCYTABBAR_LIB} Qt6::Core Qt6::Widgets)
