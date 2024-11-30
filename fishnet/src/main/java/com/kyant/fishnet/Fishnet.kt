package com.kyant.fishnet

import android.content.Context
import android.content.pm.PackageManager
import android.os.Build

object Fishnet {
    @Suppress("DEPRECATION")
    fun init(context: Context, path: String) {
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
        nativeInit(path, packageName, versionName, versionCode, cert)
    }

    fun dumpJavaCrash(javaStackTraces: String) {
        nativeDumpJavaCrash(javaStackTraces)
    }

    private external fun nativeInit(
        path: String,
        packageName: String,
        versionName: String,
        versionCode: Long,
        cert: String
    )

    private external fun nativeDumpJavaCrash(javaStackTraces: String)

    init {
        System.loadLibrary("com.kyant.fishnet")
    }
}
