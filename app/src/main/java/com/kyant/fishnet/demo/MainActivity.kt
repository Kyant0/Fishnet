package com.kyant.fishnet.demo

import android.app.Activity
import android.graphics.Outline
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
        val disasmTab = findViewById<TextView>(R.id.tab_disasm)

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
        disasmTab.background = unselectedTabDrawable

        crashTab.setOnClickListener {
            crashTab.background = selectedTabDrawable
            logTab.background = unselectedTabDrawable
            disasmTab.background = unselectedTabDrawable

            fragmentManager.beginTransaction()
                .replace(R.id.fragment_container, CrashFragment())
                .commit()
        }
        logTab.setOnClickListener {
            crashTab.background = unselectedTabDrawable
            logTab.background = selectedTabDrawable
            disasmTab.background = unselectedTabDrawable

            fragmentManager.beginTransaction()
                .replace(R.id.fragment_container, LogFragment())
                .commit()
        }
        disasmTab.setOnClickListener {
            crashTab.background = unselectedTabDrawable
            logTab.background = unselectedTabDrawable
            disasmTab.background = selectedTabDrawable

            fragmentManager.beginTransaction()
                .replace(R.id.fragment_container, DisAsmFragment())
                .commit()
        }
    }
}
