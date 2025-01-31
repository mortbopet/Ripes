# -----------------------------------------------------------------------------
# Contributors file
# A file gen_contributors.html is generated at configure time, containing the
# list of contributors to the project; this file is included through text.qrc.

add_custom_target(
    contributors_file ALL
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/text.qrc
)

if(NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(PYTHON_COMMAND python3)
else()
    set(PYTHON_COMMAND python.exe)
endif()

execute_process(
    COMMAND ${PYTHON_COMMAND} ${CMAKE_CURRENT_SOURCE_DIR}/get_contributors.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
    #COMMAND_ERROR_IS_FATAL ANY
)
