@file:Suppress("DEPRECATION", "OVERRIDE_DEPRECATION")

package com.kyant.fishnet.demo

import android.Manifest
import android.app.Fragment
import android.content.Intent
import android.content.pm.PackageManager
import android.net.Uri
import android.os.Build
import android.os.Bundle
import android.os.Environment
import android.provider.Settings
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Switch

class SettingsFragment : Fragment() {
    override fun onCreateView(inflater: LayoutInflater?, container: ViewGroup?, savedInstanceState: Bundle?): View? {
        return inflater?.inflate(R.layout.fragment_settings, container, false)
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        val context = view.context

        val internalStorageSwitch = view.findViewById<Switch>(R.id.switch_internal_storage)
        val externalStorageSwitch = view.findViewById<Switch>(R.id.switch_external_storage)
        val downloadsSwitch = view.findViewById<Switch>(R.id.switch_downloads)

        internalStorageSwitch.isChecked = LoggingConfig.savingLocation == LoggingConfig.SavingLocation.Internal
        externalStorageSwitch.isChecked = LoggingConfig.savingLocation == LoggingConfig.SavingLocation.External
        downloadsSwitch.isChecked = LoggingConfig.savingLocation == LoggingConfig.SavingLocation.Downloads

        internalStorageSwitch.setOnClickListener {
            LoggingConfig.changeSavingLocation(context, LoggingConfig.SavingLocation.Internal)
            internalStorageSwitch.isChecked = true
            externalStorageSwitch.isChecked = false
            downloadsSwitch.isChecked = false
        }
        externalStorageSwitch.setOnClickListener {
            LoggingConfig.changeSavingLocation(context, LoggingConfig.SavingLocation.External)
            internalStorageSwitch.isChecked = false
            externalStorageSwitch.isChecked = true
            downloadsSwitch.isChecked = false
        }
        downloadsSwitch.setOnClickListener {
            LoggingConfig.changeSavingLocation(context, LoggingConfig.SavingLocation.Downloads)
            internalStorageSwitch.isChecked = false
            externalStorageSwitch.isChecked = false
            downloadsSwitch.isChecked = true
            checkPermissions()
        }

        view.findViewById<View>(R.id.btn_delete_all_logs).setOnClickListener {
            LoggingConfig.SavingLocation.entries.forEach {
                try {
                    LoggingConfig.getSavingPath(context, it).apply {
                        deleteRecursively()
                        mkdirs()
                    }
                } catch (_: Exception) {
                }
            }
        }
    }

    private fun checkPermissions() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            if (!Environment.isExternalStorageManager()) {
                try {
                    startActivity(
                        Intent(
                            Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION,
                            Uri.parse("package:${context.packageName}")
                        )
                    )
                } catch (_: Exception) {
                }
            }
        } else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            val permission = Manifest.permission.WRITE_EXTERNAL_STORAGE
            if (context.checkSelfPermission(permission) != PackageManager.PERMISSION_GRANTED) {
                requestPermissions(arrayOf(permission), 0)
            }
        }
    }
}
