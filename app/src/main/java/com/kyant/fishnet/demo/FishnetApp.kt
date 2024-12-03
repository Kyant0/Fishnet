package com.kyant.fishnet.demo

import android.app.Application
import android.content.Context

class FishnetApp : Application() {
    override fun onCreate() {
        super.onCreate()
        System.loadLibrary("com.kyant.fishnet.demo")
    }

    override fun attachBaseContext(base: Context?) {
        super.attachBaseContext(base)
        LoggingConfig.init(this)
    }
}
