package be.chiahpa.khiin.keyboard

import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.Dp
import androidx.compose.ui.unit.TextUnit
import androidx.compose.ui.unit.sp
import be.chiahpa.khiin.R
import be.chiahpa.khiin.keyboard.components.IconKey
import be.chiahpa.khiin.keyboard.components.LetterKey
import be.chiahpa.khiin.keyboard.components.SymbolsKey
import be.chiahpa.khiin.utils.loggerFor

private val log = loggerFor("KeyboardLayout")

@Composable
fun KeyboardLayout(
    viewModel: KeyboardViewModel,
    rowHeight: Dp,
    fontSize: TextUnit = 28.sp,
    content: KeyboardLayoutScope.() -> Unit
) {
    val scopeContent = KeyboardLayoutScopeImpl().apply(content)

    scopeContent.rows.forEach { row ->
        Row(
            Modifier
                .fillMaxWidth()
                .height(rowHeight)
        ) {
            row.keys.forEach { key ->
                when (key.type) {
                    KeyType.LETTER -> key.label?.let {
                        LetterKey(
                            label = it,
                            fontSize = fontSize,
                            weight = key.weight,
                            onClick = {
                                viewModel.sendKey(key.label[0].code)
                            }
                        )
                    }

                    KeyType.SHIFT -> IconKey(
                        icon = R.drawable.shift,
                        weight = key.weight
                    )

                    KeyType.BACKSPACE -> IconKey(
                        icon = R.drawable.backspace,
                        weight = key.weight
                    )

                    KeyType.SYMBOLS -> SymbolsKey(
                        fontSize = fontSize,
                        weight = key.weight
                    )

                    KeyType.SPACEBAR -> LetterKey(
                        label = "",
                        fontSize = fontSize,
                        weight = key.weight
                    )

                    KeyType.ENTER -> IconKey(
                        icon = R.drawable.keyboard_return,
                        weight = key.weight
                    )
                }
            }
        }
    }
}
