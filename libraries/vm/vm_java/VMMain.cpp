#include "VMMain.h"
#include <stdio.h>
#include <string.h>
#include <eosiolib_native/vm_api.h>

/*
 * Class:     vmapi4java
 * Method:    sayHello
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_VMMain_sayHello(JNIEnv *env, jobject jobj) {
   printf("++++++++++++++hello,world\n");
}

JNIEXPORT void JNICALL Java_VMMain_apply(JNIEnv *env, jobject obj, jlong p1, jlong p2, jlong p3) {
   printf("++++++%lld %lld %lld \n", p1, p2, p3);
}

/*
 * Class:     VMMain
 * Method:    is_account
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_VMMain_is_1account
  (JNIEnv *, jobject, jlong account){
   return is_account((uint64_t)account);
}

/*
 * Class:     VMMain
 * Method:    s2n
 * Signature: (Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_VMMain_s2n
  (JNIEnv *env, jobject o, jstring str){
   jboolean isCopy = false;
   const char* _str = env->GetStringUTFChars(str, &isCopy);
   printf("+++++++++isCopy %d %s\n", isCopy, _str);
   return get_vm_api()->string_to_uint64(_str);
}

/*
 * Class:     VMMain
 * Method:    n2s
 * Signature: (J)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_VMMain_n2s
  (JNIEnv *env, jobject o, jlong account){
   char name[32];
   memset(name, 0, sizeof(name));
   get_vm_api()->uint64_to_string(account, name, sizeof(name));
   return env->NewStringUTF(name);
}

/*
 * Class:     VMMain
 * Method:    action_data_size
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_VMMain_action_1data_1size
  (JNIEnv *env, jobject o){
   return action_data_size();
}

/*
 * Class:     VMMain
 * Method:    read_action_data
 * Signature: ()[B
 */
JNIEXPORT jbyteArray JNICALL Java_VMMain_read_1action_1data
  (JNIEnv *env, jobject){
   uint32_t size = action_data_size();
   char *buf = new char[size];
   read_action_data(buf, size);

   jbyteArray jarr = env->NewByteArray(10);
   env->SetByteArrayRegion(jarr, 0, size, (jbyte*) buf);
   delete[] buf;
   return jarr;
}

/*
 * Class:     VMMain
 * Method:    require_recipient
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_VMMain_require_1recipient
  (JNIEnv *, jobject, jlong account){
   require_recipient(account);
}

/*
 * Class:     VMMain
 * Method:    require_auth
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_VMMain_require_1auth
  (JNIEnv *, jobject, jlong account){
   require_auth(account);
}

/*
 * Class:     VMMain
 * Method:    db_store_i64
 * Signature: (JJJJ[B)I
 */
JNIEXPORT jint JNICALL Java_VMMain_db_1store_1i64
  (JNIEnv *env, jobject, jlong scope, jlong table_id, jlong payer, jlong id, jbyteArray jdata){

   jsize len = env->GetArrayLength(jdata);
   char* buf = new char[len];
   env->GetByteArrayRegion(jdata, 0, len, (jbyte*)buf);

   int itr = db_store_i64(scope, table_id, payer, id, buf, len);
   delete[] buf;
   return itr;
}

/*
 * Class:     VMMain
 * Method:    db_update_i64
 * Signature: (I[B)V
 */
JNIEXPORT void JNICALL Java_VMMain_db_1update_1i64
  (JNIEnv *env, jobject, jint itr, jlong payer, jbyteArray jdata){

   jsize len = env->GetArrayLength(jdata);
   char* buf = new char[len];
   env->GetByteArrayRegion(jdata, 0, len, (jbyte*)buf);
   delete[] buf;
   db_update_i64(itr, payer, buf, len);
   return;
}

/*
 * Class:     VMMain
 * Method:    db_remove_i64
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_VMMain_db_1remove_1i64
  (JNIEnv *, jobject, jint itr){
   db_remove_i64(itr);
}

/*
 * Class:     VMMain
 * Method:    db_get_i64
 * Signature: (I)[B
 */
JNIEXPORT jbyteArray JNICALL Java_VMMain_db_1get_1i64
  (JNIEnv *env, jobject o, jint itr){
   int size = db_get_i64(itr, nullptr, 0);
   if (size == 0) {
      return NULL;
   }
   char *buf = new char[size];
   db_get_i64(itr, buf, size);

   jbyteArray jarr = env->NewByteArray(10);
   env->SetByteArrayRegion(jarr, 0, size, (jbyte*) buf);
   return jarr;
}

/*
 * Class:     VMMain
 * Method:    db_next_i64
 * Signature: (I)J
 */
JNIEXPORT jlong JNICALL Java_VMMain_db_1next_1i64
  (JNIEnv *, jobject, jint itr){
   uint64_t primary = 0;
   return db_next_i64(itr, &primary);
}

/*
 * Class:     VMMain
 * Method:    db_previous_i64
 * Signature: (I)J
 */
JNIEXPORT jlong JNICALL Java_VMMain_db_1previous_1i64
  (JNIEnv *, jobject, jint itr){
   uint64_t primary = 0;
   return db_previous_i64(itr, &primary);
}

/*
 * Class:     VMMain
 * Method:    db_find_i64
 * Signature: (JJJJ)I
 */
JNIEXPORT jint JNICALL Java_VMMain_db_1find_1i64
  (JNIEnv *, jobject, jlong code, jlong scope, jlong table_id, jlong id){
   return db_find_i64(code, scope, table_id, id);
}

/*
 * Class:     VMMain
 * Method:    db_lowerbound_i64
 * Signature: (JJJJ)I
 */
JNIEXPORT jint JNICALL Java_VMMain_db_1lowerbound_1i64
  (JNIEnv *, jobject, jlong code, jlong scope, jlong table_id, jlong id){
   return db_lowerbound_i64(code, scope, table_id, id);
}

/*
 * Class:     VMMain
 * Method:    db_upperbound_i64
 * Signature: (JJJJ)I
 */
JNIEXPORT jint JNICALL Java_VMMain_db_1upperbound_1i64
  (JNIEnv *, jobject, jlong code, jlong scope, jlong table_id, jlong id){
   return db_upperbound_i64(code, scope, table_id, id);
}

/*
 * Class:     VMMain
 * Method:    db_end_i64
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_VMMain_db_1end_1i64
  (JNIEnv *, jobject, jlong code, jlong scope, jlong table_id){
   return db_end_i64(code, scope, table_id);
}
