package com.kyant.fishnet.demo

import android.app.Application
import android.net.Uri
import com.kyant.fishnet.Fishnet
import com.kyant.fishnet.JavaExceptionHandler
import java.io.File

class FishnetApp : Application() {
    override fun onCreate() {
        super.onCreate()

        val logFile = File(filesDir, "fishnet.log")
        if (!logFile.exists()) {
            logFile.createNewFile()
        }

        contentResolver.openFileDescriptor(Uri.fromFile(logFile), "rw", null)?.use { pfd ->
            if (!Fishnet.init(this, pfd.detachFd())) {
                throw RuntimeException("Failed to initialize Fishnet")
            }
        }

        JavaExceptionHandler.init(this)

        System.loadLibrary("com.kyant.fishnet.demo")
    }
}
