add_subdirectory( libevm4eos )

add_library(vm_eth2 SHARED 
            EosState.cpp
            EosExecutive.cpp
            EosExtVM.cpp
            VM_API.cpp
            vm_eth.cpp)

set(CPP_ETHEREUM_DIR "${CMAKE_SOURCE_DIR}/externals/ethereum")

include_directories (${CPP_ETHEREUM_DIR})
include_directories (${CPP_ETHEREUM_DIR}/libethereum)
include_directories (${CPP_ETHEREUM_DIR}/evmjit/include)
include_directories (${CPP_ETHEREUM_DIR}/utils)
include_directories (jsoncpp/include)

target_compile_options(vm_eth2 PRIVATE -DVM_EVM)

target_link_libraries(vm_eth2 PRIVATE eosiolib_native fc eosvm ethashseal)

target_include_directories(vm_eth2 PRIVATE ${Boost_INCLUDE_DIR}
#   PRIVATE ${CMAKE_SOURCE_DIR}/contracts/libc++/upstream/include
#   PRIVATE ${CMAKE_SOURCE_DIR}/contracts/libc++/include/libc++/upstream/include
#   PRIVATE ${CMAKE_SOURCE_DIR}/contracts/libc++/musl/include/musl/upstream/include
#    PRIVATE ${CMAKE_SOURCE_DIR}/contracts/musl/upstream/include
    PRIVATE ${CMAKE_SOURCE_DIR}/externals/magic_get/include
    PRIVATE ${CMAKE_SOURCE_DIR}/contracts
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${CPP_ETHEREUM_DIR}
    PRIVATE ${LevelDB_INCLUDE}
)


add_library(vm_hera SHARED 
            EosState.cpp
            EosExecutive.cpp
            EosExtVM.cpp
            VM_API.cpp
            vm_eth.cpp)

include_directories (${CPP_ETHEREUM_DIR}/hera/include)

if (APPLE)
    set(_symbols_list "${CMAKE_CURRENT_SOURCE_DIR}/symbols.list")
    set(LINK_FLAGS "${LINK_FLAGS} -Wl,-exported_symbols_list,'${_symbols_list}'")
else()
    set(_version_script "${CMAKE_CURRENT_SOURCE_DIR}/version.script")
    set(LINK_FLAGS "${LINK_FLAGS} -Wl,--version-script,\"${_version_script}\"")
endif (APPLE)

target_link_libraries(vm_hera PRIVATE eosiolib_native hera fc eosvm ethashseal)

target_compile_options(vm_hera PRIVATE -DVM_HERA)

set_target_properties(vm_hera  PROPERTIES LINK_FLAGS "${LINK_FLAGS}")


target_include_directories(vm_hera PRIVATE ${Boost_INCLUDE_DIR}
    PRIVATE ${CMAKE_SOURCE_DIR}/externals/magic_get/include
    PRIVATE ${CMAKE_SOURCE_DIR}/contracts
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${CPP_ETHEREUM_DIR}
    PRIVATE ${LevelDB_INCLUDE}
)

if (EVMJIT)
add_library(vm_evmjit SHARED 
            EosState.cpp
            EosExecutive.cpp
            EosExtVM.cpp
            VM_API.cpp
            vm_eth.cpp)

include_directories (${CPP_ETHEREUM_DIR}/evmjit/include)


target_link_libraries(vm_evmjit PRIVATE eosiolib_native hera fc eosvm ethashseal 
${CMAKE_SOURCE_DIR}/externals/ethereum/evmjit/build/libevmjit/libevmjit.a)

target_compile_options(vm_evmjit PRIVATE -DVM_EVMJIT)

set_target_properties(vm_evmjit  PROPERTIES LINK_FLAGS "${LINK_FLAGS}")

target_include_directories(vm_evmjit PRIVATE ${Boost_INCLUDE_DIR}
    PRIVATE ${CMAKE_SOURCE_DIR}/externals/magic_get/include
    PRIVATE ${CMAKE_SOURCE_DIR}/contracts
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${CPP_ETHEREUM_DIR}
    PRIVATE ${LevelDB_INCLUDE}
)
endif()