package com.kyant.fishnet

class FishnetLogParser(log: String) {
    private val lines = log.lines()

    private var currentLine = 0

    fun parse(): FishnetLog {
        val header = parseHeader()
        skipBlankLine()
        val registers = parseRegisters()
        return FishnetLog(header, registers)
    }

    private fun parseHeader(): FishnetLog.Header {
        readLine("*** *** *** *** *** *** *** *** *** *** *** *** *** *** *** ***")
        val buildFingerprint = substringAfterOrEmpty("Build fingerprint: ")
        val revision = substringAfterOrEmpty("Revision: ")
        val abi = substringAfterOrEmpty("ABI: ")
        val timestamp = substringAfterOrEmpty("Timestamp: ")
        val processUpTime = substringAfterOrEmpty("Process uptime: ")
        val cmdline = substringAfterOrEmpty("Cmdline: ")
        val pidLine = substringAfterOrEmpty("pid: ")
        val (pid, tid, name) = pidLine.split(", ")
        val uid = substringAfterOrEmpty("uid: ")
        val taggedAddrCtrl = substringAfterOrEmpty("tagged_addr_ctrl: ")
        val pacEnabledKeys = substringAfterOrEmpty("pac_enabled_keys: ")
        val signalLine = substringAfterOrEmpty("signal ")
        val (signal, code, faultAddrString) = signalLine.split(", ")
        val faultAddr = faultAddrString.substringAfter("fault addr 0x")
        val cause = substringAfterOrEmpty("Cause: ")
        val abortMessage = substringAfterOrEmpty("Abort message: ")

        return FishnetLog.Header(
            buildFingerprint,
            revision,
            abi,
            timestamp,
            processUpTime,
            cmdline,
            pid,
            tid,
            name,
            uid,
            taggedAddrCtrl,
            pacEnabledKeys,
            signal,
            code,
            faultAddr.takeIf { it.isNotEmpty() }?.toULong(),
            cause,
            abortMessage,
        )
    }

    private fun parseRegisters(): List<FishnetLog.Register> {
        readLine("registers:")
        val registers = mutableListOf<FishnetLog.Register>()
        val regex = "\\s+".toRegex()
        while (currentLine < lines.size && !testBlankLine()) {
            val line = lines[currentLine++].trimStart().split(regex)
            check(line.size % 2 == 0)
            for (i in line.indices step 2) {
                registers.add(FishnetLog.Register(line[i], line[i + 1].toULong(16)))
            }
        }
        return registers
    }

    private fun testBlankLine(): Boolean {
        return lines[currentLine].isBlank()
    }

    private fun skipBlankLine() {
        check(lines[currentLine++].isBlank())
    }

    private fun readLine(line: String) {
        check(lines[currentLine++] == line)
    }

    private fun substringAfterOrEmpty(prefix: String): String {
        return if (lines[currentLine].startsWith(prefix)) {
            lines[currentLine++].substringAfter(prefix)
        } else {
            ""
        }
    }
}
