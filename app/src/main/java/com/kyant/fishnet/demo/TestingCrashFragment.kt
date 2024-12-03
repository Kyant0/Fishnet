@file:Suppress("DEPRECATION", "OVERRIDE_DEPRECATION")

package com.kyant.fishnet.demo

import android.app.Fragment
import android.os.Build
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup

class TestingCrashFragment : Fragment() {
    override fun onCreateView(inflater: LayoutInflater?, container: ViewGroup?, savedInstanceState: Bundle?): View? {
        return inflater?.inflate(R.layout.fragment_testing_crash, container, false)
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        view.findViewById<View>(R.id.btn_test_java_crash).setOnClickListener {
            javaCrash()
        }
        view.findViewById<View>(R.id.btn_test_java_thread_crash).setOnClickListener {
            javaThreadCrash()
        }
        view.findViewById<View>(R.id.btn_test_java_deep_exception).setOnClickListener {
            javaDeepException()
        }
        view.findViewById<View>(R.id.btn_test_anr).setOnClickListener {
            Thread.sleep(20_000)
        }

        view.findViewById<View>(R.id.btn_test_native_nullptr).setOnClickListener {
            nativeCrash("nullptr")
        }
        view.findViewById<View>(R.id.btn_test_native_jni_error).setOnClickListener {
            nativeCrash("jni")
        }
        view.findViewById<View>(R.id.btn_test_native_deadlock).setOnClickListener {
            nativeCrash("deadlock")
        }
        view.findViewById<View>(R.id.btn_test_native_too_many_open_files).setOnClickListener {
            nativeCrash("too_many_open_files")
        }
        view.findViewById<View>(R.id.btn_test_native_buffer_overflow).setOnClickListener {
            nativeCrash("buffer_overflow")
        }
        view.findViewById<View>(R.id.btn_test_native_scudo_error).setOnClickListener {
            nativeCrash("scudo_error")
        }
        view.findViewById<View>(R.id.btn_test_native_fdsan).setOnClickListener {
            nativeFdsanCrash()
        }
    }

    private fun javaCrash() {
        throw RuntimeException("Java crash")
    }

    private fun javaThreadCrash() {
        val thread = object : Thread() {
            override fun run() {
                throw RuntimeException("Java thread crash")
            }
        }
        thread.start()
    }

    private fun javaDeepException() {
        val context = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) context else activity
        LayoutInflater.from(context).inflate(R.layout.bad_layout, null)
    }

    private external fun nativeCrash(type: String)

    private external fun nativeFdsanCrash()
}
