package com.kyant.fishnet

data class FishnetLog(
    val header: Header,
    val registers: List<Register>,
) {
    data class Header(
        val buildFingerprint: String,
        val revision: String,
        val abi: String,
        val timestamp: String,
        val processUpTime: String,
        val cmdline: String,
        val pid: String,
        val tid: String,
        val name: String,
        val uid: String,
        val taggedAddrCtrl: String,
        val pacEnabledKeys: String,
        val signal: String,
        val code: String,
        val faultAddr: ULong?,
        val cause: String,
        val abortMessage: String,
    )

    data class Register(
        val name: String,
        val value: ULong
    )
}
