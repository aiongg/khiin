package be.chiahpa.khiin.keyboard

import be.chiahpa.khiin.keyboard.components.KeyPosition

interface KeyboardLayoutScope {
    fun row(content: KeyboardRowScope.() -> Unit)
}

class KeyboardLayoutScopeImpl : KeyboardLayoutScope {
    val rows: MutableList<KeyboardRowData> = mutableListOf()

    override fun row(content: KeyboardRowScope.() -> Unit) {
        val scopeContent = KeyboardRowScopeImpl().apply(content)
        rows.add(KeyboardRowData(scopeContent.keys))
    }
}

interface KeyboardRowScope {
    fun key(
        label: String,
        weight: Float = 1f,
        position: KeyPosition = KeyPosition.FULL_WEIGHT
    )

    fun shift(weight: Float = 1f)

    fun backspace(weight: Float = 1f)

    fun symbols(weight: Float = 1f)

    fun spacebar(weight: Float = 1f)

    fun enter(weight: Float = 1f)
}

class KeyboardRowScopeImpl : KeyboardRowScope {
    val keys: MutableList<KeyData> = mutableListOf()

    override fun key(label: String, weight: Float, position: KeyPosition) {
        keys.add(KeyData(weight = weight, label = label, position = position))
    }

    override fun shift(weight: Float) {
        keys.add(KeyData(weight = weight, type = KeyType.SHIFT))
    }

    override fun backspace(weight: Float) {
        keys.add(KeyData(weight = weight, type = KeyType.BACKSPACE))
    }

    override fun symbols(weight: Float) {
        keys.add(KeyData(weight = weight, type = KeyType.SYMBOLS))
    }

    override fun spacebar(weight: Float) {
        keys.add(KeyData(weight = weight, type = KeyType.SPACEBAR))
    }

    override fun enter(weight: Float) {
        keys.add(KeyData(weight = weight, type = KeyType.ENTER))
    }
}