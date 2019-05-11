cmake_minimum_required(VERSION 3.0)
project(aggregation-server C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "-Wall -Wextra")
# Uncomment to enable diagnostic logging
# add_definitions(-DDEBUG)

find_package(Threads)

add_executable(
        aggregation-server
        src/main.c
        src/protocol.h src/protocol.c
        src/input.h src/input.c
        src/hashtable.c src/hashtable.h
        src/error.h
)

target_link_libraries(aggregation-server ${CMAKE_THREAD_LIBS_INIT})

find_package(Doxygen)
if (DOXYGEN_FOUND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)

    add_custom_target(doc
            ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            )
endif (DOXYGEN_FOUND)
