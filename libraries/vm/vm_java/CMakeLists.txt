#set(JAVA_HOME "/Library/Java/JavaVirtualMachines/jdk-10.0.2.jdk")

find_package(JNI)
find_package(Java)

if (JNI_FOUND)
    message (STATUS "++++++++++++++JAVA_AWT_LIBRARY=${JAVA_AWT_LIBRARY}")
    message (STATUS "++++++++++++++JAVA_JVM_LIBRARY=${JNI_LIBRARIES}")
endif()

if (Java_FOUND)
    message (STATUS "++++++++++++++JAVA_INCLUDE_PATH=${JAVA_INCLUDE_PATH}")
endif()


get_filename_component(RT_JAR_PATH ${Java_JAVA_EXECUTABLE} DIRECTORY)

set(RT_JAR_PATH "${RT_JAR_PATH}/../jre/lib/rt.jar")
if (APPLE)
set(JAVA_LIB "$ENV{JAVA_HOME}/lib/server/libjvm.dylib")
else()
set(JAVA_LIB "$ENV{JAVA_HOME}/lib/server/libjvm.so")
endif()

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
set(DEBUG_FLAG "d")
endif()


add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/NativeInterface.h
    COMMAND javac -h ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/VMJava.java
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/VMJava.java
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/VMJava.jar
    COMMAND jar cf ${CMAKE_CURRENT_BINARY_DIR}/VMJava.jar -C ${CMAKE_CURRENT_BINARY_DIR} *.class
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/VMJava.class
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/VMJava.class
#-bootclasspath ${RT_JAR_PATH}
#--class-path /Users/newworld/dev/pyeos/build-debug/libraries/vm/vm_java
    COMMAND javac -d ${CMAKE_CURRENT_BINARY_DIR} -source 1.6 -target 1.6 ${CMAKE_CURRENT_SOURCE_DIR}/VMJava.java
    DEPENDS  ${CMAKE_CURRENT_SOURCE_DIR}/VMJava.java ${CMAKE_CURRENT_BINARY_DIR}/Contract.class
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Contract.class
    COMMAND javac -d ${CMAKE_CURRENT_BINARY_DIR} -source 1.6 -target 1.6 ${CMAKE_CURRENT_SOURCE_DIR}/Contract.java
    DEPENDS  ${CMAKE_CURRENT_SOURCE_DIR}/Contract.java
)

add_library(vm_java SHARED 
            vm_java.cpp
            NativeInterface.cpp
            inspector.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/NativeInterface.h
            ${CMAKE_CURRENT_BINARY_DIR}/VMJava.jar
            ${CMAKE_CURRENT_BINARY_DIR}/Contract.class
            )

target_link_libraries(vm_java PRIVATE eosiolib_native ${JAVA_LIB})

target_compile_options(vm_java PRIVATE -DDYLIB_SUFFIX="${DEBUG_FLAG}${CMAKE_SHARED_LIBRARY_SUFFIX}")

target_include_directories(vm_java PRIVATE ${Boost_INCLUDE_DIR}
    PRIVATE ${CMAKE_SOURCE_DIR}/externals/magic_get/include
    PRIVATE ${CMAKE_SOURCE_DIR}/contracts
    PRIVATE $ENV{JAVA_HOME}/include
    PRIVATE $ENV{JAVA_HOME}/include/darwin
    PRIVATE $ENV{JAVA_HOME}/include/linux
)
