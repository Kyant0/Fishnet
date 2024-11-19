package com.kyant.fishnet.demo

import android.app.Application
import android.net.Uri
import com.kyant.fishnet.Fishnet
import java.io.File

class FishnetApp : Application() {
    override fun onCreate() {
        super.onCreate()

        val filesDir = File("/data/data/com.kyant.fishnet.demo/files")
        if (!filesDir.exists()) {
            filesDir.mkdirs()
        }
        val logFile = File(filesDir, "fishnet.log")
        if (!logFile.exists()) {
            logFile.createNewFile()
        }

        contentResolver.openFile(Uri.fromFile(logFile), "rw", null)?.use { pfd ->
            if (!Fishnet.init(pfd.detachFd())) {
                throw RuntimeException("Failed to initialize Fishnet")
            }
        }

        System.loadLibrary("com.kyant.fishnet.demo")
    }
}
