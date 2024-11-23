package com.kyant.disasm

class Instruction(
    val address: Long,
    val bytes: ByteArray,
    val mnemonic: String,
    val operands: String,
)
