package com.kyant.fishnet.demo

import android.content.Context
import android.os.Environment
import com.kyant.fishnet.Fishnet
import java.io.File

object LoggingConfig {
    lateinit var logPath: File
        private set
    private lateinit var savingLocationFile: File
    var savingLocation: SavingLocation = SavingLocation.Internal
        private set

    fun init(context: Context) {
        savingLocationFile = File(context.filesDir, "savingLocation").apply {
            if (createNewFile()) {
                writeBytes(byteArrayOf(0))
            }
        }
        savingLocation = SavingLocation.entries[savingLocationFile.readBytes().first().toInt()]
        changeSavingLocation(context, savingLocation)
    }

    fun getSavingPath(context: Context, savingLocation: SavingLocation): File {
        return when (savingLocation) {
            SavingLocation.Internal -> File(context.filesDir, "logs")
            SavingLocation.External -> File(context.getExternalFilesDir(null), "logs")
            SavingLocation.Downloads -> File(
                Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS),
                "Fishnet"
            )
        }
    }

    fun changeSavingLocation(context: Context, savingLocation: SavingLocation) {
        savingLocationFile = File(context.filesDir, "savingLocation").apply {
            createNewFile()
            writeBytes(byteArrayOf(savingLocation.ordinal.toByte()))
        }
        logPath = getSavingPath(context, savingLocation).apply { mkdirs() }
        Fishnet.init(context, logPath.absolutePath)
        this.savingLocation = savingLocation
    }

    fun getLogs(): List<File> {
        return try {
            logPath.listFiles()
                ?.filter { it.extension == "log" }
                ?.reversed()
                .orEmpty()
        } catch (_: Exception) {
            emptyList()
        }
    }

    enum class SavingLocation {
        Internal,
        External,
        Downloads,
    }
}
