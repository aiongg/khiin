package be.chiahpa.khiin.ui

import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.Dp

@Composable
fun CandidatesBar(height: Dp) {
    Row(
        Modifier
            .fillMaxWidth()
            .height(height)
    ) {
        Text("Candidates shown here")
    }
}
