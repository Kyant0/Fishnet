@file:Suppress("DEPRECATION", "OVERRIDE_DEPRECATION")

package com.kyant.fishnet.demo

import android.app.Fragment
import android.os.Bundle
import android.text.SpannableStringBuilder
import android.text.Spanned
import android.text.style.ForegroundColorSpan
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.EditText
import android.widget.TextView
import com.kyant.disasm.DisAsm

@OptIn(ExperimentalStdlibApi::class)
class DisAsmFragment : Fragment() {
    override fun onCreateView(inflater: LayoutInflater?, container: ViewGroup?, savedInstanceState: Bundle?): View? {
        return inflater?.inflate(R.layout.fragment_disasm, container, false)
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        val startAddressEditText = view.findViewById<EditText>(R.id.et_address)
        val disasmEditText = view.findViewById<EditText>(R.id.et_disasm)
        val disasmButton = view.findViewById<View>(R.id.btn_disasm)
        val disasmTextView = view.findViewById<TextView>(R.id.tv_disasm)

        disasmButton.setOnClickListener {
            val regex = "\\s".toRegex()
            val result = try {
                val address = startAddressEditText.text.toString()
                    .replace(regex, "")
                    .removePrefix("0x")
                    .ifEmpty { "0" }
                    .toLong(16)
                val input = disasmEditText.text.toString()
                    .replace(regex, "")
                val bytes = buildList {
                    val endIndex = if (input.length % 2 == 0) input.lastIndex else input.lastIndex - 1
                    for (i in 0..endIndex step 2) {
                        add((input[i].digitToInt(16) shl 4 or input[i + 1].digitToInt(16)).toByte())
                    }
                }.toByteArray()
                val disasm = DisAsm.disasm(bytes, address)
                SpannableStringBuilder().apply {
                    val lowEmphasizedColor = disasmTextView.textColors.defaultColor and 0x00FFFFFF or 0x80000000.toInt()
                    val highlightedColor = context.resources.getColor(R.color.primary, context.theme)
                    var start: Int
                    val callMnemonics = setOf(
                        // ARM64
                        "bl", "blr",
                        // ARM32
                        "blx", "bl",
                        // x86/x64
                        "call", "lcall"
                    )
                    val maxByteLength = disasm.maxOf { it.bytes.size }
                    disasm.forEach { instruction ->
                        append("0x${instruction.address.toString(16).padStart(16, '0')} ")
                        start = length
                        append(instruction.bytes.toHexString().padEnd(maxByteLength * 2, ' '))
                        setSpan(
                            ForegroundColorSpan(lowEmphasizedColor),
                            start,
                            length,
                            Spanned.SPAN_EXCLUSIVE_EXCLUSIVE
                        )
                        if (instruction.mnemonic in callMnemonics) {
                            append(" : ${instruction.mnemonic.padEnd(8, ' ')} ")
                            start = length
                            append("${instruction.operands}\n")
                            setSpan(
                                ForegroundColorSpan(highlightedColor),
                                start,
                                length,
                                Spanned.SPAN_EXCLUSIVE_EXCLUSIVE
                            )
                        } else {
                            append(" : ${instruction.mnemonic.padEnd(8, ' ')} ${instruction.operands}\n")
                        }
                    }
                    if (getOrNull(length - 1) == '\n') {
                        delete(length - 1, length)
                    }
                }
            } catch (e: Exception) {
                e.message
            }
            disasmTextView.text = result
        }
    }
}
