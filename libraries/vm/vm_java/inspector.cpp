#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <jvmti.h>
#include <jni.h>

#include "inspector.h"

static jniNativeInterface* originalJniFunctions = nullptr;
static jniNativeInterface* replacedJniFunctions = nullptr;
static jvmtiEnv* jvmti = nullptr;

JNIEnv* vm_get_env();

// caller should free the returned jthread
jthread* getCurrentThread()
{
    jthread* currentThread = (jthread*)malloc(sizeof(jthread));
    jvmtiError errorCode = jvmti->GetCurrentThread(currentThread);
    if (errorCode != JVMTI_ERROR_NONE) {
        logError("Error getting the current thread [%d]", errorCode);
        exit(1);
    }
    return currentThread;
}

// print the stack trace of the given thread
void printStackTrace(jthread* thread)
{
    jvmtiError errorCode;
    jint maxFrameCount = 30;
    jvmtiFrameInfo* frames = (jvmtiFrameInfo*)malloc(sizeof(jvmtiFrameInfo) * maxFrameCount);
    jint count = 0;
    char* methodName = NULL;
    char* declaringClassName = NULL;
    jclass declaringClass;

    errorCode = jvmti->GetStackTrace(*thread, 0, maxFrameCount, frames, &count);
    if (errorCode != JVMTI_ERROR_NONE ) {
        jvmtiThreadInfo threadInfo;
        jvmti->GetThreadInfo(*thread, &threadInfo);
        logError("Error getting the stack trace of thread [%s]", threadInfo.name);
    }

    for (int i = 0; i < count; i++) {
        errorCode = jvmti->GetMethodName(frames[i].method, &methodName, NULL, NULL);
        if (errorCode == JVMTI_ERROR_NONE) {
            errorCode = jvmti->GetMethodDeclaringClass(frames[i].method, &declaringClass);
            errorCode = jvmti->GetClassSignature(declaringClass, &declaringClassName, NULL);
            if (errorCode == JVMTI_ERROR_NONE) {
                logMessage("%s::%s()", declaringClassName, methodName);
            }
        }
    }

    if (methodName != NULL) {
        jvmti->Deallocate((unsigned char*)methodName);
    }

    if (declaringClassName != NULL) {
        jvmti->Deallocate((unsigned char*)declaringClassName);
    }

    free(frames);
}

void preProcess(const char* jniFunctionName)
{
    jvmtiThreadInfo threadInfo;

    jthread* currentThread = getCurrentThread();
    jvmti->GetThreadInfo(*currentThread, &threadInfo);
    logMessage("--- [Thread: %s] %s ---", threadInfo.name, jniFunctionName);
    printStackTrace(currentThread);
    logMessage("\n");
    free(currentThread);
}

void postProcess(const char* jniFunctionName)
{
}

//////////////////////////////////////////////////////////////////////

void* newGetPrimitiveArrayCritical(JNIEnv *env, jarray array, jboolean *isCopy)
{
    preProcess("GetPrimitiveArrayCritical");
    void* result = originalJniFunctions->GetPrimitiveArrayCritical(env, array, isCopy);
    postProcess("GetPrimitiveArrayCritical");
    return result;
}

void newReleasePrimitiveArrayCritical(JNIEnv *env, jarray array, void* carray, jint mode)
{
    preProcess("ReleasePrimitiveArrayCritical");
    originalJniFunctions->ReleasePrimitiveArrayCritical(env, array, carray, mode);
    postProcess("ReleasePrimitiveArrayCritical");
}

//////////////////////////////////////////////////////////////////////

static void JNICALL gc_start(jvmtiEnv* jvmti_env)
{
    logMessage("GC start");
}

static void JNICALL gc_finish(jvmtiEnv* jvmti_env)
{
    logMessage("GC end");
}

//////////////////////////////////////////////////////////////////////

static void JNICALL vm_init(jvmtiEnv *jvmti, JNIEnv *env, jthread thread)
{
    jvmtiError errorCode = jvmti->GetJNIFunctionTable(&originalJniFunctions);
    if (errorCode != JVMTI_ERROR_NONE) {
        logError("Error getting JNI function table [%d]", errorCode);
        exit(1);
    }

    errorCode = jvmti->GetJNIFunctionTable(&replacedJniFunctions);
    if (errorCode != JVMTI_ERROR_NONE) {
        logError("Error getting JNI function table [%d]", errorCode);
        exit(1);
    }

    replacedJniFunctions->GetPrimitiveArrayCritical = newGetPrimitiveArrayCritical;
    replacedJniFunctions->ReleasePrimitiveArrayCritical = newReleasePrimitiveArrayCritical;

    errorCode = jvmti->SetJNIFunctionTable(replacedJniFunctions);
    if (errorCode != JVMTI_ERROR_NONE) {
        logError("Error setting JNI function table, err [%d]", errorCode);
        exit(1);
    }
    logMessage("Registered custom GetPrimitiveArrayCritical implementation");
    logMessage("Registered custom ReleasePrimitiveArrayCritical implementation");
}

