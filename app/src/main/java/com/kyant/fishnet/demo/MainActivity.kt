@file:Suppress("DEPRECATION")

package com.kyant.fishnet.demo

import android.app.Activity
import android.os.Bundle
import android.widget.TabHost
import android.widget.TabWidget

class MainActivity : Activity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        val tabHost = findViewById<TabHost>(R.id.tab_host)
        tabHost.setup()

        tabHost.addTab(
            tabHost
                .newTabSpec("crash")
                .setIndicator("Crash")
                .setContent(android.R.id.tabcontent)
        )
        tabHost.addTab(
            tabHost
                .newTabSpec("log")
                .setIndicator("Log")
                .setContent(android.R.id.tabcontent)
        )
        tabHost.addTab(
            tabHost
                .newTabSpec("disasm")
                .setIndicator("DisAsm")
                .setContent(android.R.id.tabcontent)
        )

        fragmentManager.beginTransaction()
            .replace(R.id.tab_content, CrashFragment())
            .commit()

        findViewById<TabWidget>(android.R.id.tabs).apply {
            getChildAt(0).setOnClickListener {
                tabHost.currentTab = 0
                fragmentManager.beginTransaction()
                    .replace(R.id.tab_content, CrashFragment())
                    .commit()
            }
            getChildAt(1).setOnClickListener {
                tabHost.currentTab = 1
                fragmentManager.beginTransaction()
                    .replace(R.id.tab_content, LogFragment())
                    .commit()
            }
            getChildAt(2).setOnClickListener {
                tabHost.currentTab = 2
                fragmentManager.beginTransaction()
                    .replace(R.id.tab_content, DisAsmFragment())
                    .commit()
            }
        }
    }
}
