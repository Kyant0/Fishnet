@file:Suppress("DEPRECATION")

package com.kyant.fishnet.demo

import android.app.Activity
import android.os.Bundle
import android.widget.TabHost

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

        tabHost.setOnTabChangedListener { tabId ->
            val transaction = fragmentManager.beginTransaction()

            when (tabId) {
                "crash" -> transaction.replace(R.id.tab_content, CrashFragment())
                "log" -> transaction.replace(R.id.tab_content, LogFragment())
                "disasm" -> transaction.replace(R.id.tab_content, DisAsmFragment())
            }
            transaction.commit()
        }
    }
}
