IF(NOT APPLE)
 # Linux or other unix
 SET(rt_library rt )
 SET(pthread_library pthread)
ENDIF(NOT APPLE)

add_executable( appbase_example main.cpp )
target_link_libraries( appbase_example appbase ${CMAKE_DL_LIBS} ${PLATFORM_SPECIFIC_LIBS} ${pthread_library})
