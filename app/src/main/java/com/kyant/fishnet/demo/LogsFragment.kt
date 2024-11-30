@file:Suppress("DEPRECATION", "OVERRIDE_DEPRECATION")

package com.kyant.fishnet.demo

import android.app.Fragment
import android.content.Context
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.AdapterView
import android.widget.ArrayAdapter
import android.widget.Spinner
import android.widget.TextView
import java.io.File
import kotlin.math.roundToInt

class LogsFragment : Fragment() {
    override fun onCreateView(inflater: LayoutInflater?, container: ViewGroup?, savedInstanceState: Bundle?): View? {
        return inflater?.inflate(R.layout.fragment_logs, container, false)
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        val progressBar = view.findViewById<View>(R.id.progress_bar)
        val spinner = view.findViewById<Spinner>(R.id.spinner_logs)
        val scrollView = view.findViewById<View>(R.id.scroll_view)
        val horizontalScrollView = view.findViewById<View>(R.id.horizontal_scroll_view)
        val logTextView = view.findViewById<TextView>(R.id.tv_log)

        val logs: List<File> = try {
            activity.filesDir.listFiles()?.filter { it.extension == "log" }?.reversed().orEmpty()
        } catch (_: Exception) {
            emptyList()
        }

        if (logs.isNotEmpty()) {
            spinner.dropDownVerticalOffset = (42 * resources.displayMetrics.density).roundToInt()
            spinner.addOnLayoutChangeListener { _, _, _, _, _, _, _, _, _ ->
                (spinner.getChildAt(0) as TextView).apply {
                    val paddingStart = (16 * resources.displayMetrics.density).roundToInt()
                    if (layoutDirection == View.LAYOUT_DIRECTION_RTL) {
                        setPadding(0, 0, paddingStart, 0)
                    } else {
                        setPadding(paddingStart, 0, 0, 0)
                    }
                    textSize = 13f
                }
                spinner.dropDownWidth = spinner.width
            }
            spinner.adapter = LogAdapter(activity, android.R.layout.simple_spinner_dropdown_item).apply {
                addAll(logs.map { it.name })
            }
            spinner.onItemSelectedListener = object : AdapterView.OnItemSelectedListener {
                override fun onItemSelected(parent: AdapterView<*>?, view: View?, position: Int, id: Long) {
                    progressBar.visibility = View.VISIBLE
                    logTextView.visibility = View.INVISIBLE
                    Handler(Looper.getMainLooper()).post {
                        logTextView.text = try {
                            logs[position].readText()
                        } catch (e: Exception) {
                            "Failed to read log file: ${e.message}"
                        }
                        progressBar.visibility = View.GONE
                        scrollView.post {
                            scrollView.scrollTo(0, 0)
                            horizontalScrollView.scrollTo(0, 0)
                            logTextView.visibility = View.VISIBLE
                        }
                    }
                }

                override fun onNothingSelected(parent: AdapterView<*>?) {
                }
            }
        } else {
            progressBar.visibility = View.GONE
            spinner.visibility = View.GONE
            logTextView.text = "No logs found."
        }
    }

    private class LogAdapter(context: Context, resource: Int) : ArrayAdapter<String>(context, resource) {
        override fun getDropDownView(position: Int, convertView: View?, parent: ViewGroup): View {
            return super.getDropDownView(position, convertView, parent).apply {
                findViewById<TextView>(android.R.id.text1).apply {
                    text = getItem(position)
                    textSize = 14f
                }
            }
        }
    }
}
