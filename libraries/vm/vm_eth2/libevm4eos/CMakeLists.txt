add_library(eosvm STATIC 
            EosVM.cpp
            EosVMCalls.cpp
            EosVMOpt.cpp
            )

set(CPP_ETHEREUM_DIR "${CMAKE_SOURCE_DIR}/libraries/ethereum")

target_link_libraries(eosvm PRIVATE eosiolib_native fc evm ethashseal)

target_include_directories(eosvm PRIVATE ${Boost_INCLUDE_DIR}
    PRIVATE ${CMAKE_SOURCE_DIR}/contracts
    PRIVATE ${CPP_ETHEREUM_DIR}
    PRIVATE ${LevelDB_INCLUDE}
)
