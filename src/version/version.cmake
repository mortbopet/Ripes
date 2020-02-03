# Modified version of:
# https://www.mattkeeter.com/blog/2018-01-06-versioning/

set(ENV{GIT_DIR} ${RIPES_SRC_DIR}/.git)
execute_process(COMMAND git describe --tags
                OUTPUT_VARIABLE GIT_REV
                ERROR_QUIET)

# Check whether we got any revision (which isn't
# always the case, e.g. when someone downloaded a zip
# file from Github instead of a checkout)
if ("${GIT_REV}" STREQUAL "")
    set(RIPES_GIT_VERSION "N/A")
else()
    string(STRIP "${GIT_REV}" RIPES_GIT_VERSION)
endif()

set(VERSION "const char* RIPES_GIT_VERSION=\"${RIPES_GIT_VERSION}\";")

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/version.cpp)
    file(READ ${CMAKE_CURRENT_SOURCE_DIR}/version.cpp VERSION_)
else()
    set(VERSION_ "")
endif()

if (NOT "${VERSION}" STREQUAL "${VERSION_}")
    file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/version/version.cpp "${VERSION}")
endif()
