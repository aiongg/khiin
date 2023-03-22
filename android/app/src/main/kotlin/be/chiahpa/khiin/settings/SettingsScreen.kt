package be.chiahpa.khiin.settings

import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.material3.TextField
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import be.chiahpa.khiin.utils.loggerFor
import khiin.proto.Command

private val log = loggerFor("SettingsScreen")

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun SettingsScreen() {
    var input by remember {
        mutableStateOf("a")
    }

    Surface(
        modifier = Modifier.fillMaxSize(),
        color = MaterialTheme.colorScheme.background
    ) {
        Column {
            Row {
                TextField(value = input, onValueChange = { input = it })
            }
        }
    }
}
