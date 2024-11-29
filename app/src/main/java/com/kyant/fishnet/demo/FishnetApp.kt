package com.kyant.fishnet.demo

import android.app.Application
import android.content.Context
import android.content.Intent
import android.net.Uri
import com.kyant.fishnet.Fishnet
import com.kyant.fishnet.JavaExceptionHandler
import java.io.File

class FishnetApp : Application() {
    override fun onCreate() {
        super.onCreate()
        System.loadLibrary("com.kyant.fishnet.demo")
    }

    override fun attachBaseContext(base: Context?) {
        super.attachBaseContext(base)

        val logFile = File(filesDir, "fishnet.log")
        if (!logFile.exists()) {
            logFile.createNewFile()
        }

        contentResolver.openFileDescriptor(Uri.fromFile(logFile), "rw", null)?.use { pfd ->
            if (!Fishnet.init(this, pfd.detachFd())) {
                throw RuntimeException("Failed to initialize Fishnet")
            }
        }

        JavaExceptionHandler.init(
            this,
            File(filesDir, "java_crashes").apply { mkdirs() },
            Intent(this, JavaCrashReporterActivity::class.java).apply {
                addFlags(Intent.FLAG_ACTIVITY_NEW_TASK or Intent.FLAG_ACTIVITY_CLEAR_TASK)
            }
        )
    }
}
