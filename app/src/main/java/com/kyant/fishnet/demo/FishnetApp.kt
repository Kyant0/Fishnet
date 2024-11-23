package com.kyant.fishnet.demo

import android.app.Application
import android.net.Uri
import com.kyant.disasm.DisAsm
import com.kyant.fishnet.Fishnet
import java.io.File

class FishnetApp : Application() {
    override fun onCreate() {
        super.onCreate()

        val logFile = File(filesDir, "fishnet.log")
        if (!logFile.exists()) {
            logFile.createNewFile()
        }

        contentResolver.openFileDescriptor(Uri.fromFile(logFile), "rw", null)?.use { pfd ->
            if (!Fishnet.init(pfd.detachFd())) {
                throw RuntimeException("Failed to initialize Fishnet")
            }
        }

        System.loadLibrary("com.kyant.fishnet.demo")

        DisAsm.disasm(byteArrayOf(0x20.toByte(), 0x01.toByte(), 0xc0.toByte(), 0x3d.toByte()), 0x51894)
            .forEach { instruction ->
                println("0x${instruction.address.toString(16)}: ${instruction.mnemonic} ${instruction.operands}")
            }
    }
}
