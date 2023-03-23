package be.chiahpa.khiin.keyboard

import androidx.compose.ui.unit.Dp
import androidx.compose.ui.unit.TextUnit
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import be.chiahpa.khiin.settings.Settings
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.launch

data class KeyboardUiState(
    val fontSize: TextUnit = 28.sp,
    val rowHeight: Dp = 60.dp
)

class KeyboardViewModel : ViewModel() {
    private val _uiState = MutableStateFlow(KeyboardUiState())

    val uiState: StateFlow<KeyboardUiState> = _uiState.asStateFlow()

    fun changeRowHeight(value: Float) {
        viewModelScope.launch {
            Settings.setRowHeight(value)
        }
    }
}
