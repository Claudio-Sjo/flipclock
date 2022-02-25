# Libraries
set(CMAKE_LIBRARY_PATH "${CMAKE_CURRENT_SOURCE_DIR}/libs")

find_library( DP_LIB arm_cortexM7lfdp_math )
find_library( SP_LIB arm_cortexM7lfsp_math )
find_library( RT_LIB clang_rt.builtins-armv7em )
find_library( C_LIB c )
find_library( M_LIB m )
#find_library( RT_LIB "clang_rt.builtins-armv7em.lib" PATHS "${CMAKE_CURRENT_SOURCE_DIR}/libs")
target_link_libraries(${PROJECT_NAME} ${DP_LIB} ${SP_LIB} ${C_LIB} ${M_LIB} ${RT_LIB})

# Compiler and linker flags
# Language standard
target_compile_options(${PROJECT_NAME} PRIVATE -std=gnu11)

# Options for Debug builds
set(CMAKE_C_FLAGS_DEBUG "-Og -g3" CACHE STRING "" FORCE)
# Options for Release builds
set(CMAKE_C_FLAGS_RELEASE "-Ofast" CACHE STRING "" FORCE) # -flto")

# Project related linker options
target_link_options(
    ${PROJECT_NAME} PRIVATE
    -target ${ARCH}
    LINKER:-Map=${PROJECT_NAME}.map
    "-L${CMAKE_CURRENT_LIST_DIR}"
    "-L${CMAKE_CURRENT_SOURCE_DIR}/libs"
    "-TRT1020.ld"
    # "$<$<CONFIG:Release>:-flto>"
    # "$<$<CONFIG:Release>:-Wl,--undefined=vTaskSwitchContext>"
)
