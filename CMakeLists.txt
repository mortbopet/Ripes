cmake_minimum_required(VERSION 3.9)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
project(Ripes CXX)

######################################################################
## GUI setup
######################################################################

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find Qt5Core and Qt5Widgets
find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets CONFIG REQUIRED)

# Finding Qt includes
include_directories(${Qt5Widgets_INCLUDE_DIRS})
include_directories(${Qt5Core_INCLUDE_DIRS})

######################################################################
## Resources setup
######################################################################
qt5_add_resources(ICONS_SRC "resources/icons/icons.qrc")
qt5_add_resources(EXAMPLES_SRC "examples/examples.qrc")

######################################################################
## Library setup
######################################################################

# Platform specific pre-target setup(s)
if(${APPLE})
    set(SYSTEM_FLAGS MACOSX_BUNDLE)
endif()

add_subdirectory(external)
add_subdirectory(src)
add_subdirectory(test)

set(APP_NAME Ripes)
add_executable(${APP_NAME} ${SYSTEM_FLAGS} ${ICONS_SRC} ${EXAMPLES_SRC} main.cpp)

# Link Qt libraries
target_link_libraries(${APP_NAME} Qt5::Core Qt5::Widgets)
# Link Ripes library
target_link_libraries(${APP_NAME} ripes_lib)
