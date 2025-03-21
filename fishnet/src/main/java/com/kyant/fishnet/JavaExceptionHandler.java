package com.kyant.fishnet;

import android.util.Log;

import java.util.Map;

final class JavaExceptionHandler {
    private JavaExceptionHandler() {
    }

    private static final String TAG = "Fishnet";
    private static final Object lock = new Object();

    private static Thread.UncaughtExceptionHandler defaultHandler = null;
    private static Thread.UncaughtExceptionHandler handler = null;

    public static void init() {
        synchronized (lock) {
            if (handler == null) {
                defaultHandler = Thread.getDefaultUncaughtExceptionHandler();
                handler = new FishnetUncaughtExceptionHandler();
            }
            Thread.setDefaultUncaughtExceptionHandler(handler);
        }
    }

    private static final class FishnetUncaughtExceptionHandler implements Thread.UncaughtExceptionHandler {
        @Override
        public void uncaughtException(Thread t, Throwable e) {
            if (defaultHandler != null) {
                Thread.setDefaultUncaughtExceptionHandler(defaultHandler);
            }

            try {
                dumpCrash(t, e);
            } catch (Exception exception) {
                Log.e(TAG, "Error while dumping Java crash", exception);
            } finally {
                if (defaultHandler != null) {
                    defaultHandler.uncaughtException(t, e);
                }
                defaultHandler = null;
                handler = null;
            }
        }
    }

    private static void dumpCrash(Thread t, Throwable e) {
        StringBuilder sb = new StringBuilder(1024);
        sb.append("  💥 ").append(e);
        StackTraceElement[] crashingThreadStackTrace = e.getStackTrace();
        if (crashingThreadStackTrace.length > 0) {
            sb.append("\n    ").append(getStackTraceString(e.getStackTrace()));
        }
        Throwable cause = e.getCause();
        if (cause != null) {
            sb.append('\n');
            while (cause != null) {
                sb.append("  💥 Caused by: ").append(cause);
                StackTraceElement[] causeStackTrace = cause.getStackTrace();
                if (causeStackTrace.length > 0) {
                    sb.append("\n    ").append(getStackTraceString(causeStackTrace));
                } else {
                    sb.append('\n');
                }
                cause = cause.getCause();
            }
        }
        NativeSignalHandler.dumpJavaCrash(
                "  🧵Crashing thread: " + toLogString(t) + '\n' + sb + '\n' +
                        getAllStackTracesExcept(t)
        );
    }

    private static String getStackTraceString(StackTraceElement[] stackTrace) {
        if (stackTrace.length == 0) {
            return "(empty stack trace)\n";
        }
        StringBuilder sb = new StringBuilder(512);
        for (int i = 0, stackTraceLength = stackTrace.length; i < stackTraceLength; i++) {
            StackTraceElement element = stackTrace[i];
            sb.append("at ").append(element);
            if (i < stackTraceLength - 1) {
                sb.append("\n    ");
            }
        }
        sb.append('\n');
        return sb.toString();
    }

    private static String getAllStackTracesExcept(Thread excludedThread) {
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
        int len = sb.length();
        if (len > 0) {
            sb.deleteCharAt(len - 1);
        }
        return sb.toString();
    }

    /**
     * @noinspection unused
     */
    private static String dumpJavaThreads() {
        return getAllStackTracesExcept(Thread.currentThread());
    }

    private static String toLogString(Thread t) {
        return "\"" + t.getName() + "\" " + (t.isDaemon() ? "daemon " : "") +
                "prio=" + t.getPriority() + " id=" + t.getId() + " state=" + t.getState();
    }
}
