package com.kyant.fishnet;

import android.content.Context;

import androidx.annotation.NonNull;

/**
 * Fishnet is an Android library that dumps Java and native crashes.
 *
 * @see Fishnet#init(Context, String)
 */
public final class Fishnet {
    private Fishnet() {
    }

    /**
     * Initialize Fishnet.
     *
     * @param context Application context
     * @param path    Absolute path to store crash logs
     */
    public static void init(@NonNull Context context, @NonNull String path) {
        NativeSignalHandler.init(context, path);
        JavaExceptionHandler.init();
    }
}
