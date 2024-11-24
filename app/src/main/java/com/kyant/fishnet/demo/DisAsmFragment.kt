@file:Suppress("DEPRECATION", "OVERRIDE_DEPRECATION")

package com.kyant.fishnet.demo

import android.app.Fragment
import android.os.Bundle
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
        val disasmEditText = view.findViewById<EditText>(R.id.et_disasm)
        val disasmButton = view.findViewById<View>(R.id.btn_disasm)
        val disasmTextView = view.findViewById<TextView>(R.id.tv_disasm)

        disasmButton.setOnClickListener {
            try {
                val input = disasmEditText.text.toString().replace("\\s".toRegex(), "")
                val bytes = buildList {
                    for (i in input.indices step 2) {
                        add((input[i].digitToInt(16) shl 4 or input[i + 1].digitToInt(16)).toByte())
                    }
                }.toByteArray()
                val disasm = DisAsm.disasm(bytes, 0)
                disasmTextView.text = disasm.joinToString("\n") { instruction ->
                    "0x${
                        instruction.address.toString(16).padStart(8, '0')
                    } [${instruction.bytes.toHexString()}] : ${instruction.mnemonic} ${instruction.operands}"
                }
            } catch (e: Exception) {
                disasmTextView.text = e.message
            }
        }
    }
}
