package be.chiahpa.khiin

import khiin.proto.Command
import khiin.proto.Request
import khiin.proto.Response
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

    fun sendCommand(req: Request): Command {
        val res = sendCommand(enginePtr, req.toByteArray())
        return command {
            request = req
            response = Response.parseFrom(res)
        }
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
