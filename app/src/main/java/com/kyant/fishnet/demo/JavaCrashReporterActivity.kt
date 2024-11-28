package com.kyant.fishnet.demo

import android.app.Activity
import android.os.Bundle
import android.view.View
import android.widget.TextView
import java.io.File
import kotlin.system.exitProcess

class JavaCrashReporterActivity : Activity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        val javaCrashes = File(filesDir, "java_crashes")
            .apply { mkdirs() }
            .listFiles()
            ?.map { it.readText() }

        if (!javaCrashes.isNullOrEmpty()) {
            setContentView(R.layout.activity_java_crash_reporter)

            findViewById<TextView>(R.id.tv_log).text = javaCrashes.lastOrNull()?.take(200000).orEmpty()

            findViewById<View>(R.id.btn_delete_log).setOnClickListener {
                File(filesDir, "java_crashes").deleteRecursively()
                exitProcess(0)
            }
        } else {
            exitProcess(0)
        }
    }
}
