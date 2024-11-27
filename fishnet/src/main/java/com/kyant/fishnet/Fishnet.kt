package com.kyant.fishnet

import android.content.Context
import android.content.pm.PackageManager

object Fishnet {
    fun init(context: Context, fd: Int): Boolean {
        val packageName = context.packageName
        val packageInfo = context.packageManager.getPackageInfo(packageName, PackageManager.GET_SIGNING_CERTIFICATES)
        val versionName = packageInfo.versionName ?: "???"
        val versionCode = packageInfo.longVersionCode
        val signingInfo = packageInfo.signingInfo
        val cert = if (signingInfo != null) {
            if (!signingInfo.hasMultipleSigners()) {
                signingInfo.signingCertificateHistory?.getOrNull(0)?.toChars()?.take(8)?.joinToString("")
            } else {
                signingInfo.apkContentsSigners?.joinToString { it.toChars().take(8).joinToString("") }
            } ?: "???"
        } else {
            "???"
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
