package be.chiahpa.khiin.keyboard.components

import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.RowScope
import androidx.compose.foundation.layout.fillMaxHeight
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.drawWithContent
import androidx.compose.ui.unit.TextUnit
import androidx.compose.ui.unit.sp

@Composable
fun RowScope.SymbolsKey(weight: Float = 1f, fontSize: TextUnit = 28.sp) {
    val baseTextStyle = MaterialTheme.typography.bodySmall.copy(fontSize = fontSize, letterSpacing = 0.5.sp)
    var textStyle by remember { mutableStateOf(baseTextStyle) }
    var ready by remember { mutableStateOf(false) }

    Box(
        contentAlignment = Alignment.Center,
        modifier = Modifier
            .weight(weight)
            .fillMaxHeight()
    ) {
        Text(
            text = "?123",
            modifier = Modifier.drawWithContent {
                if (ready) drawContent()
            },
            maxLines = 1,
            softWrap = false,
            onTextLayout = {
                if (it.didOverflowWidth) {
                    textStyle =
                        textStyle.copy(fontSize = textStyle.fontSize * 0.9f)
                } else {
                    ready = true
                }
            },
            style = textStyle
        )
    }
}
