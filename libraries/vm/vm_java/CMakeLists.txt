find_package(JNI)

if (JNI_FOUND)
    message (STATUS "++++++++++++++JAVA_AWT_LIBRARY=${JAVA_AWT_LIBRARY}")
    message (STATUS "++++++++++++++JAVA_JVM_LIBRARY=${JAVA_JVM_LIBRARY}")
endif()

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/vmapi4java.h
    COMMAND javac -h ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/vmapi4java.java
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/vmapi4java.java
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/vmapi4java.jar
    COMMAND jar cvfm ${CMAKE_CURRENT_BINARY_DIR}/vmapi4java.jar ${CMAKE_CURRENT_SOURCE_DIR}/manifest.txt ${CMAKE_CURRENT_BINARY_DIR}/*.class
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/vmapi4java.class ${CMAKE_CURRENT_SOURCE_DIR}/manifest.txt
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/vmapi4java.class
    COMMAND javac -d ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/vmapi4java.java
    DEPENDS  ${CMAKE_CURRENT_SOURCE_DIR}/vmapi4java.java
)

add_library(vm_java SHARED 
            vm_java.cpp
            vmapi4java.cpp
            ${CMAKE_CURRENT_BINARY_DIR}/vmapi4java.jar
            )

target_link_libraries(vm_java PRIVATE eosiolib_native ${JNI_LIBRARIES})

target_include_directories(vm_java PRIVATE ${Boost_INCLUDE_DIR}
    PRIVATE ${CMAKE_SOURCE_DIR}/externals/magic_get/include
    PRIVATE ${CMAKE_SOURCE_DIR}/contracts
    PRIVATE ${JNI_INCLUDE_DIRS}
)
