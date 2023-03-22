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

        setContent {
            KhiinTheme {
                SettingsScreen()
            }
        }
    }

    override fun onDestroy() {
        super.onDestroy()
    }

    private lateinit var engineManager: EngineManager
}
