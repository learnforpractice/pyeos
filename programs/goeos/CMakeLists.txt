if( UNIX AND NOT APPLE )
  set(rt_library rt )
endif()

find_package( Gperftools QUIET )
if( GPERFTOOLS_FOUND )
    message( STATUS "Found gperftools; compiling steemd with TCMalloc")
    list( APPEND PLATFORM_SPECIFIC_LIBS tcmalloc )
endif()



add_library(goeos SHARED goeos.c)

if( APPLE )
target_link_libraries( goeos pyeos-shared intl )
else( APPLE )
target_link_libraries( goeos pyeos-shared util -Xlinker -export-dynamic)
endif( APPLE )

target_include_directories( goeos PUBLIC ${CMAKE_SOURCE_DIR}/include 
                                  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
                                  PUBLIC ${CMAKE_SOURCE_DIR}/libraries/python/Include
                                  PUBLIC ${CMAKE_SOURCE_DIR}/libraries/python
                                  PUBLIC ${CMAKE_SOURCE_DIR}/libraries/chain
                                  PUBLIC  ${CMAKE_SOURCE_DIR}/plugins/wallet_plugin/include
                                  PUBLIC  ${CMAKE_SOURCE_DIR}/plugins/wallet_plugin/include
                                  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/interface"
                                  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/util"
                                  PRIVATE "${CPP_ETHEREUM_DIR}"
                                  )



####
#### C Library section
####

# Make sure C library files (.a/.lib files) are built under 'lib' subdir
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_BINARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)


####
#### Go language section
####

# Create bridge.go bridge file using 'go.in' template

set(INFILE ${CMAKE_CURRENT_SOURCE_DIR}/src/bridge/bridge.go.in)
set(OUTFILE ${CMAKE_CURRENT_SOURCE_DIR}/src/bridge/bridge.go)

set(CFLAGS "-I${CMAKE_CURRENT_SOURCE_DIR}/include")

if (APPLE)
    set(LDFLAGS "-L${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/libgoeos.dylib")
else (APPLE)
    set(LDFLAGS "-L${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/libgoeos.so")
endif(APPLE)

string(REPLACE ";" " " LDFLAGS "${LDFLAGS}")


message(STATUS "Creating .go bridge file")
message(STATUS "Input template:${INFILE}" )
message(STATUS "Output file   :${OUTFILE}")
message(STATUS "Output file CFLAGS : ${CFLAGS}")
message(STATUS "Output file LDFLAGS: ${LDFLAGS}")

configure_file( ${INFILE} ${OUTFILE} )
configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/src/goeos/server.go.in ${CMAKE_CURRENT_SOURCE_DIR}/src/goeos/server.go )


# Call 'go build' to generate executable file
# GOPATH / GOBIN
# see https://golang.org/doc/articles/go_command.html
# note: order matters... binary must come first so bridge.go can be found.
set(GOPATH ${CMAKE_CURRENT_SOURCE_DIR}:${CMAKE_CURRENT_BINARY_DIR})
set(GOBIN "${CMAKE_CURRENT_BINARY_DIR}/bin")

message(STATUS "GOPATH=${GOPATH}")
message(STATUS "GOBIN=${GOBIN}")
message(STATUS "executable file: ${GOBIN}/${CMAKE_PROJECT_NAME}")

# create shell programs to help user set GOPATH and GOBIN

set(SHFILE ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/setenv.sh)
file(WRITE  ${SHFILE} "")
file(APPEND ${SHFILE} "GOPATH=${GOPATH}\n")
file(APPEND ${SHFILE} "GOBIN=${GOBIN}\n")
file(APPEND ${SHFILE} "export GOPATH\n")
file(APPEND ${SHFILE} "export GOBIN\n")
file(APPEND ${SHFILE} "echo GOPATH=$GOPATH\n")
file(APPEND ${SHFILE} "echo GOBIN=$GOBIN\n")

set(CSHFILE ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/setenv.csh)
file(WRITE  ${CSHFILE} "")
file(APPEND ${CSHFILE} "setenv GOPATH ${GOPATH}\n")
file(APPEND ${CSHFILE} "setenv GOBIN ${GOBIN}\n")
file(APPEND ${CSHFILE} "echo GOPATH=$GOPATH\n")
file(APPEND ${CSHFILE} "echo GOBIN=$GOBIN\n")


# Setup 'go install' for Makefile.

# To dump alot more debug info with go build '-x', set the
# environment variable VERBOSE=1 before running cmake
# (for example, VERBOSE=1 cmake)

if(DEFINED ENV{VERBOSE})
    set(GOFLAGS "-v" "-x")
else()
    set(GOFLAGS "")
endif()

#set(CUSTCMD strace -f go install -v hello)
#set(CUSTCMD go build hello)

set(GOCMD go build ${GOFLAGS} goeos)
message(STATUS "gocmd ${GOCMD}")

set(GOFULLCMD GOBIN=${GOBIN} GOPATH=${GOPATH} ${GOCMD})

# the args to add_custom_target have ';' but i want to print with spaces ' '
# so pretty print here
string(REPLACE ";" " " GOCMDPP "${GOFULLCMD}")

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/none
    COMMAND rm ${CMAKE_CURRENT_BINARY_DIR}/goeos
    COMMAND touch ${CMAKE_CURRENT_BINARY_DIR}/none
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/cython/eostypes.pyx ${CMAKE_CURRENT_SOURCE_DIR}/cython/eostypes_.pxd
)
set(GOSRC src/goeos/main.go
    src/goeos/server.go 
    src/goeos/client.go 
    src/bridge/bridge.go.in
)

add_custom_target( goinstall ALL
  ${GOFULLCMD}
  COMMENT "Running ${GOCMDPP}"
  DEPENDS goeos ${OUTFILE} ${GOSRC} ${CMAKE_CURRENT_SOURCE_DIR}/src/goeos/server.go
)

