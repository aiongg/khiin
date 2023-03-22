package be.chiahpa.khiin

import android.inputmethodservice.InputMethodService
import android.view.View
import android.view.inputmethod.EditorInfo
import be.chiahpa.khiin.service.KeyboardViewLifecycleOwner
import be.chiahpa.khiin.ui.ComposeKeyboardView

class KhiinService : InputMethodService() {
    private val keyboardViewLifecycleOwner = KeyboardViewLifecycleOwner()

    override fun onCreate() {
        super.onCreate()
        keyboardViewLifecycleOwner.onCreate()
    }

    override fun onCreateInputView(): View {
        keyboardViewLifecycleOwner.attachToDecorView(
            window?.window?.decorView
        )

        return ComposeKeyboardView(this)
    }

    override fun onStartInputView(info: EditorInfo?, restarting: Boolean) {
        keyboardViewLifecycleOwner.onResume()
    }

    override fun onFinishInputView(finishingInput: Boolean) {
        keyboardViewLifecycleOwner.onPause()
    }

    override fun onDestroy() {
        super.onDestroy()
        keyboardViewLifecycleOwner.onDestroy()
    }
}
