package be.chiahpa.khiin

import android.inputmethodservice.InputMethodService
import android.view.View
import android.view.inputmethod.EditorInfo
import be.chiahpa.khiin.keyboard.KeyboardViewModel
import be.chiahpa.khiin.service.KhiinServiceLifecycleOwner
import be.chiahpa.khiin.service.copyAssetToFiles
import be.chiahpa.khiin.ui.ComposeKeyboardView
import khiin.proto.CommandType
import khiin.proto.keyEvent
import khiin.proto.request
import java.io.File

class KhiinService : InputMethodService() {
    private val lifecycleOwner = KhiinServiceLifecycleOwner()
    private lateinit var engineManager: EngineManager
    private lateinit var keyboardViewModel: KeyboardViewModel

    override fun onCreate() {
        super.onCreate()
        lifecycleOwner.onCreate()

        copyAssetToFiles(this, "khiin.db")
        engineManager = EngineManager(File(filesDir, "khiin.db").absolutePath)
    }

    override fun onCreateInputView(): View {
        lifecycleOwner.attachToDecorView(
            window?.window?.decorView
        )

        val cmd = request {
            type = CommandType.CMD_SEND_KEY
            keyEvent = keyEvent {
                keyCode = 97
            }
        }

        val res = engineManager.sendCommand(cmd)

        return ComposeKeyboardView(this, res)
    }

    override fun onStartInputView(info: EditorInfo?, restarting: Boolean) {
        lifecycleOwner.onResume()
    }

    override fun onFinishInputView(finishingInput: Boolean) {
        lifecycleOwner.onPause()
    }

    override fun onDestroy() {
        super.onDestroy()
        lifecycleOwner.onDestroy()
        engineManager.shutdown()
    }
}
