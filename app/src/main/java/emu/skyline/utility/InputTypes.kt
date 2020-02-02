package emu.skyline.utility
// Needs to be synchronised with input/npad.h  in libskyline
enum class NpadButton(val id:Long) {
    A(1 shl 0),
    B(1 shl 1),
    X(1 shl 2),
    Y(1 shl 3),
    L3(1 shl 4),
    R3(1 shl 5),
    L(1 shl 6),
    R(1 shl 7),
    ZL(1 shl 8),
    ZR(1 shl 9),
    Plus(1 shl 10),
    Minus(1 shl 11),
    DpadLeft(1 shl 12),
    DpadUp(1 shl 13),
    DpadRight(1 shl 14),
    DpadDown(1 shl 15),
    LeftStickLeft(1 shl 16),
    LeftStickUp(1 shl 17),
    LeftStickRight(1 shl 18),
    LeftStickDown(1 shl 19),
    RightStickLeft(1 shl 20),
    RightStickUp(1 shl 21),
    RightStickRight(1 shl 22),
    RightStickDown(1 shl 23),
    LeftSL(1 shl 24),
    LeftSR(1 shl 25),
    RightSL(1 shl 26),
    RightSR(1 shl 27),
    Touch(1 shl 28)
}

enum class NpadAxisId() {
    RX,
    RY,
    LX,
    LY
}

enum class ButtonState(val state: Boolean) {
    Released(false),
    Pressed(true)
}