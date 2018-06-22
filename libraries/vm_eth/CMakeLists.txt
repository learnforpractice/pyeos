add_library(vm_eth SHARED 
#            EosState.cpp
#            EosExecutive.cpp
#            EosDB.cpp
#            EosExtVM.cpp
            vm_eth.cpp)

set(CPP_ETHEREUM_DIR "${CMAKE_SOURCE_DIR}/libraries/ethereum")

include_directories (${CPP_ETHEREUM_DIR})
include_directories (${CPP_ETHEREUM_DIR}/libethereum)
include_directories (${CPP_ETHEREUM_DIR}/evmjit/include)
include_directories (${CPP_ETHEREUM_DIR}/utils)
include_directories (jsoncpp/include)

target_link_libraries(vm_eth PRIVATE eosiolib_native)

target_include_directories(vm_eth PRIVATE ${Boost_INCLUDE_DIR}
#   PRIVATE ${CMAKE_SOURCE_DIR}/contracts/libc++/upstream/include
#   PRIVATE ${CMAKE_SOURCE_DIR}/contracts/libc++/include/libc++/upstream/include
#   PRIVATE ${CMAKE_SOURCE_DIR}/contracts/libc++/musl/include/musl/upstream/include
#    PRIVATE ${CMAKE_SOURCE_DIR}/contracts/musl/upstream/include
    PRIVATE ${CMAKE_SOURCE_DIR}/externals/magic_get/include
    PRIVATE ${CMAKE_SOURCE_DIR}/contracts
    PRIVATE ${CPP_ETHEREUM_DIR}
    PRIVATE ${LevelDB_INCLUDE}
    PRIVATE ${CPP_ETHEREUM_DIR}
)
