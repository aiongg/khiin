package be.chiahpa.khiin.ui

import android.annotation.SuppressLint
import android.content.Context
import androidx.compose.runtime.Composable
import androidx.compose.ui.platform.AbstractComposeView
import be.chiahpa.khiin.ui.theme.KhiinTheme
import khiin.proto.Command

@SuppressLint("ViewConstructor")
class ComposeKeyboardView constructor(context: Context, val command: Command) :
    AbstractComposeView(context) {
    @Composable
    override fun Content() {
        KhiinTheme {
            KeyboardScreen(command = command)
        }
    }
}
