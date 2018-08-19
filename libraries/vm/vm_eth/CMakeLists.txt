add_library(vm_eth SHARED 
#            EosDB.cpp
            Executive.cpp
            State.cpp
            Precompiled.cpp
            SealEngine.cpp
            ExtVM.cpp
            vm_eth.cpp)

set(CPP_ETHEREUM_DIR "${CMAKE_SOURCE_DIR}/libraries/ethereum")

include_directories (${CPP_ETHEREUM_DIR})
include_directories (${CPP_ETHEREUM_DIR}/libethereum)
include_directories (${CPP_ETHEREUM_DIR}/evmjit/include)
include_directories (${CPP_ETHEREUM_DIR}/utils)
include_directories (jsoncpp/include)

target_link_libraries(vm_eth PRIVATE eosiolib_native evm4eos ${Boost_LIBRARIES})

target_include_directories(vm_eth PRIVATE ${Boost_INCLUDE_DIR}
    PRIVATE ${CMAKE_SOURCE_DIR}/externals/magic_get/include
    PRIVATE ${CMAKE_SOURCE_DIR}/contracts
    PRIVATE ${CMAKE_SOURCE_DIR}/libraries/vm/libevm4eos/include
    PRIVATE ${CMAKE_SOURCE_DIR}/libraries/vm/libevm4eos
    PRIVATE ${CMAKE_SOURCE_DIR}/contracts
    PRIVATE ${CPP_ETHEREUM_DIR}
    PRIVATE ${LevelDB_INCLUDE}
    PRIVATE ${CPP_ETHEREUM_DIR}
)
