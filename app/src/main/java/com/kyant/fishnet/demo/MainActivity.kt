@file:Suppress("DEPRECATION", "OVERRIDE_DEPRECATION")

package com.kyant.fishnet.demo

import android.app.ActionBar
import android.app.Activity
import android.app.Fragment
import android.app.FragmentTransaction
import android.os.Bundle
import android.view.Menu
import android.view.MenuItem
import java.io.File

class MainActivity : Activity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        checkNotNull(actionBar).apply {
            navigationMode = ActionBar.NAVIGATION_MODE_TABS
            newTab().apply {
                text = "Test"
                setTabListener(TabListener(CrashingTestFragment()))
                addTab(this)
            }
            newTab().apply {
                text = "Logs"
                setTabListener(TabListener(LogsFragment()))
                addTab(this)
            }
            newTab().apply {
                text = "DisAsm"
                setTabListener(TabListener(DisAsmFragment()))
                addTab(this)
            }
        }
    }

    override fun onCreateOptionsMenu(menu: Menu?): Boolean {
        menuInflater.inflate(R.menu.main, menu)
        return true
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        return when (item.itemId) {
            R.id.item_clear_logs -> {
                try {
                    File(filesDir, "logs").apply {
                        deleteRecursively()
                        mkdirs()
                    }
                } catch (_: Exception) {
                }
                true
            }

            else -> super.onOptionsItemSelected(item)
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
