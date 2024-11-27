package com.kyant.fishnet.demo

import android.app.Activity
import android.graphics.Outline
import android.graphics.drawable.Drawable
import android.os.Build
import android.os.Bundle
import android.view.View
import android.view.ViewOutlineProvider
import android.widget.LinearLayout
import android.widget.TextView

class MainActivity : Activity() {
    private lateinit var selectedTabDrawable: Drawable
    private var selectedTabColor: Int = 0
    private var unselectedTabColor: Int = 0

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

        selectedTabDrawable = resources.getDrawable(R.color.tab, theme)
        selectedTabColor = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            resources.getColor(R.color.tab_selected_text, theme)
        } else {
            resources.getColor(R.color.tab_selected_text)
        }
        unselectedTabColor = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            resources.getColor(R.color.text, theme)
        } else {
            resources.getColor(R.color.text)
        }

        val cornerRadiusPx = 12 * resources.displayMetrics.density
        tabLayout.outlineProvider = object : ViewOutlineProvider() {
            override fun getOutline(view: View, outline: Outline) {
                outline.setRoundRect(0, 0, view.width, view.height, cornerRadiusPx)
            }
        }
        tabLayout.clipToOutline = true

        crashTab.select()
        logTab.unselect()
        disasmTab.unselect()

        crashTab.setOnClickListener {
            crashTab.select()
            logTab.unselect()
            disasmTab.unselect()

            fragmentManager.beginTransaction()
                .replace(R.id.fragment_container, CrashFragment())
                .commit()
        }
        logTab.setOnClickListener {
            crashTab.unselect()
            logTab.select()
            disasmTab.unselect()

            fragmentManager.beginTransaction()
                .replace(R.id.fragment_container, LogFragment())
                .commit()
        }
        disasmTab.setOnClickListener {
            crashTab.unselect()
            logTab.unselect()
            disasmTab.select()

            fragmentManager.beginTransaction()
                .replace(R.id.fragment_container, DisAsmFragment())
                .commit()
        }
    }

    private fun TextView.select() {
        setTextColor(selectedTabColor)
        background = selectedTabDrawable
    }

    private fun TextView.unselect() {
        setTextColor(unselectedTabColor)
        background = null
    }
}
