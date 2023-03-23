package be.chiahpa.khiin.settings

import android.content.Context
import androidx.datastore.core.DataStore
import androidx.datastore.preferences.core.Preferences
import androidx.datastore.preferences.core.edit
import androidx.datastore.preferences.core.floatPreferencesKey
import androidx.datastore.preferences.preferencesDataStore
import be.chiahpa.khiin.Khiin
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.map

private const val PREFS_FILE = "settings"
private val KBD_ROW_HEIGHT = floatPreferencesKey("keyboard_row_height")

object Settings {
    private val Context.settingsDataStore: DataStore<Preferences> by preferencesDataStore(
        PREFS_FILE
    )

    private val dataStore
        get() = Khiin.context.settingsDataStore

    suspend fun setRowHeight(heightDp: Float) {
        dataStore.edit {
            it[KBD_ROW_HEIGHT] = heightDp
        }
    }

    val rowHeightFlow: Flow<Float> = dataStore.data.map {
        it[KBD_ROW_HEIGHT] ?: 60f
    }
}
