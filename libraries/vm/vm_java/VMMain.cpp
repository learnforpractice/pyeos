#include "VMJava.h"
#include <stdio.h>

/*
 * Class:     vmapi4java
 * Method:    sayHello
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_VMJava_sayHello
  (JNIEnv *env, jobject jobj) {
   printf("hello,world\n");
}

