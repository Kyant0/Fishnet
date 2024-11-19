@file:Suppress("DEPRECATION", "OVERRIDE_DEPRECATION")

package com.kyant.fishnet.demo

import android.app.Fragment
import android.graphics.Typeface
import android.os.Bundle
import android.text.TextUtils
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Button
import android.widget.RadioButton
import android.widget.RadioGroup
import kotlin.math.roundToInt

class CrashFragment : Fragment() {
    override fun onCreateView(inflater: LayoutInflater?, container: ViewGroup?, savedInstanceState: Bundle?): View? {
        return inflater?.inflate(R.layout.fragment_crash, container, false)
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        view.findViewById<Button>(R.id.btn_test_java_crash).setOnClickListener {
            throw RuntimeException("Java Crash")
        }
        view.findViewById<Button>(R.id.btn_test_anr).setOnClickListener {
            Thread.sleep(10000)
        }

        view.findViewById<Button>(R.id.btn_test_native_crash).setOnClickListener {
            nativeCrash("SIGSEGV")
        }
        view.findViewById<Button>(R.id.btn_test_jni_abort).setOnClickListener {
            jniAbort()
        }
        view.findViewById<Button>(R.id.btn_test_fdsan_crash).setOnClickListener {
            nativeFdsanCrash()
        }

        view.findViewById<RadioGroup>(R.id.radio_group__signals).apply {
            signals.forEach { (value, name, desc) ->
                val radioButton = RadioButton(context).apply {
                    minHeight = (minHeight * 0.8f).roundToInt()
                    text = "${name.padEnd(12)} ${"($value)".padEnd(4)} $desc"
                    textSize = 12f
                    typeface = Typeface.MONOSPACE
                    maxLines = 1
                    ellipsize = TextUtils.TruncateAt.END
                    setOnClickListener {
                        signal(value)
                    }
                }
                addView(radioButton)
            }
        }
    }

    private external fun nativeCrash(type: String)

    private external fun signal(signal: Int)

    private external fun jniAbort()

    private external fun nativeFdsanCrash()

    private companion object {
        val signals: List<Triple<Int, String, String>> = listOf(
            Triple(1, "SIGHUP", "Hangup"),
            Triple(2, "SIGINT", "Interrupt"),
            Triple(3, "SIGQUIT", "Quit"),
            Triple(4, "SIGILL", "Illegal instruction"),
            Triple(5, "SIGTRAP", "Trap"),
            Triple(6, "SIGABRT", "Aborted"),
            Triple(6, "SIGIOT", "Aborted"),
            Triple(7, "SIGBUS", "Bus error"),
            Triple(8, "SIGFPE", "Floating point exception"),
            Triple(9, "SIGKILL", "Killed"),
            Triple(10, "SIGUSR1", "User signal 1"),
            Triple(11, "SIGSEGV", "Segmentation fault"),
            Triple(12, "SIGUSR2", "User signal 2"),
            Triple(13, "SIGPIPE", "Broken pipe"),
            Triple(14, "SIGALRM", "Alarm clock"),
            Triple(15, "SIGTERM", "Terminated"),
            Triple(16, "SIGSTKFLT", "Stack fault"),
            Triple(17, "SIGCHLD", "Child exited"),
            Triple(18, "SIGCONT", "Continue"),
            Triple(19, "SIGSTOP", "Stopped (signal)"),
            Triple(20, "SIGTSTP", "Stopped"),
            Triple(21, "SIGTTIN", "Stopped (tty input)"),
            Triple(22, "SIGTTOU", "Stopped (tty output)"),
            Triple(23, "SIGURG", "Urgent I/O condition"),
            Triple(24, "SIGXCPU", "CPU time limit exceeded"),
            Triple(25, "SIGXFSZ", "File size limit exceeded"),
            Triple(26, "SIGVTALRM", "Virtual timer expired"),
            Triple(27, "SIGPROF", "Profiling timer expired"),
            Triple(28, "SIGWINCH", "Window size changed"),
            Triple(29, "SIGIO", "I/O possible"),
            Triple(29, "SIGPOLL", "I/O possible"),
            Triple(30, "SIGPWR", "Power failure"),
        )
    }
}
