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
import khiin.proto.Command

fun loggerFor(tag: String): (String) -> Unit {
    return { android.util.Log.d(tag, it) }
}

private val log = loggerFor("SettingsScreen")

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun SettingsScreen(command: Command) {
    var input by remember {
        mutableStateOf("")
    }

    log("here")

    Surface(
        modifier = Modifier.fillMaxSize(),
        color = MaterialTheme.colorScheme.background
    ) {
        Column {
            Row {
                command.response.candidateList.candidatesList.forEach {
                    Text(it.value)
                }
            }


            Row {
                TextField(value = input, onValueChange = { input = it })
            }
        }
    }
}
