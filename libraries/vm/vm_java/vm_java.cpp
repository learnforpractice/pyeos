#include <stdlib.h>
#include <stdio.h>
#include <eosiolib_native/vm_api.h>
#include <jni.h>

#include "vmapi4java.h"

static JavaVM *vm = nullptr;
static JNIEnv *env = nullptr;

void vm_init(struct vm_api* api) {
   printf("vm_java: init\n");
   vm_register_api(api);

   JavaVMInitArgs vm_args;
   vm_args.version = JNI_VERSION_1_2;

   JavaVMOption options[1];
//   options[0].optionString    = "-Djava.class.path=../vmapi4java.jar";
   options[0].optionString    = "-Djava.class.path=../vmapi4java.jar";
   vm_args.options            = options;
   vm_args.nOptions           = 1;
   vm_args.ignoreUnrecognized = JNI_TRUE;

   // Construct a VM
   jint res = JNI_CreateJavaVM(&vm, (void **)&env, &vm_args);
}

void vm_deinit() {
   printf("vm_java: deinit\n");
   // Shutdown the VM.
   vm->DestroyJavaVM();
}

void invoke_class(JNIEnv* env)
{
    jclass hello_world_class;
    jmethodID main_method;
    jmethodID square_method;
    jmethodID power_method;
    jint number=20;
    jint exponent=3;

    hello_world_class = env->FindClass("vmapi4java");
    main_method = env->GetStaticMethodID(hello_world_class, "main", "([Ljava/lang/String;)V");
    square_method = env->GetStaticMethodID(hello_world_class, "square", "(I)I");
    power_method = env->GetStaticMethodID(hello_world_class, "power", "(II)I");
    env->CallStaticVoidMethod(hello_world_class, main_method, NULL);

    printf("%d squared is %d\n", number,
        env->CallStaticIntMethod(hello_world_class, square_method, number));

    printf("%d raised to the %d power is %d\n", number, exponent,
        env->CallStaticIntMethod(hello_world_class, power_method, number, exponent));
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
