# Generate version.h
execute_process(
    COMMAND git describe --long --all --dirty
    WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
    OUTPUT_STRIP_TRAILING_WHITESPACE
    OUTPUT_VARIABLE BUILD_VERSION
)
string(TIMESTAMP BUILD_DATE "%Y-%m-%dT%H:%M:%S")

configure_file(${CMAKE_CURRENT_LIST_DIR}/version.h.in version.h)
