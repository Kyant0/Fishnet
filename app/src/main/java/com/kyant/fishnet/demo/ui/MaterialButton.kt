package com.kyant.fishnet.demo.ui

import android.content.Context
import android.graphics.Outline
import android.graphics.Typeface
import android.os.Build
import android.util.AttributeSet
import android.util.TypedValue
import android.view.Gravity
import android.view.View
import android.view.ViewOutlineProvider
import android.widget.TextView
import com.kyant.fishnet.demo.R
import kotlin.math.roundToInt

class MaterialButton @JvmOverloads constructor(
    context: Context, attrs: AttributeSet? = null
) : TextView(context, attrs) {
    init {
        background = resources.getDrawable(R.color.button, context.theme)
        val value = TypedValue()
        context.theme.resolveAttribute(android.R.attr.selectableItemBackground, value, true)
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            foreground = resources.getDrawable(value.resourceId, context.theme)
        }
        val horizontalPadding = (16 * resources.displayMetrics.density).roundToInt()
        val verticalPadding = (10 * resources.displayMetrics.density).roundToInt()
        setPadding(horizontalPadding, verticalPadding, horizontalPadding, verticalPadding)
        gravity = Gravity.CENTER
        context.theme.resolveAttribute(android.R.attr.textColorPrimary, value, true)
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            setTextColor(resources.getColor(value.resourceId, context.theme))
        } else {
            setTextColor(resources.getColor(value.resourceId))
        }
        textSize = 15f
        typeface = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            Typeface.create(Typeface.DEFAULT_BOLD, 500, false)
        } else {
            Typeface.DEFAULT_BOLD
        }
        val cornerRadiusPx = 12 * resources.displayMetrics.density
        outlineProvider = object : ViewOutlineProvider() {
            override fun getOutline(view: View, outline: Outline) {
                outline.setRoundRect(0, 0, view.width, view.height, cornerRadiusPx)
            }
        }
        clipToOutline = true
    }
}
