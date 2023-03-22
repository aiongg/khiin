package be.chiahpa.khiin

import android.content.Context
import android.os.Bundle
import android.util.Log
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import be.chiahpa.khiin.ui.theme.KhiinIMETheme
import khiin.proto.CommandType
import khiin.proto.candidateListOrNull
import khiin.proto.command
import khiin.proto.keyEvent
import khiin.proto.request
import java.io.File
import java.io.FileOutputStream
import java.io.IOException
import java.io.InputStream
import java.io.OutputStream
import kotlin.jvm.Throws

@Throws(IOException::class)
private fun copyFile(inputStream: InputStream, outputStream: OutputStream) {
    val buffer = ByteArray(8192)
    var read: Int
    while (inputStream.read(buffer).also { read = it } != -1) {
        outputStream.write(buffer, 0, read)
    }
}

private fun copyAssetToFiles(
    context: Context,
    filename: String,
    overwrite: Boolean = false
) {
    if (context.assets.list("")?.contains(filename) != true) {
        return
    }

    val outFile = File(context.filesDir, filename)

    if (outFile.exists() && !overwrite) {
        return
    }

    val instream = context.assets.open(filename)
    val outstream = FileOutputStream(outFile)

    try {
        copyFile(instream, outstream)
    } catch (e: Exception) {
        Log.e("copyfile", "Could not copy $filename", e)
    }

    instream.close()
    outstream.flush()
    outstream.close()
}

class SettingsActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        copyAssetToFiles(this, "khiin.db")

        engineManager = EngineManager(File(filesDir, "khiin.db").absolutePath)

        val cmd = request {
                type = CommandType.CMD_SEND_KEY
                keyEvent = keyEvent {
                    keyCode = 97
                }
            }

        val ok = engineManager.sendCommand(cmd)

        setContent {
            KhiinIMETheme {
                // A surface container using the 'background' color from the theme
                Surface(
                    modifier = Modifier.fillMaxSize(),
                    color = MaterialTheme.colorScheme.background
                ) {
                    Row {
                        ok.response.candidateList.candidatesList.forEach {
                            Text(it.value)
                        }
                    }
                }
            }
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        engineManager.shutdown()
    }

    private lateinit var engineManager: EngineManager
}

@Composable
fun Greeting(name: String, modifier: Modifier = Modifier) {
    Text(
        text = "Hello $name!",
        modifier = modifier
    )
}
