package com.kyant.fishnet

import android.content.Context
import android.content.Intent
import kotlin.system.exitProcess

object JavaExceptionHandler {
    private var handler: Thread.UncaughtExceptionHandler? = null
    private var intent: Intent? = null
    private var mainThread: Thread? = null

    fun init(context: Context, intent: Intent?): Boolean {
        this.intent = intent
        mainThread = Thread.currentThread()
        if (handler == null) {
            handler = Thread.UncaughtExceptionHandler { _, e -> processThrowable(context, e) }
        }
        Thread.setDefaultUncaughtExceptionHandler(handler)
        return false
    }

    @JvmStatic
    private fun dumpJavaThreads(): String {
        val stackTraces = Thread.getAllStackTraces().map { (t, s) ->
            val stackTrace = s.withIndex().joinToString("\n    ") { (i, e) ->
                "#${i.toString().padStart(2, '0')} $e"
            }
            "  🧵Thread: ${t.toLogString()}\n    $stackTrace"
        }
        return stackTraces.joinToString("\n\n")
    }

    private fun processThrowable(context: Context, e: Throwable) {
        e.printStackTrace()

        val crashingThreadStackTrace = "     💥 $e\n    " +
                e.stackTrace.withIndex().joinToString("\n    ") { (i, e) ->
                    "#${i.toString().padStart(2, '0')} $e"
                }
        val stackTraces = Thread.getAllStackTraces().map { (t, s) ->
            val stackTrace = s.withIndex().joinToString("\n    ") { (i, e) ->
                "#${i.toString().padStart(2, '0')} $e"
            }
            "  🧵Thread: ${t.toLogString()}\n    $stackTrace"
        }.joinToString("\n\n")
        Fishnet.dumpJavaCrash(
            "  🧵Crashing thread: ${Thread.currentThread().toLogString()}\n" +
                    crashingThreadStackTrace + "\n\n" +
                    stackTraces
        )

        if (intent != null) {
            context.startActivity(intent)
        }
        exitProcess(0)
    }

    private fun Thread.toLogString(): String {
        return "$name (id: $id, priority: $priority, state: $state)"
    }
}
