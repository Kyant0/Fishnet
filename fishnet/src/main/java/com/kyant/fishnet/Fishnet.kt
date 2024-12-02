package com.kyant.fishnet

import android.content.Context

object Fishnet {
    @JvmStatic
    fun init(context: Context, path: String) {
        NativeSignalHandler.init(context, path)
        JavaExceptionHandler.init()
    }
}
