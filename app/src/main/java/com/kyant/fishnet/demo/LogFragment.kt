@file:Suppress("DEPRECATION", "OVERRIDE_DEPRECATION")

package com.kyant.fishnet.demo

import android.app.Fragment
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.TextView

class LogFragment : Fragment() {
    override fun onCreateView(inflater: LayoutInflater?, container: ViewGroup?, savedInstanceState: Bundle?): View? {
        return inflater?.inflate(R.layout.fragment_log, container, false)
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        val logTextView = view.findViewById<TextView>(R.id.tv_log)
        val progressBar = view.findViewById<View>(R.id.progress_bar)
        Handler(Looper.getMainLooper()).post {
            getLog()?.let { logTextView.text = it }
            progressBar.visibility = View.GONE
        }
    }

    private fun getLog(): String? {
        return try {
            activity.filesDir.listFiles()?.lastOrNull()?.readText()
        } catch (_: Exception) {
            null
        }
    }
}
