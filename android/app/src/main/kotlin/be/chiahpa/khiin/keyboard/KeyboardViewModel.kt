package be.chiahpa.khiin.keyboard

import androidx.compose.runtime.MutableState
import androidx.compose.ui.unit.Dp
import androidx.compose.ui.unit.TextUnit
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.lifecycle.ViewModel
import androidx.lifecycle.ViewModelProvider
import androidx.lifecycle.viewModelScope
import be.chiahpa.khiin.EngineManager
import be.chiahpa.khiin.settings.Settings
import khiin.proto.Command
import khiin.proto.CommandType
import khiin.proto.keyEvent
import khiin.proto.request
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.launch

inline fun <VM : ViewModel> viewModelFactory(crossinline f: () -> VM) =
    object : ViewModelProvider.Factory {
        override fun <T : ViewModel> create(modelClass: Class<T>): T = f() as T
    }

data class KeyboardUiState(
    val candidates: List<String> = listOf()
)

class KeyboardViewModel(dbPath: String) : ViewModel() {
    private val engineManager: EngineManager = EngineManager(dbPath)

    private val _uiState = MutableStateFlow(KeyboardUiState())
    val uiState: StateFlow<KeyboardUiState> = _uiState.asStateFlow()

    fun sendKey(key: Int) {
        viewModelScope.launch {
            val req = request {
                type = CommandType.CMD_SEND_KEY
                keyEvent = keyEvent {
                    keyCode = key
                }
            }

            val res = engineManager.sendCommand(req)

            _uiState.value = uiState.value.copy(
                candidates =
                res.response.candidateList.candidatesList.map {
                    it.value
                })
        }
    }

    override fun onCleared() {
        super.onCleared()
        engineManager.shutdown()
    }
}