static void _StartFunction(jvmtiEnv* jvmti_env, JNIEnv* jni_env, void* arg) {

}

jthread jthr(JNIEnv *env) {
    jclass thrClass;
    jmethodID cid;
    jthread res;

    thrClass = env->FindClass("java/lang/Thread");
    cid = env->GetMethodID(thrClass, "<init>", "()V");
    res = env->NewObject(thrClass, cid);
    return res;
}

JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM *vm, char *options, void *reserved) {
    jvmtiError errorCode;
    jvmtiCapabilities   capabilities;
    jvmtiEventCallbacks callbacks;
    return 0;

    jint returnCode = vm->GetEnv((void **)&jvmti, JVMTI_VERSION);
    if (returnCode != JNI_OK) {
        logError("Cannot get jvmti environment [%d]", returnCode);
        exit(1);
    }

    errorCode = jvmti->GetCapabilities(&capabilities);
    if (errorCode != JVMTI_ERROR_NONE) {
        logError("Error getting capabilities [%d]", errorCode);
        exit(1);
    }

    capabilities.can_generate_garbage_collection_events = 1;
    capabilities.can_signal_thread = 1;

    errorCode = jvmti->AddCapabilities(&capabilities);
    if (errorCode != JVMTI_ERROR_NONE) {
        logError("Error adding capabilities [%d]", errorCode);
        exit(1);
    }

    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.VMInit                  = &vm_init;
    callbacks.GarbageCollectionStart  = &gc_start;
    callbacks.GarbageCollectionFinish = &gc_finish;
    jvmti->SetEventCallbacks(&callbacks, sizeof(callbacks));
    jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_VM_INIT, NULL);
    jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_GARBAGE_COLLECTION_START, NULL);
    jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_GARBAGE_COLLECTION_FINISH, NULL);
    logMessage("jni_stack agent is loaded");


//    errorCode = jvmti->RunAgentThread(jthr(vm_get_env()), _StartFunction, NULL, JVMTI_THREAD_MIN_PRIORITY);


    return JNI_OK;
}

JNIEXPORT void JNICALL Agent_OnUnload(JavaVM *vm)
{
    logMessage("Unloading jni_stack agent");
}


#if 0
#include "inspector.h"
#include <stdio.h>
#include <memory>
using namespace std;

extern "C" JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM *jvm, char *options, void *reserved) {
   /* We return JNI_OK to signify success */
   printf("++++++++++hello, jvmti!\n");
   return JNI_OK;
}

static void check_jvmti_error(jvmtiEnv *jvmti, jvmtiError errnum,
        const char *str) {
    if (errnum != JVMTI_ERROR_NONE) {
        char *errnum_str;

        errnum_str = NULL;
        (void) jvmti->GetErrorName(errnum, &errnum_str);

        printf("ERROR: JVMTI: %d(%s): %s\n", errnum,
                (errnum_str == NULL ? "Unknown" : errnum_str),
                (str == NULL ? "" : str));
    }
}

JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM *jvm, char *options, void *reserved)
{
    jvmtiError error;
    jint res;
    jvmtiEnv *jvmti = NULL;

    /* Setup initial global agent data area
    *   Use of static/extern data should be handled carefully here.
    *   We need to make sure that we are able to cleanup after ourselves
    *     so anything allocated in this library needs to be freed in
    *     the Agent_OnUnload() function.
    */

    /*  We need to first get the jvmtiEnv* or JVMTI environment */

    res = jvm->GetEnv((void **) &jvmti, JVMTI_VERSION);

    if (res != JNI_OK || jvmti == NULL)
    {
        /* This means that the VM was unable to obtain this version of the
        *   JVMTI interface, this is a fatal error.
        */
        printf("ERROR: Unable to access JVMTI Version 1 (0x%x),"
            " is your J2SE a 1.5 or newer version?"
            " JNIEnv's GetEnv() returned %d\n",
            JVMTI_VERSION_1, res);
        return JNI_OK;

    }

    /* Here we save the jvmtiEnv* for Agent_OnUnload(). */
    jvmti_agent = std::shared_ptr(new Darkfall::JVM::JVMTIAgent(jvm, jvmti));


    jvmtiCapabilities jvmti_supported_interpreter_capabilities =
    {
        0, // can_tag_objects
        1, // can_generate_field_modification_events
        1, // can_generate_field_access_events
        1, // can_get_bytecodes
        0, // can_get_synthetic_attribute
        0, // can_get_owned_monitor_info
        0, // can_get_current_contended_monitor
        0, // can_get_monitor_info
        0, // can_pop_frame
        1, // can_redefine_classes
        0, // can_signal_thread
        1, // can_get_source_file_name
        1, // can_get_line_numbers
        1, // can_get_source_debug_extension
        1, // can_access_local_variables
        0, // can_maintain_original_method_order
        1, // can_generate_single_step_events
        1, // can_generate_exception_events
        1, // can_generate_frame_pop_events
        1, // can_generate_breakpoint_events
        1, // can_suspend
        1, // can_redefine_any_class
        0, // can_get_current_thread_cpu_time
        0, // can_get_thread_cpu_time
        1, // can_generate_method_entry_events
        1, // can_generate_method_exit_events
        1, // can_generate_all_class_hook_events
        1, // can_generate_compiled_method_load_events
        0, // can_generate_monitor_events
        0, // can_generate_vm_object_alloc_events
        1, // can_generate_native_method_bind_events
        0, // can_generate_garbage_collection_events
        0  // can_generate_object_free_events
    };


    error = jvmti->AddCapabilities(&jvmti_supported_interpreter_capabilities);
    check_jvmti_error(jvmti, error, "Unable to get necessary JVMTI capabilities.");


    jvmtiEventCallbacks callbacks = {NULL};
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.ClassLoad               = reinterpret_cast(jvmti_callback_class_load);
    callbacks.ClassPrepare            = reinterpret_cast(jvmti_callback_class_prepare);
    callbacks.ClassFileLoadHook       = reinterpret_cast(jvmti_callback_class_file_load_hook);
    callbacks.FieldAccess             = reinterpret_cast(jvmti_callback_field_access);
    callbacks.FieldModification       = reinterpret_cast(jvmti_callback_field_modification);
    callbacks.MethodEntry             = reinterpret_cast(jvmti_callback_method_entry);
    callbacks.MethodExit              = reinterpret_cast(jvmti_callback_method_exit);
    callbacks.NativeMethodBind        = reinterpret_cast(jvmti_callback_native_method_bind);
    callbacks.VMDeath                 = reinterpret_cast(jvmti_callback_vm_death);
    callbacks.VMInit                  = reinterpret_cast(jvmti_callback_vm_init);
    callbacks.VMStart                 = reinterpret_cast(jvmti_callback_vm_start);

    //cb.DynamicCodeGenerated    = reinterpret_cast(jvmti_callback_dynamic_code_generated);
    //cb.SingleStep              = jvmti_callback_single_step;
    //cb.Breakpoint              = jvmti_callback_breakpoint;
    //cb.FramePop                = jvmti_callback_frame_pop;
    //cb.Exception               = jvmti_callback_exception;
    //cb.ExceptionCatch          = jvmti_callback_exception_catch;
    //cb.ThreadStart             = jvmti_callback_thread_start;
    //cb.ThreadEnd               = jvmti_callback_thread_end;
    //cb.CompiledMethodLoad      = jvmti_callback_compiled_method_load;
    //cb.CompiledMethodUnload    = jvmti_callback_compiled_method_unload;
    //cb.DataDumpRequest         = jvmti_callback_data_dump_request;
    //cb.MonitorContendedEnter   = jvmti_callback_monitor_contended_enter;
    //cb.MonitorContendedEntered = jvmti_callback_monitor_contended_entered;
    //cb.MonitorWait             = jvmti_callback_monitor_wait;
    //cb.MonitorWaited           = jvmti_callback_monitor_waited;
    //cb.VMObjectAlloc           = jvmti_callback_vm_object_alloc;
    //cb.ObjectFree              = jvmti_callback_object_free;
    //cb.GarbageCollectionStart  = jvmti_callback_garbage_collection_start;
    //cb.GarbageCollectionFinish = jvmti_callback_garbage_collection_finish;

    error = jvmti->SetEventCallbacks(&callbacks, (jint)sizeof(callbacks));
    check_jvmti_error(jvmti, error, "Cannot set jvmti callbacks");

    /* At first the only initial events we are interested in are VM
    *   initialization, VM death, and Class File Loads.
    *   Once the VM is initialized we will request more events.
    */

    error = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_CLASS_LOAD, reinterpret_cast(NULL));
    error = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_CLASS_PREPARE, reinterpret_cast(NULL));
    error = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_CLASS_FILE_LOAD_HOOK, reinterpret_cast(NULL));
    error = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_FIELD_ACCESS, reinterpret_cast(NULL));
    error = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_FIELD_MODIFICATION, reinterpret_cast(NULL));
    error = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_METHOD_ENTRY, reinterpret_cast(NULL));
    error = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_METHOD_EXIT, reinterpret_cast(NULL));
    error = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_NATIVE_METHOD_BIND, reinterpret_cast(NULL));
    error = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_VM_DEATH, reinterpret_cast(NULL));
    error = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_VM_INIT, reinterpret_cast(NULL));
    error = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_VM_START, reinterpret_cast(NULL));

    //error = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_DYNAMIC_CODE_GENERATED, reinterpret_cast(NULL));
    //error = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_VM_OBJECT_ALLOC, reinterpret_cast(NULL));
    //error = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_SINGLE_STEP, reinterpret_cast(NULL));
    //error = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_BREAKPOINT, reinterpret_cast(NULL));

    check_jvmti_error(jvmti, error, "Cannot set event notification");


    /* Here we create a raw monitor for our use in this agent to
    *   protect critical sections of code.
    */
    //error = jvmti->CreateRawMonitor(jvmti, "agent data", &(gdata->lock));
    //check_jvmti_error(jvmti, error, "Cannot create raw monitor");


    /* We return JNI_OK to signify success */
    return JNI_OK;


}
#endif
