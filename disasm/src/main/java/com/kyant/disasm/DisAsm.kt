package com.kyant.disasm

object DisAsm {
    external fun disasm(bytes: ByteArray, address: Long): Array<Instruction>

    init {
        System.loadLibrary("com.kyant.disasm")
    }
}
