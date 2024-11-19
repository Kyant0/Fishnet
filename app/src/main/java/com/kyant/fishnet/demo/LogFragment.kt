@file:Suppress("DEPRECATION", "OVERRIDE_DEPRECATION")

package com.kyant.fishnet.demo

import android.app.Fragment
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Button
import android.widget.TextView
import java.io.File

class LogFragment : Fragment() {
    private lateinit var logTextView: TextView

    override fun onCreateView(inflater: LayoutInflater?, container: ViewGroup?, savedInstanceState: Bundle?): View? {
        return inflater?.inflate(R.layout.fragment_log, container, false)
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        logTextView = view.findViewById(R.id.tv_log)
        logTextView.text = getLog()

        view.findViewById<Button>(R.id.btn_refresh_log).setOnClickListener {
            logTextView.text = getLog()
        }
    }

    private fun getLog(): String {
        return try {
            val logFile = File("/data/data/com.kyant.fishnet.demo/files/fishnet.log")
            logFile.readText()
        } catch (e: Exception) {
            e.printStackTrace()
            "Failed to read log file: ${e.message}"
        }
    }
}
