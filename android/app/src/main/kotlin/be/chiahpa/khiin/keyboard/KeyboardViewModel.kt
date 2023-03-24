package be.chiahpa.khiin.keyboard

import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.geometry.Rect
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import be.chiahpa.khiin.EngineManager
import be.chiahpa.khiin.utils.loggerFor
import khiin.proto.CandidateList
import khiin.proto.CommandType
import khiin.proto.Request
import khiin.proto.keyEvent
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.launch

private val log = loggerFor("KeyboardViewModel")

sealed class CandidateState {
    object Empty : CandidateState()
    class Loaded(val candidates: CandidateList) : CandidateState()
}

typealias KeyCoordinateMap = Map<KeyData, Rect>

internal fun KeyCoordinateMap.keyAt(offset: Offset): KeyData? {
    this.forEach { (key, rect) ->
        if (rect.contains(offset)) {
            return key
        }
    }

    return null
}

class KeyboardViewModel(dbPath: String) : ViewModel() {
    init {
        EngineManager.startup(dbPath)
    }

    override fun onCleared() {
        super.onCleared()
        EngineManager.shutdown()
    }

    private val _candidateState =
        MutableStateFlow<CandidateState>(CandidateState.Empty)

    val candidateState: StateFlow<CandidateState> =
        _candidateState.asStateFlow()

    private val _keyBounds =
        MutableStateFlow<KeyCoordinateMap>(mapOf())

    val keyBounds: StateFlow<KeyCoordinateMap> =
        _keyBounds.asStateFlow()

    fun sendKey(key: KeyData) {
        val req = Request.newBuilder()

        if (key.type == KeyType.LETTER && !key.label.isNullOrEmpty()) {
            req.apply {
                type = CommandType.CMD_SEND_KEY
                keyEvent = keyEvent {
                    keyCode = key.label[0].code
                }
            }
        }

        if (req.type != CommandType.CMD_UNSPECIFIED) {
            viewModelScope.launch {
                val res = EngineManager.sendCommand(req.build())
                if (res.response.candidateList.candidatesList.isNotEmpty()) {
                    _candidateState.value = CandidateState.Loaded(res.response.candidateList)
                }
            }
        }
    }

    fun setKeyBounds(
        keyData: KeyData,
        bounds: Rect
    ) {
        val next = keyBounds.value.toMutableMap()
        next[keyData] = bounds
        _keyBounds.value = next
    }
}
