package com.kyant.fishnet

import android.content.Context
import android.content.Intent
import android.widget.Toast
import java.io.File
import kotlin.system.exitProcess

object JavaExceptionHandler {
    private var handler: Thread.UncaughtExceptionHandler? = null
    private var intent: Intent? = null
    private var path: File? = null

    fun init(context: Context, path: File, intent: Intent, launchWhenStart: Boolean = false): Boolean {
        this.intent = intent
        this.path = path
        if (launchWhenStart) {
            val crashReports = path.listFiles()
            if (!crashReports.isNullOrEmpty()) {
                context.startActivity(intent)
                return true
            }
        }
        if (handler == null) {
            handler = Thread.UncaughtExceptionHandler { _, e -> processThrowable(context, e) }
        }
        Thread.setDefaultUncaughtExceptionHandler(handler)
        return false
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
            "  🧵Thread: ${t.toLogString()})\n    $stackTrace"
        }.joinToString("\n\n")
        val text = Fishnet.dump(
            "  🧵Crashing thread: ${Thread.currentThread().toLogString()}\n" +
                    crashingThreadStackTrace + "\n\n" +
                    stackTraces
        )

        val crashReports = path?.listFiles()
        if (crashReports != null &&
            crashReports.size > 1 &&
            run {
                val last = crashReports.last()
                val secondLast = crashReports[crashReports.size - 2]
                last.readText() == secondLast.readText() &&
                        last.lastModified() - secondLast.lastModified() < 10_000L
            }
        ) {
            Toast.makeText(context, "Crash loop detected", Toast.LENGTH_LONG).show()
            exitProcess(0)
        }

        val crashReport = File(path, "${System.currentTimeMillis()}.txt")
        crashReport.writeText(text)
        context.startActivity(intent)
        exitProcess(0)
    }

    private fun Thread.toLogString(): String {
        return "$name (id: $id, priority: $priority, state: $state)"
    }
}
