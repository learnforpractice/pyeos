file(GLOB ABI_FILES "*.abi")
configure_file("${ABI_FILES}" "${CMAKE_CURRENT_BINARY_DIR}" COPYONLY)

add_wast_executable(TARGET eosio.system
  INCLUDE_FOLDERS ${STANDARD_INCLUDE_FOLDERS}
  LIBRARIES libc++ libc eosiolib eosio.token
  DESTINATION_FOLDER ${CMAKE_CURRENT_BINARY_DIR}
)

add_library(eosio_native SHARED eosio.system.cpp) #exchange_state.cpp

target_link_libraries(eosio_native PRIVATE eosiolib_native softfloat)

target_include_directories(eosio_native PRIVATE ${Boost_INCLUDE_DIR}
#   PRIVATE ${CMAKE_SOURCE_DIR}/contracts/libc++/upstream/include
#   PRIVATE ${CMAKE_SOURCE_DIR}/contracts/libc++/include/libc++/upstream/include
#   PRIVATE ${CMAKE_SOURCE_DIR}/contracts/libc++/musl/include/musl/upstream/include
#    PRIVATE ${CMAKE_SOURCE_DIR}/contracts/musl/upstream/include
    PRIVATE ${CMAKE_SOURCE_DIR}/externals/magic_get/include
     PRIVATE ${CMAKE_SOURCE_DIR}/contracts
)

