add_library(eosiolib_native SHARED 
eosiolib.cpp
action.cpp
chain.cpp
crypto.cpp
db.cpp
memory.cpp
permission.cpp
print.cpp
privileged.cpp
system.cpp
transaction.cpp

math/fabs.cpp
math/pow.cpp
math/sqrt.cpp
math/scalbn.cpp

double.hpp
) #exchange_state.cpp

target_link_libraries( eosiolib_native PUBLIC softfloat)

target_include_directories(eosiolib_native PRIVATE ${Boost_INCLUDE_DIR}
#   PRIVATE ${CMAKE_SOURCE_DIR}/contracts/libc++/upstream/include
#   PRIVATE ${CMAKE_SOURCE_DIR}/contracts/libc++/include/libc++/upstream/include
#   PRIVATE ${CMAKE_SOURCE_DIR}/contracts/libc++/musl/include/musl/upstream/include
#    PRIVATE ${CMAKE_SOURCE_DIR}/contracts/musl/upstream/include
    PRIVATE ${CMAKE_SOURCE_DIR}/externals/magic_get/include
    PRIVATE ${CMAKE_SOURCE_DIR}/contracts
    PRIVATE ${CMAKE_SOURCE_DIR}/contracts/eosiolib
    PUBLIC ${CMAKE_SOURCE_DIR}/libraries/chain/include
    PUBLIC ${CMAKE_SOURCE_DIR}/libraries/fc/include
    PUBLIC ${CMAKE_SOURCE_DIR}/libraries/chainbase/include

)

add_executable ( st test/main.cpp )
target_link_libraries( st PUBLIC softfloat)

target_include_directories( st PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/source/include" "${CMAKE_CURRENT_SOURCE_DIR}/source/8086-SSE" "${CMAKE_CURRENT_SOURCE_DIR}/build/Linux-x86_64-GCC" )
