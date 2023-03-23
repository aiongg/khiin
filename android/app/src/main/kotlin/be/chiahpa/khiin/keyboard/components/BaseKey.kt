package be.chiahpa.khiin.keyboard.components

import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.RowScope
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxHeight
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.MaterialTheme
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.Dp
import androidx.compose.ui.unit.dp

enum class KeyPosition {
    FULL_WEIGHT,
    ALIGN_RIGHT,
    ALIGN_LEFT
}

@Composable
fun RowScope.BaseKey(
    weight: Float = 1f,
    keyColor: Color = Color.Transparent,
    keyPosition: KeyPosition = KeyPosition.FULL_WEIGHT,
    cornerSize: Dp = 12.dp,
    onClick: () -> Unit = {},
    content: @Composable () -> Unit
) {
    Box(
        contentAlignment = Alignment.Center,
        modifier = Modifier
            .weight(weight)
            .fillMaxHeight()
//            .border(BorderStroke(0.25.dp, Color.LightGray))
//            .background(backgroundColor)
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
        Row(
            modifier = Modifier.fillMaxSize()
        ) {
            val keyWeight = 1f / weight
            val spacerWeight = 1f - keyWeight

            if (weight != 1f && keyPosition == KeyPosition.ALIGN_RIGHT) {
                Spacer(modifier = Modifier.weight(spacerWeight))
            }

            Box(
                contentAlignment = Alignment.Center,
                modifier = Modifier
                    .fillMaxSize()
                    .weight(keyWeight)
                    .padding(4.dp, 8.dp)
                    .clip(RoundedCornerShape(cornerSize))
                    .background(keyColor)
            ) {
                content()
            }

            if (weight != 1f && keyPosition == KeyPosition.ALIGN_LEFT) {
                Spacer(modifier = Modifier.weight(spacerWeight))
            }
        }
    }
}
