package be.chiahpa.khiin.ui

import android.content.Context
import androidx.compose.runtime.Composable
import androidx.compose.ui.platform.AbstractComposeView
import be.chiahpa.khiin.ui.theme.KhiinTheme

class ComposeKeyboardView constructor(context: Context) :
    AbstractComposeView(context) {
    @Composable
    override fun Content() {
        KhiinTheme {
            KeyboardScreen()
        }
    }
}
