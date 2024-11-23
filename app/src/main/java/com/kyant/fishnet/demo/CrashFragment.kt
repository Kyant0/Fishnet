@file:Suppress("DEPRECATION", "OVERRIDE_DEPRECATION")

package com.kyant.fishnet.demo

import android.app.Fragment
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup

class CrashFragment : Fragment() {
    override fun onCreateView(inflater: LayoutInflater?, container: ViewGroup?, savedInstanceState: Bundle?): View? {
        return inflater?.inflate(R.layout.fragment_crash, container, false)
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        view.findViewById<View>(R.id.btn_test_java_crash).setOnClickListener {
            throw RuntimeException("Java Crash")
        }
        view.findViewById<View>(R.id.btn_test_anr).setOnClickListener {
            Thread.sleep(10000)
        }

        view.findViewById<View>(R.id.btn_test_native_crash).setOnClickListener {
            nativeCrash("SIGSEGV")
        }
        view.findViewById<View>(R.id.btn_test_jni_abort).setOnClickListener {
            println(jniAbort())
        }
        view.findViewById<View>(R.id.btn_test_fdsan_crash).setOnClickListener {
            nativeFdsanCrash()
        }
    }

    private external fun nativeCrash(type: String)

    private external fun jniAbort(): String

    private external fun nativeFdsanCrash()
}
