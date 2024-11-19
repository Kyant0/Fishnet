package com.kyant.fishnet

object Fishnet {
    external fun init(fd: Int): Boolean

    init {
        System.loadLibrary("com.kyant.fishnet")
    }
}
