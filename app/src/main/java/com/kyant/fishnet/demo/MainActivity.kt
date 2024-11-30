@file:Suppress("DEPRECATION", "OVERRIDE_DEPRECATION")

package com.kyant.fishnet.demo

import android.app.ActionBar
import android.app.Activity
import android.app.Fragment
import android.app.FragmentTransaction
import android.os.Bundle

class MainActivity : Activity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        checkNotNull(actionBar).apply {
            navigationMode = ActionBar.NAVIGATION_MODE_TABS
            newTab().apply {
                text = "Crash"
                setTabListener(TabListener(CrashFragment()))
                addTab(this)
            }
            newTab().apply {
                text = "Log"
                setTabListener(TabListener(LogFragment()))
                addTab(this)
            }
            newTab().apply {
                text = "DisAsm"
                setTabListener(TabListener(DisAsmFragment()))
                addTab(this)
            }
        }
    }

    private inner class TabListener(private val fragment: Fragment) : ActionBar.TabListener {
        override fun onTabSelected(tab: ActionBar.Tab?, ft: FragmentTransaction?) {
            fragmentManager.beginTransaction()
                .replace(R.id.content, fragment)
                .commit()
        }

        override fun onTabUnselected(tab: ActionBar.Tab?, ft: FragmentTransaction?) {
        }

        override fun onTabReselected(tab: ActionBar.Tab?, ft: FragmentTransaction?) {
            fragmentManager.beginTransaction()
                .replace(R.id.content, fragment)
                .commit()
        }
    }
}
