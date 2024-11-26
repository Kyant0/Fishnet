@file:Suppress("DEPRECATION", "OVERRIDE_DEPRECATION")

package com.kyant.fishnet.demo

import android.app.Fragment
import android.os.Build
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.TextView
import java.io.File

class LogFragment : Fragment() {
    override fun onCreateView(inflater: LayoutInflater?, container: ViewGroup?, savedInstanceState: Bundle?): View? {
        return inflater?.inflate(R.layout.fragment_log, container, false)
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        val logTextView = view.findViewById<TextView>(R.id.tv_log)
        val progressBar = view.findViewById<View>(R.id.progress_bar)
        Handler(Looper.getMainLooper()).post {
            val log = getLog()
            logTextView.text = log
            progressBar.visibility = View.GONE
        }
    }

    private fun getLog(): String {
        return try {
            val logFile = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
                File(context.filesDir, "fishnet.log")
            } else {
                File(activity.filesDir, "fishnet.log")
            }
            logFile.readText().takeIf { it.isNotEmpty() }
        } catch (_: Exception) {
            null
        } ?: "No log available"
    }
}
