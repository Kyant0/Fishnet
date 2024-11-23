@file:Suppress("DEPRECATION", "OVERRIDE_DEPRECATION")

package com.kyant.fishnet.demo

import android.app.Fragment
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.RowScope
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.layout.wrapContentSize
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.platform.ComposeView
import androidx.compose.ui.text.font.Font
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.unit.dp
import com.kyant.fishnet.FishnetLog
import com.kyant.fishnet.FishnetLogParser
import com.kyant.fishnet.demo.ui.FishnetTheme
import java.io.File

class LogFragment : Fragment() {
    private var logParser: FishnetLogParser? = null

    val jetbrainsMono = FontFamily(Font(R.font.jetbrains_mono))

    override fun onCreateView(inflater: LayoutInflater?, container: ViewGroup?, savedInstanceState: Bundle?): View {
        logParser = getFishnetLogParser()
        val log = logParser?.parse()
        return ComposeView(context).apply {
            log?.apply {
                setContent {
                    FishnetTheme {
                        Column {
                            Text("Registers:")

                            Surface(
                                Modifier.padding(8.dp),
                                shape = RoundedCornerShape(24.dp),
                                color = MaterialTheme.colorScheme.surfaceContainerLowest
                            ) {
                                Column(Modifier.padding(8.dp)) {
                                    for (i in registers.indices step 2) {
                                        val register1 = registers[i]
                                        val register2 = registers.getOrNull(i + 1)

                                        Row {
                                            Register(register1)
                                            if (register2 != null) {
                                                Register(register2)
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    @Composable
    fun RowScope.Register(register: FishnetLog.Register) {
        Row(
            Modifier
                .weight(1f)
                .height(24.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            Surface(
                Modifier.size(32.dp, 20.dp),
                shape = CircleShape,
                color = MaterialTheme.colorScheme.primaryContainer
            ) {
                Text(
                    register.name,
                    Modifier.wrapContentSize(Alignment.Center),
                    style = MaterialTheme.typography.labelMedium
                )
            }
            Spacer(Modifier.width(8.dp))
            val alpha = if (register.value != 0UL) 1f else 0.38f
            Text(
                "%08X".format((register.value shr 32).toInt()),
                Modifier.alpha(alpha),
                style = MaterialTheme.typography.labelMedium.copy(fontFamily = jetbrainsMono)
            )
            Spacer(Modifier.width(4.dp))
            Text(
                "%08X".format(register.value.toInt()),
                Modifier.alpha(alpha),
                style = MaterialTheme.typography.labelMedium.copy(fontFamily = jetbrainsMono)
            )
        }
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        logParser = getFishnetLogParser()

        logParser?.parse()?.apply {
            println(this)
        }
    }

    private fun getFishnetLogParser(): FishnetLogParser? {
        return try {
            val logFile = File(context.filesDir, "fishnet.log")
            FishnetLogParser(logFile.readText())
        } catch (e: Exception) {
            e.printStackTrace()
            null
        }
    }
}
