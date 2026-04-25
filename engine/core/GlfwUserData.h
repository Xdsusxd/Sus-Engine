#pragma once

namespace Engine {

class Window;
class Input;

// Shared data passed via glfwSetWindowUserPointer
// so multiple systems can receive GLFW callbacks
struct GlfwUserData {
    Window* window = nullptr;
    Input*  input  = nullptr;
};

} // namespace Engine
