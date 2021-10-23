cmake_minimum_required(VERSION 3.9)

# Find Qt5Core and Qt5Widgets
find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets CONFIG REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB LIB_SOURCES *.cpp graphics/*.cpp)
file(GLOB LIB_HEADERS *.h graphics/*.h)
file(GLOB LIB_UIS *.ui)

set(FANCYTABBAR_LIB fancytabbar_lib)
add_library(${FANCYTABBAR_LIB} STATIC ${LIB_SOURCES} ${LIB_HEADERS} ${LIB_UIS})
target_link_libraries(${FANCYTABBAR_LIB} Qt5::Core Qt5::Widgets)
