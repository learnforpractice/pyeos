#include "inspector.h"
#include <stdio.h>

extern "C" JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM *jvm, char *options, void *reserved) {
   /* We return JNI_OK to signify success */
   printf("++++++++++hello, jvmti!\n");
   return JNI_OK;
}

