package com.kyant.fishnet;

import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.Signature;
import android.content.pm.SigningInfo;
import android.os.Build;

final class NativeSignalHandler {
    private NativeSignalHandler() {
    }

    static void init(Context context, String path) {
        String packageName = context.getPackageName();

        PackageInfo packageInfo;
        int flags = Build.VERSION.SDK_INT >= Build.VERSION_CODES.P
                ? PackageManager.GET_SIGNING_CERTIFICATES
                : PackageManager.GET_SIGNATURES;
        try {
            packageInfo = context.getPackageManager().getPackageInfo(packageName, flags);
        } catch (PackageManager.NameNotFoundException e) {
            packageInfo = null;
        }

        if (packageInfo == null) {
            nativeInit(path, packageName, "???", 0, "???");
            return;
        }

        String versionName = packageInfo.versionName != null ? packageInfo.versionName : "???";

        long versionCode = Build.VERSION.SDK_INT >= Build.VERSION_CODES.P
                ? packageInfo.getLongVersionCode()
                : packageInfo.versionCode;

        Signature[] signatures = null;
        String cert = "???";

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            SigningInfo signingInfo = packageInfo.signingInfo;
            if (signingInfo != null) {
                if (!signingInfo.hasMultipleSigners()) {
                    signatures = signingInfo.getSigningCertificateHistory();
                } else {
                    signatures = signingInfo.getApkContentsSigners();
                }
            }
        } else {
            signatures = packageInfo.signatures;
        }

        if (signatures != null) {
            if (signatures.length == 1) {
                cert = signatures[0].toCharsString().substring(0, 8);
            } else {
                StringBuilder sb = new StringBuilder(signatures.length * 9);
                for (Signature signature : signatures) {
                    sb.append(signature.toCharsString().substring(0, 8)).append(' ');
                }
                int len = sb.length();
                if (len > 0) {
                    sb.deleteCharAt(sb.length() - 1);
                }
                cert = sb.toString();
            }
        }

        nativeInit(path, packageName, versionName, versionCode, cert);
    }

    static void dumpJavaCrash(String javaStackTraces) {
        nativeDumpJavaCrash(javaStackTraces);
    }

    private static native void nativeInit(
            String path,
            String packageName,
            String versionName,
            long versionCode,
            String cert
    );

    private static native void nativeDumpJavaCrash(String javaStackTraces);

    static {
        System.loadLibrary("com.kyant.fishnet");
    }
}
