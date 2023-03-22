package be.chiahpa.khiin.ui

import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.Dp
import androidx.compose.ui.unit.dp

val layout = listOf(
    "qwertyuiop".split(""),
    "asdfghjkl".split(""),
    "zxcvbnm".split("")
)

@Composable
fun KeyboardScreen(rowHeight: Dp = 56.dp) {
    Surface(
        Modifier
            .fillMaxWidth()
    ) {
        Column(Modifier.fillMaxWidth()) {
            Row(
                Modifier
                    .fillMaxWidth()
                    .height(rowHeight)) {
                Text("Candidates shown here")
            }
            layout.forEach {
                Row(Modifier
                    .fillMaxWidth()
                    .height(rowHeight)) {
                    it.forEach {
                        Key(it)
                    }
                }
            }
        }
    }
}

@Composable
fun Key(label: String) {
    TextButton(onClick = {}) {
        Text(label)
    }
}
