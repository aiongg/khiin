package be.chiahpa.khiin.keyboard

import androidx.compose.foundation.gestures.detectDragGesturesAfterLongPress
import androidx.compose.foundation.gestures.detectTapGestures
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.material3.Surface
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.ui.unit.Dp
import androidx.compose.ui.unit.dp
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import be.chiahpa.khiin.keyboard.components.KeyPosition
import be.chiahpa.khiin.settings.Settings
import be.chiahpa.khiin.utils.loggerFor

private val log = loggerFor("KeyboardScreen")

@Composable
fun KeyboardScreen(
    viewModel: KeyboardViewModel
) {

    val rowHeightDp by Settings.rowHeightFlow.collectAsStateWithLifecycle(
        initialValue = 60f
    )

    val totalHeight = rowHeightDp * 5

    Box {
        Surface(
            Modifier
                .fillMaxWidth()
        ) {
            Column(Modifier.fillMaxWidth()) {
                CandidatesBar(viewModel = viewModel, height = rowHeightDp.dp)
                QwertyKeyboard(
                    viewModel = viewModel,
                    rowHeight = rowHeightDp.dp
                )
            }
        }

        KeyboardTouchDelegate(
            viewModel = viewModel,
            totalHeight = totalHeight.dp
        )
    }
}

@Composable
fun KeyboardTouchDelegate(viewModel: KeyboardViewModel, totalHeight: Dp) {
    val keyBounds by viewModel.keyBounds.collectAsStateWithLifecycle()
    var currentKey by remember { mutableStateOf(KeyData()) }
    var currentOffset by remember { mutableStateOf(Offset.Zero) }

    Box(
        modifier = Modifier
            .fillMaxWidth()
            .height(totalHeight)
            .pointerInput(Unit) {
                detectTapGestures(
                    onTap = {
                        keyBounds.keyAt(it)?.also { key ->
                            currentKey = key
                            log("Pressed key: ${key.label}")
                            viewModel.sendKey(key)
                        }
                    },
                )
            }
            .pointerInput(Unit) {
                detectDragGesturesAfterLongPress(
                    onDragStart = {
                        currentOffset = it

                        keyBounds.keyAt(it)?.also { key ->
                            currentKey = key
                            log("Long pressed key: ${key.label}")
                        }
                    },
                    onDrag = { _, dragAmount ->
                        currentOffset += dragAmount
                        keyBounds.keyAt(currentOffset)?.also { key ->
                            if (key != currentKey) {
                                currentKey = key
                                log("Dragged to key: ${key.label}")
                            }
                        }
                    }
                )
            }
    )
}

@Composable
fun QwertyKeyboard(viewModel: KeyboardViewModel, rowHeight: Dp) {
    KeyboardLayout(
        viewModel = viewModel
    ) {
        this.rowHeight = rowHeight

        row {
            alpha("q")
            alpha("w")
            alpha("e")
            alpha("r")
            alpha("t")
            alpha("y")
            alpha("u")
            alpha("i")
            alpha("o")
            alpha("p")
        }

        row {
            alpha("a", 1.5f, KeyPosition.ALIGN_RIGHT)
            alpha("s")
            alpha("d")
            alpha("f")
            alpha("g")
            alpha("h")
            alpha("j")
            alpha("k")
            alpha("l", 1.5f, KeyPosition.ALIGN_LEFT)
        }

        row {
            shift(1.5f)
            alpha("z")
            alpha("x")
            alpha("c")
            alpha("v")
            alpha("b")
            alpha("n")
            alpha("m")
            backspace(1.5f)
        }

        row {
            symbols(1.5f)
            alpha(",")
            spacebar(5f)
            alpha(".")
            enter(1.5f)
        }
    }
}
