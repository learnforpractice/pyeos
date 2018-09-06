#include "vmapi4java.h"
#include <stdio.h>

/*
 * Class:     vmapi4java
 * Method:    sayHello
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_vmapi4java_sayHello
  (JNIEnv *env, jobject jobj) {
   printf("hello,world\n");
}

