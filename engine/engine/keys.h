#pragma once

namespace khiin::engine {

enum class KeyCode {
    BACK = 0x08,
    TAB = 0x09,
    ENTER = 0x0D,
    SHIFT = 0x10,
    CTRL = 0x11,
    ALT = 0x12,
    ESC = 0x1B,
    SPACE = 0x20,
    PGUP = 0x21,
    PGDN = 0x22,
    END = 0x23,
    HOME = 0x24,
    LEFT = 0x25,
    UP = 0x26,
    RIGHT = 0x27,
    DOWN = 0x28,
    DEL = 0x2E,
    D0 = 0x30,
    D1 = 0x31,
    D2 = 0x32,
    D3 = 0x33,
    D4 = 0x34,
    D5 = 0x35,
    D6 = 0x36,
    D7 = 0x37,
    D8 = 0x38,
    D9 = 0x39,
    A = 0x41,
    B = 0x42,
    C = 0x43,
    D = 0x44,
    E = 0x45,
    F = 0x46,
    G = 0x47,
    H = 0x48,
    I = 0x49,
    J = 0x4A,
    K = 0x4B,
    L = 0x4C,
    M = 0x4D,
    N = 0x4E,
    O = 0x4F,
    P = 0x50,
    Q = 0x51,
    R = 0x52,
    S = 0x53,
    T = 0x54,
    U = 0x55,
    V = 0x56,
    W = 0x57,
    X = 0x58,
    Y = 0x59,
    Z = 0x5A,
    A_LC = 0x61,
    B_LC = 0x62,
    C_LC = 0x63,
    D_LC = 0x64,
    E_LC = 0x65,
    F_LC = 0x66,
    G_LC = 0x67,
    H_LC = 0x68,
    I_LC = 0x69,
    J_LC = 0x6A,
    K_LC = 0x6B,
    L_LC = 0x6C,
    M_LC = 0x6D,
    N_LC = 0x6E,
    O_LC = 0x6F,
    P_LC = 0x70,
    Q_LC = 0x71,
    R_LC = 0x72,
    S_LC = 0x73,
    T_LC = 0x74,
    U_LC = 0x75,
    V_LC = 0x76,
    W_LC = 0x77,
    X_LC = 0x78,
    Y_LC = 0x79,
    Z_LC = 0x7A,
};

inline bool IsAlphaNumeric(KeyCode code) {
    return KeyCode::D0 <= code && code <= KeyCode::D9 || KeyCode::A <= code && code <= KeyCode::Z ||
           KeyCode::A_LC <= code && code <= KeyCode::Z_LC;
}

} // namespace khiin::engine
