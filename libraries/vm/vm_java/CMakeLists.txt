#set(JAVA_HOME "/Library/Java/JavaVirtualMachines/jdk-10.0.2.jdk")

#find_package(JNI)
#find_package(Java)

if (JNI_FOUND)
    message (STATUS "++++++++++++++JAVA_AWT_LIBRARY=${JAVA_AWT_LIBRARY}")
    message (STATUS "++++++++++++++JAVA_JVM_LIBRARY=${JNI_LIBRARIES}")
endif()

if (Java_FOUND)
    message (STATUS "++++++++++++++JAVA_INCLUDE_PATH=${JAVA_INCLUDE_PATH}")
endif()


get_filename_component(RT_JAR_PATH ${Java_JAVA_EXECUTABLE} DIRECTORY)

set(RT_JAR_PATH "${RT_JAR_PATH}/../jre/lib/rt.jar")
set(JAVA_LIB "$ENV{JAVA_HOME}/lib/server/libjvm.dylib")

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/VMMain.h
    COMMAND javac -h ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/VMMain.java
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/VMMain.java
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/VMMain.jar
    COMMAND jar cvfm ${CMAKE_CURRENT_BINARY_DIR}/VMMain.jar ${CMAKE_CURRENT_SOURCE_DIR}/manifest.txt ${CMAKE_CURRENT_BINARY_DIR}/*.class
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/VMMain.class ${CMAKE_CURRENT_SOURCE_DIR}/manifest.txt
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/VMMain.class
    COMMAND javac -d ${CMAKE_CURRENT_BINARY_DIR} -source 1.6 -target 1.6 -bootclasspath ${RT_JAR_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/VMMain.java
    DEPENDS  ${CMAKE_CURRENT_SOURCE_DIR}/VMMain.java
)

add_library(vm_java SHARED 
            vm_java.cpp
            VMMain.cpp
            inspector.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/VMMain.h
            ${CMAKE_CURRENT_BINARY_DIR}/VMMain.jar
            )

target_link_libraries(vm_java PRIVATE eosiolib_native ${JAVA_LIB})

target_include_directories(vm_java PRIVATE ${Boost_INCLUDE_DIR}
    PRIVATE ${CMAKE_SOURCE_DIR}/externals/magic_get/include
    PRIVATE ${CMAKE_SOURCE_DIR}/contracts
    PRIVATE $ENV{JAVA_HOME}/include
    PRIVATE $ENV{JAVA_HOME}/include/darwin
)
