#include "core/Window.h"
#include "core/Logger.h"
#include "core/GlfwUserData.h"

#include <GLFW/glfw3.h>

namespace Engine {

static GlfwUserData s_UserData;

bool Window::Init(const WindowConfig& config) {
    LOG_INFO("Window", "Initializing GLFW...");

    if (!glfwInit()) {
        LOG_FATAL("Window", "Failed to initialize GLFW!");
        return false;
    }

    // We'll use Vulkan later — no OpenGL context
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, config.resizable ? GLFW_TRUE : GLFW_FALSE);

    GLFWmonitor* monitor = config.fullscreen ? glfwGetPrimaryMonitor() : nullptr;

    m_Handle = glfwCreateWindow(config.width, config.height, 
                                 config.title.c_str(), monitor, nullptr);
    if (!m_Handle) {
        LOG_FATAL("Window", "Failed to create GLFW window!");
        glfwTerminate();
        return false;
    }

    // Store shared user data for callbacks
    s_UserData.window = this;
    glfwSetWindowUserPointer(m_Handle, &s_UserData);
    glfwSetFramebufferSizeCallback(m_Handle, FramebufferResizeCallback);

    // Get actual framebuffer size
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(m_Handle, &fbWidth, &fbHeight);
    m_Width  = fbWidth;
    m_Height = fbHeight;

    LOG_INFO("Window", "Window created: {}x{} — \"{}\"", m_Width, m_Height, config.title);
    return true;
}

void Window::Shutdown() {
    if (m_Handle) {
        glfwDestroyWindow(m_Handle);
        m_Handle = nullptr;
        LOG_INFO("Window", "Window destroyed");
    }
    glfwTerminate();
    LOG_INFO("Window", "GLFW terminated");
}

bool Window::IsOpen() const {
    return m_Handle && !glfwWindowShouldClose(m_Handle);
}

void Window::PollEvents() {
    glfwPollEvents();
}

void Window::SwapBuffers() {
    // No-op for now — Vulkan will handle presentation
}

float Window::GetAspectRatio() const {
    if (m_Height == 0) return 1.0f;
    return static_cast<float>(m_Width) / static_cast<float>(m_Height);
}

void Window::SetTitle(const std::string& title) {
    if (m_Handle) {
        glfwSetWindowTitle(m_Handle, title.c_str());
    }
}

void Window::FramebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto* data = static_cast<GlfwUserData*>(glfwGetWindowUserPointer(window));
    if (data && data->window) {
        data->window->m_Width  = width;
        data->window->m_Height = height;
        data->window->m_Resized = true;
        LOG_INFO("Window", "Framebuffer resized: {}x{}", width, height);
    }
}

} // namespace Engine
