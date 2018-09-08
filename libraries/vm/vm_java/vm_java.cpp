#include <stdlib.h>
#include <stdio.h>
#include <eosiolib_native/vm_api.h>
#include <jni.h>

#include "VMMain.h"

static JavaVM *vm = nullptr;
static JNIEnv *env = nullptr;

void invoke_class(JNIEnv* env);

void vm_init(struct vm_api* api) {
   printf("vm_java: init\n");
   vm_register_api(api);

   JavaVMInitArgs vm_args;
   vm_args.version = JNI_VERSION_10;
   JNI_GetDefaultJavaVMInitArgs(&vm_args);

   JavaVMOption options[2];
//   options[0].optionString    = "-Djava.class.path=../vmapi4java.jar";
//   options[0].optionString    = "-Djava.class.path=/Library/Java/JavaVirtualMachines/jdk1.8.0_171.jdk/Contents/Home/jre/lib;/Users/newworld/dev/pyeos/build-debug/libraries/vm/vm_java";
   options[0].optionString    = "-Djava.class.path=/Users/newworld/dev/pyeos/build-debug/libraries/vm/vm_java";
//   options[0].optionString    = "-Djava.class.path=/Users/newworld/eclipse-workspace/javatest";
   options[1].optionString = "-agentpath:../libs/libvm_javad.dylib";

   vm_args.options            = options;
   vm_args.nOptions           = 2;
   vm_args.ignoreUnrecognized = JNI_TRUE;

   // Construct a VM
   jint res = JNI_CreateJavaVM(&vm, (void **)&env, &vm_args);
   invoke_class(env);
}

void vm_deinit() {
   printf("vm_java: deinit\n");
   // Shutdown the VM.
//   vm->DestroyJavaVM();
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

    const char *days[]={"Sunday",
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

#if 0
jint JNICALL my_jni_create_java_vm(JavaVM **pvm, void **penv, void *args)
{
    JavaVMInitArgs *vm_args = reinterpret_cast(args);
    std::vector vm_options;
    for(jint i = 0; i nOptions; ++i)
    {
        vm_options.push_back(vm_args->options[i]);
    }

    JavaVMOption jvmti_agent_option_agent_path;
    JavaVMOption jvmti_agent_option_abort;
    JavaVMOption jvmti_agent_option_exit;
    JavaVMOption jvmti_agent_option_vfprintf;

    std::string option_string("-agentpath:" + get_path_to_dll(dll_module_handle));
    jvmti_agent_option_agent_path.optionString = const_cast(option_string.c_str());
    vm_options.push_back(jvmti_agent_option_agent_path);

    jvmti_agent_option_abort.optionString = "abort";
    jvmti_agent_option_exit.optionString = "exit";
    jvmti_agent_option_vfprintf.optionString = "vfprintf";

    jvmti_agent_option_abort.extraInfo = reinterpret_cast(my_jvm_abort);
    jvmti_agent_option_exit.extraInfo = reinterpret_cast(my_jvm_exit);
    jvmti_agent_option_vfprintf.extraInfo = reinterpret_cast(my_jvm_vfprintf);

    vm_options.push_back(jvmti_agent_option_abort);
    vm_options.push_back(jvmti_agent_option_exit);
    vm_options.push_back(jvmti_agent_option_vfprintf);

    vm_args->nOptions += 4;
    vm_args->options = &vm_options[0];


    return jni_create_java_vm_hook.OriginalFunc(3, pvm, penv, vm_args);

}
#endif
