add_subdirectory( libevm4eos )

add_library(vm_eth2 SHARED 
            EosState.cpp
            EosExecutive.cpp
            EosExtVM.cpp
            VM_API.cpp
            vm_eth.cpp)

set(CPP_ETHEREUM_DIR "${CMAKE_SOURCE_DIR}/libraries/ethereum")

include_directories (${CPP_ETHEREUM_DIR})
include_directories (${CPP_ETHEREUM_DIR}/libethereum)
include_directories (${CPP_ETHEREUM_DIR}/evmjit/include)
include_directories (${CPP_ETHEREUM_DIR}/utils)
include_directories (jsoncpp/include)

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
