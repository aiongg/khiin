package be.chiahpa.khiin.keyboard

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import androidx.lifecycle.viewmodel.compose.viewModel
import be.chiahpa.khiin.settings.Settings
import khiin.proto.Command

@Composable
fun KeyboardScreen(
    viewModel: KeyboardViewModel
) {
    val uiState by viewModel.uiState.collectAsStateWithLifecycle()

    val rowHeightDp = Settings.rowHeightFlow.collectAsStateWithLifecycle(
        initialValue = 60f
    )
    val rowHeight = rowHeightDp.value.dp

    Surface(
        Modifier
            .fillMaxWidth()
    ) {
        Column(Modifier.fillMaxWidth()) {
            CandidatesBar(height = rowHeight)

            Column(Modifier.fillMaxWidth()) {
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.SpaceAround
                ) {
                    uiState.candidates.forEach {
                        Text(it, fontSize = 28.sp)
                    }
                }
            }

            KeyboardLayout(
                viewModel = viewModel,
                rowHeight = rowHeight
            ) {
                row {
                    key("q")
                    key("w")
                    key("e")
                    key("r")
                    key("t")
                    key("y")
                    key("u")
                    key("i")
                    key("o")
                    key("p")
                }

                row {
                    key("a", 1.5f)
                    key("s")
                    key("d")
                    key("f")
                    key("g")
                    key("h")
                    key("j")
                    key("k")
                    key("l", 1.5f)
                }

                row {
                    shift(1.5f)
                    key("z")
                    key("x")
                    key("c")
                    key("v")
                    key("b")
                    key("n")
                    key("m")
                    backspace(1.5f)
                }

                row {
                    symbols(1.5f)
                    key(",")
                    spacebar(5f)
                    key(".")
                    enter(1.5f)
                }
            }
        }
    }
}
