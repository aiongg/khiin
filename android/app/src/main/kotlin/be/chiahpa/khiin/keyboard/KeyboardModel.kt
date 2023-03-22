package be.chiahpa.khiin.keyboard

enum class KeyType {
    LETTER,
    SHIFT,
    BACKSPACE,
    SYMBOLS,
    SPACEBAR,
    ENTER
}

data class KeyboardRowData(val keys: List<KeyData>)

data class KeyData(
    val label: String? = null,
    val weight: Float = 1f,
    val type: KeyType = KeyType.LETTER
)
