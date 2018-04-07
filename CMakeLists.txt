cmake_minimum_required( VERSION 3.5 )

if (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    # The default toolchain file configures compilers and build environment.
    # This configuration is also used by hunter to build dependencies.
    # CMake will cache this value, not need to explictly specify CACHE param.
    set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/libraries/ethereum/cmake/toolchain.cmake)
endif()

set(ETH_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libraries/ethereum/cmake" CACHE PATH "The path to the cmake directory")
list(APPEND CMAKE_MODULE_PATH ${ETH_CMAKE_DIR})
# Defines EOS library target.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/libraries/fc/CMakeModules")

set(CPP_ETHEREUM_DIR "${CMAKE_SOURCE_DIR}/libraries/ethereum" CACHE PATH "Path to the root directory for cpp-ethereum")

# set cmake_policies
include(EthPolicy)
eth_policy()

if(CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo" CACHE STRING "" FORCE)
endif()

# Map current configuration to configurations of imported targets.
#set(CMAKE_MAP_IMPORTED_CONFIG_DEBUG Release)
#set(CMAKE_MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release)

set(HUNTER_CONFIGURATION_TYPES Release)
set(HUNTER_JOBS_NUMBER 4)
set(HUNTER_CACHE_SERVERS "https://github.com/ethereum/hunter-cache")
set(HUNTER_PASSWORDS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Hunter/passwords.cmake)

# In CI builds upload the binaries if the GITHUB_USER_PASSWORD was decrypted
# (only for branches and internal PRs).
if("$ENV{CI}" AND NOT "$ENV{GITHUB_USER_PASSWORD}" STREQUAL "")
    set(run_upload YES)
else()
    set(run_upload NO)
endif()
option(HUNTER_RUN_UPLOAD "Upload binaries to the cache server" ${run_upload})

include(HunterGate)
HunterGate(
    URL "https://github.com/learnforpractice/hunter/archive/v0.20.27.2.tar.gz"
    SHA1 "a8829718f514ba870f8be876ed508638c2e9cac7"
    LOCAL
)

project( EOS )

enable_testing()


include( GNUInstallDirs )
include( VersionMacros )
include( SetupTargetMacros )
include( InstallDirectoryPermissions )

set( BLOCKCHAIN_NAME "Eos" )
set( CMAKE_CXX_STANDARD 14 )
set( CMAKE_CXX_EXTENSIONS ON )
set( CXX_STANDARD_REQUIRED ON)

set(VERSION_MAJOR 3)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)

set( CLI_CLIENT_EXECUTABLE_NAME cleos )
set( GUI_CLIENT_EXECUTABLE_NAME eosio )
set( CUSTOM_URL_SCHEME "gcs" )
set( INSTALLER_APP_ID "68ad7005-8eee-49c9-95ce-9eed97e5b347" )

# http://stackoverflow.com/a/18369825
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.8)
        message(FATAL_ERROR "GCC version must be at least 4.8!")
    endif()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.3)
        message(FATAL_ERROR "Clang version must be at least 3.3!")
    endif()
endif()


set(CMAKE_EXPORT_COMPILE_COMMANDS "ON")
set(BUILD_DOXYGEN FALSE CACHE BOOL "Build doxygen documentation on every make")
set(BUILD_MONGO_DB_PLUGIN FALSE CACHE BOOL "Build mongo database plugin")
set(EOS_EGENESIS_JSON "${CMAKE_CURRENT_SOURCE_DIR}/genesis.json" )

#set (USE_PCH 1)

if (USE_PCH)
  include (cotire)
endif(USE_PCH)


hunter_add_package(OpenSSL)
find_package(OpenSSL REQUIRED)

if(UNIX)
  if(APPLE)
    set(whole_archive_flag "-force_load")
    set(no_whole_archive_flag "")
  else()
    set(whole_archive_flag "--whole-archive")
    set(no_whole_archive_flag "--no-whole-archive")
  endif()
else()
  set(whole_archive_flag "--whole-archive")
  set(no_whole_archive_flag "--no-whole-archive")
endif()

hunter_add_package(BZip2)
find_package( BZip2 )

SET( Boost_USE_STATIC_LIBS ON CACHE STRING "ON or OFF" )
IF( WIN32 )
  SET(BOOST_ROOT $ENV{BOOST_ROOT})
  set(Boost_USE_MULTITHREADED ON)
  set(BOOST_ALL_DYN_LINK OFF) # force dynamic linking for all libraries
ENDIF(WIN32)

