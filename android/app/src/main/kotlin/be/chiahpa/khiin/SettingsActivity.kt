package be.chiahpa.khiin

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import be.chiahpa.khiin.service.copyAssetToFiles
import be.chiahpa.khiin.settings.SettingsScreen
import be.chiahpa.khiin.ui.theme.KhiinTheme
import khiin.proto.CommandType
import khiin.proto.keyEvent
import khiin.proto.request
import java.io.File

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
            KhiinTheme {
                SettingsScreen(ok)
            }
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        engineManager.shutdown()
    }

    private lateinit var engineManager: EngineManager
}
