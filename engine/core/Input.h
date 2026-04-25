#pragma once

#include <array>
#include <cstdint>

struct GLFWwindow;

namespace Engine {

// ── Key codes (mirrors GLFW key codes) ────────────────────────
enum class Key : int {
    Unknown = -1,
    Space = 32,
    Apostrophe = 39,
    Comma = 44,
    Minus = 45,
    Period = 46,
    Slash = 47,
    Num0 = 48, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
    Semicolon = 59,
    Equal = 61,
    A = 65, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    LeftBracket = 91,
    Backslash = 92,
    RightBracket = 93,
    GraveAccent = 96,
    Escape = 256,
    Enter = 257,
    Tab = 258,
    Backspace = 259,
    Insert = 260,
    Delete = 261,
    Right = 262,
    Left = 263,
    Down = 264,
    Up = 265,
    PageUp = 266,
    PageDown = 267,
    Home = 268,
    End = 269,
    CapsLock = 280,
    ScrollLock = 281,
    NumLock = 282,
    PrintScreen = 283,
    Pause = 284,
    F1 = 290, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    LeftShift = 340,
    LeftControl = 341,
    LeftAlt = 342,
    LeftSuper = 343,
    RightShift = 344,
    RightControl = 345,
    RightAlt = 346,
    RightSuper = 347
};

// ── Mouse buttons ─────────────────────────────────────────────
enum class MouseButton : int {
    Left   = 0,
    Right  = 1,
    Middle = 2,
    Button4 = 3,
    Button5 = 4
};

class Input {
public:
    static constexpr int MAX_KEYS = 512;
    static constexpr int MAX_MOUSE_BUTTONS = 8;

    // Call once with the GLFW window to install callbacks
    bool Init(GLFWwindow* window);
    void Shutdown();

    // Call at the END of each frame to update previous state
    void Update();

    // ── Keyboard ──────────────────────────────────────────────
    bool IsKeyDown(Key key) const;       // Held this frame
    bool IsKeyPressed(Key key) const;    // Just pressed this frame
    bool IsKeyReleased(Key key) const;   // Just released this frame

    // ── Mouse ─────────────────────────────────────────────────
    bool IsMouseDown(MouseButton btn) const;
    bool IsMousePressed(MouseButton btn) const;
    bool IsMouseReleased(MouseButton btn) const;

    double GetMouseX() const { return m_MouseX; }
    double GetMouseY() const { return m_MouseY; }
    double GetMouseDeltaX() const { return m_MouseDX; }
    double GetMouseDeltaY() const { return m_MouseDY; }
    double GetScrollDelta() const { return m_ScrollDelta; }

    // ── Gamepad (stub) ────────────────────────────────────────
    bool IsGamepadConnected(int index = 0) const;
    // Future: axis values, button states

private:
    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

    // Current frame state
    std::array<bool, MAX_KEYS> m_Keys{};
    std::array<bool, MAX_KEYS> m_PrevKeys{};

    std::array<bool, MAX_MOUSE_BUTTONS> m_MouseButtons{};
    std::array<bool, MAX_MOUSE_BUTTONS> m_PrevMouseButtons{};

    double m_MouseX  = 0.0;
    double m_MouseY  = 0.0;
    double m_PrevMouseX = 0.0;
    double m_PrevMouseY = 0.0;
    double m_MouseDX = 0.0;
    double m_MouseDY = 0.0;
    double m_ScrollDelta = 0.0;
    bool   m_FirstMouse = true;
};

} // namespace Engine
