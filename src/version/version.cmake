set(ENV{GIT_DIR} ${RIPES_SRC_DIR}/.git)
execute_process(COMMAND git describe --tags --exclude "continuous" 
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

set(VERSION_FILE ${RIPES_SRC_DIR}/src/version/gen_versionnumber.h)

# New version file content
set(VERSION "#define RIPES_GIT_VERSION \"${RIPES_GIT_VERSION}\"")

# Only rewrite the version file if a change is seen
if(EXISTS ${VERSION_FILE})
    file(READ ${VERSION_FILE} OLD_VERSION)
else()
    set(OLD_VERSION "")
endif()

if (NOT "${VERSION}" STREQUAL "${OLD_VERSION}")
    file(WRITE ${VERSION_FILE} "${VERSION}")
endif()