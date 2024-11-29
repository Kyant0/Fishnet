package com.kyant.fishnet.demo

import android.app.Activity
import android.os.Bundle
import android.view.View
import android.widget.TextView
import kotlin.system.exitProcess

class JavaCrashReporterActivity : Activity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        val javaCrashes = filesDir
            .listFiles()
            ?.filter { it.isFile && it.name.startsWith("java_") }
            ?.map { it.readText() }

        setContentView(R.layout.activity_java_crash_reporter)

        findViewById<TextView>(R.id.tv_log).text = javaCrashes?.lastOrNull()?.take(200000).orEmpty()

        findViewById<View>(R.id.btn_exit).setOnClickListener {
            exitProcess(0)
        }
    }
}
