package be.chiahpa.khiin.keyboard

import android.annotation.SuppressLint
import android.content.Context
import androidx.compose.runtime.Composable
import androidx.compose.ui.platform.AbstractComposeView
import androidx.lifecycle.viewModelScope
import androidx.lifecycle.viewmodel.compose.viewModel
import be.chiahpa.khiin.theme.KhiinTheme
import khiin.proto.Command
import kotlinx.coroutines.launch

@SuppressLint("ViewConstructor")
class ComposeKeyboardView constructor(context: Context, private val dbPath: String) :
    AbstractComposeView(context) {
    @Composable
    override fun Content() {
        val keyboardViewModel: KeyboardViewModel = viewModel(factory = viewModelFactory {
            KeyboardViewModel(dbPath)
        })

        KhiinTheme {
            KeyboardScreen(viewModel = keyboardViewModel)
        }
    }
}
