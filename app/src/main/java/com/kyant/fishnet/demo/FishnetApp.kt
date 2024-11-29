package com.kyant.fishnet.demo

import android.app.Application
import android.content.Context
import android.content.Intent
import com.kyant.fishnet.Fishnet
import com.kyant.fishnet.JavaExceptionHandler

class FishnetApp : Application() {
    override fun onCreate() {
        super.onCreate()
        System.loadLibrary("com.kyant.fishnet.demo")
    }

    override fun attachBaseContext(base: Context?) {
        super.attachBaseContext(base)

        if (!Fishnet.init(this, filesDir.absolutePath)) {
            throw RuntimeException("Failed to initialize Fishnet")
        }

        JavaExceptionHandler.init(
            this,
            Intent(this, JavaCrashReporterActivity::class.java).apply {
                addFlags(Intent.FLAG_ACTIVITY_NEW_TASK or Intent.FLAG_ACTIVITY_CLEAR_TASK)
            }
        )
    }
}
