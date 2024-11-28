package com.kyant.fishnet

import android.content.Context
import android.content.pm.PackageManager
import android.os.Build

object Fishnet {
    @Suppress("DEPRECATION")
    fun init(context: Context, fd: Int): Boolean {
        val packageName = context.packageName
        val packageInfo =
            context.packageManager.getPackageInfo(
                packageName,
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) PackageManager.GET_SIGNING_CERTIFICATES
                else PackageManager.GET_SIGNATURES
            )
        val versionName = packageInfo.versionName ?: "???"
        val versionCode =
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) packageInfo.longVersionCode
            else packageInfo.versionCode.toLong()
        val cert = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            val signingInfo = packageInfo.signingInfo
            if (signingInfo != null) {
                if (!signingInfo.hasMultipleSigners()) {
                    signingInfo.signingCertificateHistory?.getOrNull(0)?.toChars()?.take(8)?.joinToString("")
                } else {
                    signingInfo.apkContentsSigners?.joinToString { it.toChars().take(8).joinToString("") }
                } ?: "???"
            } else {
                "???"
            }
        } else {
            packageInfo.signatures?.joinToString { it.toChars().take(8).joinToString("") } ?: "???"
        }
        return nativeInit(fd, packageName, versionName, versionCode, cert)
    }

    private external fun nativeInit(
        fd: Int,
        packageName: String,
        versionName: String,
        versionCode: Long,
        cert: String
    ): Boolean

    init {
        System.loadLibrary("com.kyant.fishnet")
    }
}
