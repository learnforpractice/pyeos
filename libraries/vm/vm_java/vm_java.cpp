#include <stdlib.h>
#include <stdio.h>
#include <eosiolib_native/vm_api.h>
#include <jni.h>

#include "VMMain.h"

static JavaVM* vm = nullptr;
static JNIEnv* env = nullptr;

void invoke_class(JNIEnv* env);

JNIEnv* vm_get_env() {
   return env;
}

void vm_init(struct vm_api* api) {
   printf("vm_java: init\n");
   vm_register_api(api);

   JavaVMInitArgs vm_args;
   vm_args.version = JNI_VERSION_10;
   JNI_GetDefaultJavaVMInitArgs(&vm_args);

   JavaVMOption options[4];
//   options[0].optionString    = "-Djava.class.path=../vmapi4java.jar";
//   options[0].optionString    = "-Djava.class.path=/Library/Java/JavaVirtualMachines/jdk1.8.0_171.jdk/Contents/Home/jre/lib;/Users/newworld/dev/pyeos/build-debug/libraries/vm/vm_java";
//   options[0].optionString    = "-Djava.class.path=/Users/newworld/dev/pyeos/build/libraries/vm/vm_java";
//   options[0].optionString    = "-Djava.class.path=/Users/newworld/eclipse-workspace/javatest";
   options[0].optionString    = "-Djava.class.path=../libs/VMJava.jar";
   options[1].optionString = "-agentpath:../libs/libvm_java" DYLIB_SUFFIX;
   options[2].optionString = "-Xmx256M";
   options[3].optionString = "-Xms256M";

   vm_args.options            = options;
   vm_args.nOptions           = 4;
   vm_args.ignoreUnrecognized = JNI_TRUE;

   // Construct a VM
   jint res = JNI_CreateJavaVM(&vm, (void **)&env, &vm_args);
}

void vm_deinit() {
   printf("vm_java: deinit\n");
   // Shutdown the VM.
//   vm->DestroyJavaVM();
}

static jclass main_class = nullptr;
static jmethodID apply_method = nullptr;
static jmethodID setcode_method = nullptr;

int vm_setcode(uint64_t account) {
   vmdlog("+++++vm_java: setcode\n");
   size_t size = 0;
   const char* code = get_vm_api()->get_code(account, &size);
   if (size <= 0) {
      return 1;
   }

   JNIEnv* env = nullptr;
   //   printf("+++++vm_java: apply\n");// %llu %llu %llu %d\n", receiver, account, act, sizeof(jlong));
   vm->AttachCurrentThread((void**)&env, nullptr);
   if (main_class == nullptr) {
      main_class = env->FindClass("VMJava");
   }
   if (main_class == nullptr) {
      vmdlog("VMJava class not found!");
      return 0;
   }

   if (setcode_method == nullptr) {
      setcode_method = env->GetStaticMethodID(main_class, "setcode", "(J)I");
   }
   if (setcode_method == nullptr) {
      vmdlog("apply method not found in VMJava");
      return 0;
   }

   env->CallStaticIntMethod(main_class, setcode_method, (jlong)account);
//   vm->DetachCurrentThread();
   return 1;
}

int vm_apply(uint64_t receiver, uint64_t account, uint64_t act) {
   JNIEnv* env = nullptr;
//   printf("+++++vm_java: apply %llu %llu %llu %d\n", receiver, account, act, sizeof(jlong));
   vm->AttachCurrentThread((void**)&env, nullptr);
   if (main_class == nullptr) {
      main_class = env->FindClass("VMJava");
   }
   if (main_class == nullptr) {
      vmdlog("VMJava class not found!");
      return 0;
   }

   if (apply_method == nullptr) {
      apply_method = env->GetStaticMethodID(main_class, "apply", "(JJJ)I");
   }
   if (apply_method == nullptr) {
      vmdlog("apply method not found in VMJava");
      return 0;
   }

   jint ret = env->CallStaticIntMethod(main_class, apply_method, (jlong)receiver, (jlong)account, (jlong)act);
//   vmdlog("ret %d \n", ret);

   eosio_assert(ret != 0, "call apply failed!");
   //   vm->DetachCurrentThread();
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
