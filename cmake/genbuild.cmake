# Cross-platform build.h generator (replacement for share/genbuild.sh)
# Usage: cmake -P genbuild.cmake <output_file> [source_dir]
set(OUTPUT_FILE "${CMAKE_ARGV3}")
set(SOURCE_DIR "${CMAKE_ARGV4}")
if(NOT SOURCE_DIR)
    set(SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/..")
endif()

set(BUILD_DESC "// No build information available")
set(BUILD_DATE "")

find_program(GIT_EXECUTABLE git)
if(GIT_EXECUTABLE)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --dirty
        WORKING_DIRECTORY "${SOURCE_DIR}"
        OUTPUT_VARIABLE GIT_DESC
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    execute_process(
        COMMAND ${GIT_EXECUTABLE} log -n 1 --format=%ci
        WORKING_DIRECTORY "${SOURCE_DIR}"
        OUTPUT_VARIABLE GIT_TIME
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    if(GIT_DESC)
        set(BUILD_DESC "#define BUILD_DESC \"${GIT_DESC}\"")
    endif()
    if(GIT_TIME)
        set(BUILD_DATE "#define BUILD_DATE \"${GIT_TIME}\"")
    else()
        set(BUILD_DATE "#define BUILD_DATE \"\"")
    endif()
else()
    set(BUILD_DATE "#define BUILD_DATE \"\"")
endif()

file(WRITE "${OUTPUT_FILE}" "${BUILD_DESC}\n${BUILD_DATE}\n")
