package com.kyant.fishnet

internal object JavaExceptionHandler {
    private var defaultHandler: Thread.UncaughtExceptionHandler? = null
    private var handler: Thread.UncaughtExceptionHandler? = null

    fun init() {
        if (handler == null) {
            defaultHandler = Thread.getDefaultUncaughtExceptionHandler()
            handler = Thread.UncaughtExceptionHandler { t, e ->
                val crashingThreadStackTrace = "    💥 $e\n    " +
                        e.stackTrace.joinToString("\n    ") { "at $it" }
                val stackTraces = Thread.getAllStackTraces().filterNot { it.key.id == t.id }.map { (t, s) ->
                    val stackTrace = s.joinToString("\n    ") { "at $it" }
                    "  🧵Thread: ${t.toLogString()}\n    $stackTrace"
                }.joinToString("\n\n")
                NativeSignalHandler.dumpJavaCrash(
                    "  🧵Crashing thread: ${t.toLogString()}\n" +
                            crashingThreadStackTrace + "\n\n" +
                            stackTraces
                )

                defaultHandler?.uncaughtException(t, e)
            }
        }
        Thread.setDefaultUncaughtExceptionHandler(handler)
    }

    @JvmStatic
    private fun dumpJavaThreads(): String {
        val stackTraces = Thread.getAllStackTraces().map { (t, s) ->
            val stackTrace = s.joinToString("\n    ") { "at $it" }
            "  🧵Thread: ${t.toLogString()}\n    $stackTrace"
        }
        return stackTraces.joinToString("\n\n")
    }

    private fun Thread.toLogString(): String {
        return "\"$name\" ${if (isDaemon) "daemon " else ""}prio=$priority id=$id\n" +
                "   java.lang.Thread.State=$state"
    }
}
