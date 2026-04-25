#include "core/Input.h"
#include "core/Logger.h"
#include "core/GlfwUserData.h"

#include <GLFW/glfw3.h>

namespace Engine {

bool Input::Init(GLFWwindow* window) {
    if (!window) {
        LOG_ERROR("Input", "Cannot init Input — null window handle");
        return false;
    }

    // Register ourselves in the shared user data
    auto* data = static_cast<GlfwUserData*>(glfwGetWindowUserPointer(window));
    if (data) {
        data->input = this;
    }

    glfwSetKeyCallback(window, KeyCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetCursorPosCallback(window, CursorPosCallback);
    glfwSetScrollCallback(window, ScrollCallback);

    m_Keys.fill(false);
    m_PrevKeys.fill(false);
    m_MouseButtons.fill(false);
    m_PrevMouseButtons.fill(false);
    m_FirstMouse = true;

    LOG_INFO("Input", "Input system initialized");
    return true;
}

void Input::Shutdown() {
    LOG_INFO("Input", "Input system shut down");
}

void Input::Update() {
    // Copy current state to previous
    m_PrevKeys = m_Keys;
    m_PrevMouseButtons = m_MouseButtons;

    // Calculate mouse delta
    if (m_FirstMouse) {
        m_PrevMouseX = m_MouseX;
        m_PrevMouseY = m_MouseY;
        m_FirstMouse = false;
    }
    m_MouseDX = m_MouseX - m_PrevMouseX;
    m_MouseDY = m_MouseY - m_PrevMouseY;
    m_PrevMouseX = m_MouseX;
    m_PrevMouseY = m_MouseY;

    // Reset scroll delta (consumed per frame)
    m_ScrollDelta = 0.0;
}

// ── Keyboard queries ──────────────────────────────────────────
bool Input::IsKeyDown(Key key) const {
    int k = static_cast<int>(key);
    return (k >= 0 && k < MAX_KEYS) ? m_Keys[k] : false;
}

bool Input::IsKeyPressed(Key key) const {
    int k = static_cast<int>(key);
    return (k >= 0 && k < MAX_KEYS) ? (m_Keys[k] && !m_PrevKeys[k]) : false;
}

bool Input::IsKeyReleased(Key key) const {
    int k = static_cast<int>(key);
    return (k >= 0 && k < MAX_KEYS) ? (!m_Keys[k] && m_PrevKeys[k]) : false;
}

// ── Mouse queries ─────────────────────────────────────────────
bool Input::IsMouseDown(MouseButton btn) const {
    int b = static_cast<int>(btn);
    return (b >= 0 && b < MAX_MOUSE_BUTTONS) ? m_MouseButtons[b] : false;
}

bool Input::IsMousePressed(MouseButton btn) const {
    int b = static_cast<int>(btn);
    return (b >= 0 && b < MAX_MOUSE_BUTTONS) ? (m_MouseButtons[b] && !m_PrevMouseButtons[b]) : false;
}

bool Input::IsMouseReleased(MouseButton btn) const {
    int b = static_cast<int>(btn);
    return (b >= 0 && b < MAX_MOUSE_BUTTONS) ? (!m_MouseButtons[b] && m_PrevMouseButtons[b]) : false;
}

// ── Gamepad stub ──────────────────────────────────────────────
bool Input::IsGamepadConnected(int index) const {
    return glfwJoystickPresent(index) == GLFW_TRUE;
}

// ── GLFW Callbacks (static) ──────────────────────────────────
void Input::KeyCallback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/) {
    auto* data = static_cast<GlfwUserData*>(glfwGetWindowUserPointer(window));
    if (!data || !data->input || key < 0 || key >= MAX_KEYS) return;

    auto* self = data->input;
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        self->m_Keys[key] = true;
    } else if (action == GLFW_RELEASE) {
        self->m_Keys[key] = false;
    }
}

void Input::MouseButtonCallback(GLFWwindow* window, int button, int action, int /*mods*/) {
    auto* data = static_cast<GlfwUserData*>(glfwGetWindowUserPointer(window));
    if (!data || !data->input || button < 0 || button >= MAX_MOUSE_BUTTONS) return;

    auto* self = data->input;
    if (action == GLFW_PRESS) {
        self->m_MouseButtons[button] = true;
    } else if (action == GLFW_RELEASE) {
        self->m_MouseButtons[button] = false;
    }
}

void Input::CursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    auto* data = static_cast<GlfwUserData*>(glfwGetWindowUserPointer(window));
    if (!data || !data->input) return;
    data->input->m_MouseX = xpos;
    data->input->m_MouseY = ypos;
}

void Input::ScrollCallback(GLFWwindow* window, double /*xoffset*/, double yoffset) {
    auto* data = static_cast<GlfwUserData*>(glfwGetWindowUserPointer(window));
    if (!data || !data->input) return;
    data->input->m_ScrollDelta = yoffset;
}

} // namespace Engine
