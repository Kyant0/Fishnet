@file:Suppress("DEPRECATION", "OVERRIDE_DEPRECATION")

package com.kyant.fishnet.demo

import android.app.Fragment
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.compose.foundation.horizontalScroll
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.ComposeView
import androidx.compose.ui.text.font.Font
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.unit.dp
import com.kyant.fishnet.demo.ui.FishnetTheme
import java.io.File

class RawLogFragment : Fragment() {
    val jetbrainsMono = FontFamily(Font(R.font.jetbrains_mono))

    override fun onCreateView(inflater: LayoutInflater?, container: ViewGroup?, savedInstanceState: Bundle?): View? {
        val log = getLog()
        return ComposeView(context).apply {
            setContent {
                FishnetTheme {
                    Column(
                        Modifier
                            .horizontalScroll(rememberScrollState())
                            .verticalScroll(rememberScrollState())
                            .padding(8.dp)
                    ) {
                        Text(
                            log,
                            fontFamily = jetbrainsMono,
                            style = MaterialTheme.typography.bodySmall
                        )
                    }
                }
            }
        }
    }

    private fun getLog(): String {
        return try {
            val logFile = File(context.filesDir, "fishnet.log")
            logFile.readText()
        } catch (e: Exception) {
            e.printStackTrace()
            "Failed to read log file: ${e.message}"
        }
    }
}
