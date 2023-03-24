package be.chiahpa.khiin.keyboard

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.material3.MaterialTheme
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.Dp
import androidx.compose.ui.unit.TextUnit
import androidx.compose.ui.unit.dp
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
    fontSize: TextUnit = 28.sp,
    content: KeyboardLayoutScope.() -> Unit
) {
    val scopeData = KeyboardLayoutScopeImpl().apply(content)
    val keyboardRows = scopeData.toImmutable()
    val rowHeight = scopeData.rowHeight

    val theme = KeyboardTheme(
        background = Color(0xFFEEEEEE),
        key = Color.White,
        label = Color.DarkGray,
        accentKey = MaterialTheme.colorScheme.primary.copy(0.2f),
        accentLabel = Color.DarkGray,
        actionKey = MaterialTheme.colorScheme.primary.copy(0.5f),
        actionLabel = Color.DarkGray,
    )

    val theme2 = KeyboardTheme(
        background = Color(0xff00363d),
        key = Color(0xff004f58),
        label = Color(0xffcde7ec),
        accentKey = Color(0xffb1cbd0),
        accentLabel = Color(0xff00363d),
        actionKey = Color(0xff4fd8ec),
        actionLabel = Color(0xff00363d),
    )

    keyboardRows.items.forEach { row ->
        Row(
            Modifier
                .fillMaxWidth()
                .height(rowHeight)
                .background(theme.background)
        ) {
            row.items.forEach { key ->
                when (key.type) {
                    KeyType.LETTER -> key.label?.let { label ->
                        LetterKey(
                            label = label,
                            fontSize = fontSize,
                            textColor = theme.label,
                            keyColor = theme.key,
                            weight = key.weight,
                            keyPosition = key.position,
                            onLayout = { viewModel.setKeyBounds(key, it) }
                        )
                    }

                    KeyType.SHIFT -> IconKey(
                        icon = R.drawable.shift,
                        weight = key.weight,
                        keyColor = theme.accentKey,
                        tint = theme.accentLabel
                    )

                    KeyType.BACKSPACE -> IconKey(
                        icon = R.drawable.backspace,
                        weight = key.weight,
                        keyColor = theme.accentKey,
                        tint = theme.accentLabel
                    )

                    KeyType.SYMBOLS -> SymbolsKey(
                        fontSize = fontSize,
                        weight = key.weight,
                        keyColor = theme.accentKey,
                        textColor = theme.accentLabel,
                        cornerSize = 24.dp
                    )

                    KeyType.SPACEBAR -> LetterKey(
                        label = "",
                        fontSize = fontSize,
                        weight = key.weight,
                        keyColor = theme.key
                    )

                    KeyType.ENTER -> IconKey(
                        icon = R.drawable.keyboard_return,
                        weight = key.weight,
                        keyColor = theme.actionKey,
                        tint = theme.actionLabel,
                        cornerSize = 24.dp
                    )
                }
            }
        }
    }
}
