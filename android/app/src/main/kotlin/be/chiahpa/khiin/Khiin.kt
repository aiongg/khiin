package be.chiahpa.khiin

import android.app.Application
import android.content.Context
import androidx.datastore.core.DataStore
import androidx.datastore.preferences.core.Preferences
import androidx.datastore.preferences.preferencesDataStore

//private val Context.dataStore: DataStore<Preferences> by preferencesDataStore(name = "settings")

class Khiin: Application() {
    companion object {
        lateinit var context: Khiin
            private set

//        val dataStore: DataStore<Preferences>
//            get() = context.dataStore
    }

    override fun onCreate() {
        super.onCreate()
        context = this
    }
}
