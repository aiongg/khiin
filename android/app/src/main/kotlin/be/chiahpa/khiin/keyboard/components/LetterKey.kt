package be.chiahpa.khiin.keyboard.components

import android.view.MotionEvent
import androidx.compose.foundation.BorderStroke
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.RowScope
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxHeight
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.ExperimentalComposeUiApi
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.input.pointer.pointerInteropFilter
import androidx.compose.ui.unit.TextUnit
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import be.chiahpa.khiin.utils.loggerFor

private val log = loggerFor("LetterKey")

@Composable
fun RowScope.LetterKey(
    label: String,
    weight: Float = 1f,
    fontSize: TextUnit = 28.sp,
    textColor: Color = MaterialTheme.colorScheme.onSurface,
    keyColor: Color = Color.Transparent,
    keyPosition: KeyPosition = KeyPosition.FULL_WEIGHT,
    onClick: () -> Unit = {}
) {
    BaseKey(
        weight = weight,
        keyColor = keyColor,
        keyPosition = keyPosition,
        onClick = onClick
    ) {
        Text(
            text = label,
            fontSize = fontSize,
            color = textColor
        )
    }
}
