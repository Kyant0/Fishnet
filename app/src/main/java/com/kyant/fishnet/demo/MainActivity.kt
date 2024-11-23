package com.kyant.fishnet.demo

import android.graphics.Outline
import android.os.Bundle
import android.view.View
import android.view.ViewOutlineProvider
import android.widget.LinearLayout
import android.widget.TextView
import androidx.activity.ComponentActivity

class MainActivity : ComponentActivity() {
    @Suppress("DEPRECATION")
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        fragmentManager.beginTransaction()
            .replace(R.id.fragment_container, CrashFragment())
            .commit()

        val tabLayout = findViewById<LinearLayout>(R.id.tab_layout)
        val crashTab = findViewById<TextView>(R.id.tab_crash)
        val rawLogTab = findViewById<TextView>(R.id.tab_raw_log)
        val logTab = findViewById<TextView>(R.id.tab_log)

        val selectedTabDrawable = resources.getDrawable(R.color.tab, theme)
        val unselectedTabDrawable = resources.getDrawable(R.color.tab_container, theme)

        val cornerRadiusPx = 12 * resources.displayMetrics.density
        tabLayout.outlineProvider = object : ViewOutlineProvider() {
            override fun getOutline(view: View, outline: Outline) {
                outline.setRoundRect(0, 0, view.width, view.height, cornerRadiusPx)
            }
        }
        tabLayout.clipToOutline = true
        crashTab.background = selectedTabDrawable
        logTab.background = unselectedTabDrawable

        crashTab.setOnClickListener { v ->
            v.background = selectedTabDrawable
            rawLogTab.background = unselectedTabDrawable
            logTab.background = unselectedTabDrawable

            fragmentManager.beginTransaction()
                .replace(R.id.fragment_container, CrashFragment())
                .commit()
        }
        rawLogTab.setOnClickListener { v ->
            v.background = selectedTabDrawable
            crashTab.background = unselectedTabDrawable
            logTab.background = unselectedTabDrawable

            fragmentManager.beginTransaction()
                .replace(R.id.fragment_container, RawLogFragment())
                .commit()
        }
        logTab.setOnClickListener { v ->
            v.background = selectedTabDrawable
            rawLogTab.background = unselectedTabDrawable
            crashTab.background = unselectedTabDrawable

            fragmentManager.beginTransaction()
                .replace(R.id.fragment_container, LogFragment())
                .commit()
        }
    }
}
