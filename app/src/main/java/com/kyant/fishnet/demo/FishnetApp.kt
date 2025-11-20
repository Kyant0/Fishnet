package com.kyant.fishnet.demo

import android.app.Application

class FishnetApp : Application() {
    override fun onCreate() {
        super.onCreate()
        LoggingConfig.init(this)
        System.loadLibrary("fishnet_demo")
    }
}
