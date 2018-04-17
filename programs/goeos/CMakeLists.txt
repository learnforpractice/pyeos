cmake_minimum_required(VERSION 3.2)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/)

project(hello_world Go)

set(INFILE ${PROJECT_SOURCE_DIR}/src/bridge/bridge.go.in)
set(OUTFILE ${PROJECT_BINARY_DIR}/src/bridge/bridge.go)
set(C_LIBRARY_PATH ${PROJECT_BINARY_DIR}/lib)

set(CFLAGS "-I${C_INCLUDE_PATH}")
set(LDFLAGS "-L${C_LIBRARY_PATH} -l${C_LIBRARY_NAME}")

message(STATUS "Creating .go bridge file")
message(STATUS "Input template:${INFILE}" )
message(STATUS "Output file   :${OUTFILE}")
message(STATUS "Output file CFLAGS : ${CFLAGS}")
message(STATUS "Output file LDFLAGS: ${LDFLAGS}")

configure_file( ${INFILE} ${OUTFILE} )


set(SOURCE_FILES src/main.go)

add_executable(hello_world ${SOURCE_FILES})

target_link_libraries( hello_world appbase )

