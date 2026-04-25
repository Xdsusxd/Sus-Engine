#pragma once

#include <string>
#include <cstdint>

struct GLFWwindow;

namespace Engine {

struct WindowConfig {
    std::string title  = "FY-Engine";
    int32_t width      = 1280;
    int32_t height     = 720;
    bool    resizable  = true;
    bool    vsync      = true;
    bool    fullscreen = false;
};

class Window {
public:
    bool Init(const WindowConfig& config = {});
    void Shutdown();

    // Returns true if the window should remain open
    bool IsOpen() const;

    // Poll events and swap buffers
    void PollEvents();
    void SwapBuffers();

    // Getters
    GLFWwindow* GetHandle() const { return m_Handle; }
    int32_t GetWidth() const { return m_Width; }
    int32_t GetHeight() const { return m_Height; }
    float GetAspectRatio() const;
    bool WasResized() const { return m_Resized; }
    void ResetResizedFlag() { m_Resized = false; }

    // Set title dynamically (e.g. for FPS counter)
    void SetTitle(const std::string& title);

private:
    static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);

    GLFWwindow* m_Handle = nullptr;
    int32_t     m_Width  = 0;
    int32_t     m_Height = 0;
    bool        m_Resized = false;
};

} // namespace Engine
