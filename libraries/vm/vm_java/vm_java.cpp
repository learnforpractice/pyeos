#include <stdlib.h>
#include <stdio.h>
#include <eosiolib_native/vm_api.h>
#include <jni.h>

#include "VMJava.h"

static JavaVM *vm = nullptr;
static JNIEnv *env = nullptr;

void invoke_class(JNIEnv* env);

void vm_init(struct vm_api* api) {
   printf("vm_java: init\n");
   vm_register_api(api);

   JavaVMInitArgs vm_args;
   vm_args.version = JNI_VERSION_1_2;
   JNI_GetDefaultJavaVMInitArgs(&vm_args);

   JavaVMOption options[1];
//   options[0].optionString    = "-Djava.class.path=../vmapi4java.jar";
//   options[0].optionString    = "-Djava.class.path=/Library/Java/JavaVirtualMachines/jdk1.8.0_171.jdk/Contents/Home/jre/lib;/Users/newworld/dev/pyeos/build-debug/libraries/vm/vm_java";
//   options[0].optionString    = "-Djava.class.path=/Users/newworld/dev/pyeos/build-debug/libraries/vm/vm_java";
   options[0].optionString    = "-Djava.class.path=/Users/newworld/eclipse-workspace/javatest";

   vm_args.options            = options;
   vm_args.nOptions           = 1;
   vm_args.ignoreUnrecognized = JNI_TRUE;

   // Construct a VM
   jint res = JNI_CreateJavaVM(&vm, (void **)&env, &vm_args);
   invoke_class(env);
}

void vm_deinit() {
   printf("vm_java: deinit\n");
   // Shutdown the VM.
   vm->DestroyJavaVM();
}

void invoke_class(JNIEnv* env)
{
    jclass hello_world_class = nullptr;
    jmethodID main_method = nullptr;
    jmethodID square_method = nullptr;
    jmethodID power_method = nullptr;
    jint number=20;
    jint exponent=3;

    hello_world_class = env->FindClass("java/lang/String");
//    hello_world_class = env->FindClass("javatest/Main");
    hello_world_class = env->FindClass("VMMain");
    main_method = env->GetStaticMethodID(hello_world_class, "main", "([Ljava/lang/String;)V");
    square_method = env->GetStaticMethodID(hello_world_class, "square", "(I)I");
    power_method = env->GetStaticMethodID(hello_world_class, "power", "(II)I");
    env->CallStaticVoidMethod(hello_world_class, main_method, NULL);

    char *days[]={"Sunday",
                    "Monday",
                    "Tuesday",
                    "Wednesday",
                    "Thursday",
                    "Friday",
                    "Saturday"};

   jstring str;
   jobjectArray day = 0;
   jsize len = 7;
   int i;

   day = env->NewObjectArray(len,env->FindClass("java/lang/String"),0);

   for(i=0;i<7;i++)
   {
      str = env->NewStringUTF(days[i]);
      env->SetObjectArrayElement(day,i,str);
   }

   env->CallStaticIntMethod(hello_world_class, main_method, day);

#if 0
    printf("%d squared is %d\n", number,
        env->CallStaticIntMethod(hello_world_class, square_method, number));

    printf("%d raised to the %d power is %d\n", number, exponent,
        env->CallStaticIntMethod(hello_world_class, power_method, number, exponent));
#endif
}

int vm_setcode(uint64_t account) {
   printf("+++++vm_java: setcode\n");
   invoke_class(env);
   return 0;
}

int vm_apply(uint64_t receiver, uint64_t account, uint64_t act) {
   printf("+++++vm_java: apply\n");
   return 1;
}

int vm_call(uint64_t account, uint64_t func) {
   printf("+++++vm_java: call\n");
   return 0;
}
