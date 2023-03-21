package be.chiahpa.khiin

import khiin.proto.Command
import khiin.proto.command

const val KHIIN_ANDROID_NATIVE_LIBRARY = "khiindroid"

class EngineManager() {
    init {
        System.loadLibrary(KHIIN_ANDROID_NATIVE_LIBRARY)
    }

    constructor(dbFileName: String) : this() {
        this.dbFileName = dbFileName
        enginePtr = load(dbFileName)
    }

    fun sendCommand(cmd: Command): Command {
        val ret = sendCommand(enginePtr, cmd.toByteArray())
        val x = Command.parseFrom(ret)
        return x
    }

    fun shutdown() {
        shutdown(enginePtr)
    }

    private external fun load(dbFileName: String): Long

    private external fun sendCommand(enginePtr: Long, cmdBytes: ByteArray): ByteArray

    private external fun shutdown(enginePtr: Long)

    private lateinit var dbFileName: String

    private var enginePtr: Long = 0
}
