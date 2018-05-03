
add_executable(shared_vector shared_vector.cpp)

target_link_libraries( shared_vector ${Boost_LIBRARIES})

target_compile_options(micropython PRIVATE -g)

target_include_directories( shared_vector PRIVATE ${Boost_INCLUDE_DIRS}
                                  )
