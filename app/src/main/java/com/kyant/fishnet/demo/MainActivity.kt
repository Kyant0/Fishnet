package com.kyant.fishnet.demo

import android.app.Activity
import android.graphics.Outline
import android.graphics.drawable.ColorDrawable
import android.os.Build
import android.os.Bundle
import android.view.View
import android.view.ViewOutlineProvider
import android.widget.LinearLayout
import android.widget.TextView

class MainActivity : Activity() {
    @Suppress("DEPRECATION")
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        fragmentManager.beginTransaction()
            .replace(R.id.fragment_container, CrashFragment())
            .commit()

        val tabLayout = findViewById<LinearLayout>(R.id.tab_layout)
        val crashTab = findViewById<TextView>(R.id.tab_crash)
        val logTab = findViewById<TextView>(R.id.tab_log)

        val unselectedDrawable = ColorDrawable(getColor(android.R.color.system_control_activated_dark))
        val selectedDrawable =
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S)
                ColorDrawable(getColor(android.R.color.system_accent1_100))
            else
                ColorDrawable(getColor(android.R.color.darker_gray))

        val cornerRadiusPx = 12f * resources.displayMetrics.density
        tabLayout.outlineProvider = object : ViewOutlineProvider() {
            override fun getOutline(view: View, outline: Outline) {
                outline.setRoundRect(0, 0, view.width, view.height, cornerRadiusPx)
            }
        }
        tabLayout.clipToOutline = true
        crashTab.background = selectedDrawable
        logTab.background = unselectedDrawable

        crashTab.setOnClickListener { v ->
            v.background = selectedDrawable
            logTab.background = unselectedDrawable

            fragmentManager.beginTransaction()
                .replace(R.id.fragment_container, CrashFragment())
                .commit()
        }
        logTab.setOnClickListener { v ->
            v.background = selectedDrawable
            crashTab.background = unselectedDrawable

            fragmentManager.beginTransaction()
                .replace(R.id.fragment_container, LogFragment())
                .commit()
        }
    }
}
