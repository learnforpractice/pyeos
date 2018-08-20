add_library(vm_example SHARED 
            vm_example.cpp)


target_link_libraries(vm_example PRIVATE eosiolib_native)

target_include_directories(vm_example PRIVATE ${Boost_INCLUDE_DIR}
    PRIVATE ${CMAKE_SOURCE_DIR}/externals/magic_get/include
    PRIVATE ${CMAKE_SOURCE_DIR}/contracts
)
