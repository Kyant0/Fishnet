#include <jni.h>

#include "capstone/capstone.h"

#if defined(__arm__)
#define CS_ARCH CS_ARCH_ARM
#define CS_MODE CS_MODE_ARM
#elif defined(__aarch64__)
#define CS_ARCH CS_ARCH_AARCH64
#define CS_MODE CS_MODE_LITTLE_ENDIAN
#elif defined(__i386__)
#define CS_ARCH CS_ARCH_X86
#define CS_MODE CS_MODE_32
#elif defined(__riscv)
#define CS_ARCH CS_ARCH_RISCV
#define CS_MODE CS_MODE_64
#elif defined(__x86_64__)
#define CS_ARCH CS_ARCH_X86
#define CS_MODE CS_MODE_64
#endif

jclass instruction_class;
jmethodID instruction_constructor;

JNIEXPORT jobjectArray JNICALL
Java_com_kyant_disasm_DisAsm_disasm(JNIEnv *env, jobject, jbyteArray bytes, jlong address) {
    jbyte *data = (*env)->GetByteArrayElements(env, bytes, nullptr);
    const size_t size = (*env)->GetArrayLength(env, bytes);

    csh handle;
    cs_insn *insn;

    if (cs_open(CS_ARCH, CS_MODE, &handle) != CS_ERR_OK) {
        return nullptr;
    }

    const size_t count = cs_disasm(handle, (uint8_t *) data, size, address, 0, &insn);
    if (count == 0) {
        cs_close(&handle);
        return nullptr;
    }

    jobjectArray instructions = (*env)->NewObjectArray(env, count, instruction_class, nullptr);
    for (size_t i = 0; i < count; i++) {
        const cs_insn *current = &insn[i];
        const jbyteArray current_bytes = (*env)->NewByteArray(env, current->size);
        (*env)->SetByteArrayRegion(env, current_bytes, 0, current->size, (const jbyte *) current->bytes);

        const jobject instruction = (*env)->NewObject(env, instruction_class, instruction_constructor,
                                                      (jlong) current->address, current_bytes,
                                                      (*env)->NewStringUTF(env, current->mnemonic),
                                                      (*env)->NewStringUTF(env, current->op_str));
        (*env)->SetObjectArrayElement(env, instructions, i, instruction);
    }

    cs_free(insn, count);
    cs_close(&handle);

    return instructions;
}

JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env;
    if ((*vm)->GetEnv(vm, (void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }

    jclass _instruction_class = (*env)->FindClass(env, "com/kyant/disasm/Instruction");
    instruction_class = (*env)->NewGlobalRef(env, _instruction_class);
    instruction_constructor = (*env)->GetMethodID(
            env, instruction_class, "<init>", "(J[BLjava/lang/String;Ljava/lang/String;)V");
    (*env)->DeleteLocalRef(env, _instruction_class);

    return JNI_VERSION_1_6;
}

JNIEXPORT void JNI_OnUnload(JavaVM *vm, void *) {
    JNIEnv *env;
    if ((*vm)->GetEnv(vm, (void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        return;
    }

    (*env)->DeleteGlobalRef(env, instruction_class);

    instruction_class = nullptr;

    instruction_constructor = nullptr;
}