hunter_add_package(Boost COMPONENTS 
    thread
    date_time
    filesystem
    system
    program_options
    signals
    serialization
    chrono
    test
    context
    locale
    iostreams
    fiber)
    
find_package(Boost REQUIRED COMPONENTS
    thread
    date_time
    filesystem
    system
    program_options
    signals
    serialization
    chrono
    unit_test_framework
    context
    locale
    iostreams)


if( WIN32 )

    message( STATUS "Configuring Eos on WIN32")
    set( DB_VERSION 60 )
    set( BDB_STATIC_LIBS 1 )

    set( ZLIB_LIBRARIES "" )
    SET( DEFAULT_EXECUTABLE_INSTALL_DIR bin/ )

    set(CRYPTO_LIB)

    #looks like this flag can have different default on some machines.
    SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /SAFESEH:NO")
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")

    # Probably cmake has a bug and vcxproj generated for executable in Debug conf. has disabled debug info
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /DEBUG")

    # On windows tcl should be installed to the directory pointed by setenv.bat script
    SET(TCL_INCLUDE_PATH $ENV{TCL_ROOT}/include)
    MESSAGE(STATUS "tcl INCLUDE PATH: ${TCL_INCLUDE_PATH}")

    FIND_PACKAGE(TCL)
    MESSAGE(STATUS "tcl_library: ${TCL_LIBRARY}")

    SET(TCL_LIBS "optimized;${TCL_LIBRARY};debug;")
    get_filename_component(TCL_LIB_PATH "${TCL_LIBRARY}" PATH)
    get_filename_component(TCL_LIB_NAME "${TCL_LIBRARY}" NAME_WE)
    get_filename_component(TCL_LIB_EXT "${TCL_LIBRARY}" EXT)

    SET(TCL_LIBS "${TCL_LIBS}${TCL_LIB_PATH}/${TCL_LIB_NAME}g${TCL_LIB_EXT}")
    SET(TCL_LIBRARY ${TCL_LIBS})

else( WIN32 ) # Apple AND Linux

    if( APPLE )
        # Apple Specific Options Here
        message( STATUS "Configuring Eos on OS X" )
        set( CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -Wall -Wno-deprecated-declarations" )
    else( APPLE )
        # Linux Specific Options Here
        message( STATUS "Configuring Eos on Linux" )
        set( CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -Wall" )
        if ( FULL_STATIC_BUILD )
          set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++ -static-libgcc")
        endif ( FULL_STATIC_BUILD )

        if( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" )
            if( CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 4.0.0 OR CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.0.0 )
                set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-invalid-partial-specialization" )
            endif()
        endif()
    endif( APPLE )

    if( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" )
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-builtin-memcmp" )
    endif()

    if( "${CMAKE_GENERATOR}" STREQUAL "Ninja" )
        if( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" )
            set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcolor-diagnostics" )
        endif()
    endif()

    # based on http://www.delorie.com/gnu/docs/gdb/gdb_70.html
    # uncomment this line to tell GDB about macros (slows compile times)
    set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -gdwarf-2 -g3" )

endif( WIN32 )

set(ENABLE_COVERAGE_TESTING FALSE CACHE BOOL "Build Eos for code coverage analysis")

if(ENABLE_COVERAGE_TESTING)
  SET(CMAKE_CXX_FLAGS "--coverage ${CMAKE_CXX_FLAGS}")
  find_program( LCOV_PATH  lcov )
  find_program( GENHTML_PATH NAMES genhtml)
endif()

add_subdirectory( externals )

include(wasm)

add_subdirectory( libraries )
add_subdirectory( contracts )
add_subdirectory( plugins )
add_subdirectory( programs )
add_subdirectory( scripts )
add_subdirectory( tests )
add_subdirectory( tools )
add_subdirectory( debian )

install_directory_permissions(DIRECTORY ${CMAKE_INSTALL_FULL_SYSCONFDIR}/eosio)

install(FILES testnet.template DESTINATION ${CMAKE_INSTALL_FULL_SYSCONFDIR}/eosio/launcher)
install_directory_permissions(DIRECTORY ${CMAKE_INSTALL_FULL_SYSCONFDIR}/eosio)
install_directory_permissions(DIRECTORY ${CMAKE_INSTALL_FULL_SYSCONFDIR}/eosio/launcher)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/testnet.template ${CMAKE_CURRENT_BINARY_DIR}/etc/eosio/launcher/testnet.template COPYONLY)

include(installer)

include(doxygen)
