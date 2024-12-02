package com.kyant.fishnet;

import android.content.Context;

import java.util.Objects;

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
    public static void init(Context context, String path) {
        Objects.requireNonNull(context, "context must not be null");
        Objects.requireNonNull(path, "path must not be null");

        NativeSignalHandler.init(context, path);
        JavaExceptionHandler.init();
    }
}
