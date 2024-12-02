package com.kyant.fishnet;

import androidx.annotation.NonNull;

import java.util.Map;

final class JavaExceptionHandler {
    private JavaExceptionHandler() {
    }

    private static Thread.UncaughtExceptionHandler defaultHandler = null;
    private static Thread.UncaughtExceptionHandler handler = null;

    public static void init() {
        if (handler == null) {
            defaultHandler = Thread.getDefaultUncaughtExceptionHandler();
            handler = (t, e) -> {
                String crashingThreadStackTrace = "    💥 " + e + "\n    " +
                        getStackTraceString(e.getStackTrace());
                String stackTraces = getAllStackTracesExcept(t);
                NativeSignalHandler.dumpJavaCrash(
                        "  🧵Crashing thread: " + toLogString(t) + '\n' +
                                crashingThreadStackTrace + '\n' +
                                stackTraces
                );

                if (defaultHandler != null) {
                    defaultHandler.uncaughtException(t, e);
                }
            };
        }
        Thread.setDefaultUncaughtExceptionHandler(handler);
    }

    @NonNull
    private static String getStackTraceString(@NonNull StackTraceElement[] stackTrace) {
        StringBuilder sb = new StringBuilder(128);
        for (StackTraceElement element : stackTrace) {
            sb.append("at ").append(element).append("\n    ");
        }
        return sb.toString();
    }

    @NonNull
    private static String getAllStackTracesExcept(@NonNull Thread excludedThread) {
        StringBuilder sb = new StringBuilder(128);
        for (Map.Entry<Thread, StackTraceElement[]> entry : Thread.getAllStackTraces().entrySet()) {
            Thread t = entry.getKey();
            StackTraceElement[] stackTrace = entry.getValue();
            if (t.getId() != excludedThread.getId()) {
                sb.append("  🧵Thread: ").append(toLogString(t)).append("\n    ");
                sb.append(getStackTraceString(stackTrace));
                sb.append('\n');
            }
        }
        return sb.toString();
    }

    /**
     * @noinspection unused
     */
    @NonNull
    private static String dumpJavaThreads() {
        StringBuilder sb = new StringBuilder(1024);
        for (Map.Entry<Thread, StackTraceElement[]> entry : Thread.getAllStackTraces().entrySet()) {
            Thread t = entry.getKey();
            StackTraceElement[] stackTrace = entry.getValue();
            sb.append("  🧵Thread: ").append(toLogString(t)).append("\n    ");
            sb.append(getStackTraceString(stackTrace));
            sb.append('\n');
        }
        return sb.toString();
    }

    @NonNull
    private static String toLogString(@NonNull Thread t) {
        return "\"" + t.getName() + "\" " + (t.isDaemon() ? "daemon " : "") +
                "prio=" + t.getPriority() + " id=" + t.getId() + "\n" +
                "   java.lang.Thread.State=" + t.getState();
    }
}