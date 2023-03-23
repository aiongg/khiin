package be.chiahpa.khiin.keyboard.components

import android.view.MotionEvent
import androidx.compose.foundation.BorderStroke
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.RowScope
import androidx.compose.foundation.layout.fillMaxHeight
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.ExperimentalComposeUiApi
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.input.pointer.pointerInteropFilter
import androidx.compose.ui.unit.TextUnit
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import be.chiahpa.khiin.utils.loggerFor

private val log = loggerFor("LetterKey")

@OptIn(ExperimentalComposeUiApi::class)
@Composable
fun RowScope.LetterKey(
    label: String,
    modifier: Modifier = Modifier,
    weight: Float = 1f,
    fontSize: TextUnit = 28.sp,
    onClick: () -> Unit = {}
) {
    Box(
        contentAlignment = Alignment.Center,
        modifier = modifier
            .weight(weight)
            .fillMaxHeight()
            .border(BorderStroke(1.dp, Color.Red))
            .clickable { onClick() }
//            .pointerInteropFilter {
//                when (it.action) {
//                    MotionEvent.ACTION_MOVE -> {
//                        log("Moved to: ${it.rawX.toString()}, ${it.rawY.toString()}")
//                    }
//                }
//                true
//            }
    ) {
        Text(
            text = label,
            fontSize = fontSize,
        )
    }
}