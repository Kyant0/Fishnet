package com.kyant.fishnet.demo.ui

import android.content.Context
import android.graphics.Typeface
import android.os.Build
import android.util.AttributeSet
import android.widget.TextView
import com.kyant.fishnet.demo.R

class MonospacedText @JvmOverloads constructor(
    context: Context, attrs: AttributeSet? = null
) : TextView(context, attrs) {
    init {
        typeface = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            resources.getFont(R.font.jetbrains_mono)
        } else {
            Typeface.MONOSPACE
        }
    }
}
