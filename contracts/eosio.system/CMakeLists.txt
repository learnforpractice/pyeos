file(GLOB ABI_FILES "*.abi")
configure_file("${ABI_FILES}" "${CMAKE_CURRENT_BINARY_DIR}" COPYONLY)

add_wast_executable(TARGET eosio.system
  INCLUDE_FOLDERS ${STANDARD_INCLUDE_FOLDERS}
  LIBRARIES libc++ libc eosiolib eosio.token
  DESTINATION_FOLDER ${CMAKE_CURRENT_BINARY_DIR}
)

add_library(eosiosystem SHARED eosio.system.cpp) #exchange_state.cpp

target_link_libraries(eosiosystem PRIVATE eosiolib_native)

target_include_directories(eosiosystem PRIVATE ${Boost_INCLUDE_DIR}
#   PRIVATE ${CMAKE_SOURCE_DIR}/contracts/libc++/upstream/include
#   PRIVATE ${CMAKE_SOURCE_DIR}/contracts/libc++/include/libc++/upstream/include
#   PRIVATE ${CMAKE_SOURCE_DIR}/contracts/libc++/musl/include/musl/upstream/include
#    PRIVATE ${CMAKE_SOURCE_DIR}/contracts/musl/upstream/include
    PRIVATE ${CMAKE_SOURCE_DIR}/externals/magic_get/include
     PRIVATE ${CMAKE_SOURCE_DIR}/contracts
)

