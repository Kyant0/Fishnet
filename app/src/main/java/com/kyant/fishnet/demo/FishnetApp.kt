package com.kyant.fishnet.demo

import android.app.Application
import android.content.Context
import com.kyant.fishnet.Fishnet
import java.io.File

class FishnetApp : Application() {
    override fun onCreate() {
        super.onCreate()
        System.loadLibrary("com.kyant.fishnet.demo")
    }

    override fun attachBaseContext(base: Context?) {
        super.attachBaseContext(base)
        val logPath = File(filesDir, "logs").apply { mkdirs() }.absolutePath
        Fishnet.init(this, logPath)
    }
}
