#pragma once

#include <chrono>

namespace Engine {

class Time {
public:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = Clock::time_point;

    void Init() {
        m_StartTime = Clock::now();
        m_LastFrame = m_StartTime;
        m_DeltaTime = 0.0f;
        m_TotalTime = 0.0f;
        m_FrameCount = 0;
    }

    void Update() {
        auto now = Clock::now();
        std::chrono::duration<float> delta = now - m_LastFrame;
        m_DeltaTime = delta.count();
        
        std::chrono::duration<float> total = now - m_StartTime;
        m_TotalTime = total.count();
        
        m_LastFrame = now;
        m_FrameCount++;
    }

    // Delta time in seconds
    float DeltaTime() const { return m_DeltaTime; }

    // Total elapsed time since Init() in seconds
    float TotalTime() const { return m_TotalTime; }

    // Frames per second (smoothed)
    float FPS() const { return m_DeltaTime > 0.0f ? 1.0f / m_DeltaTime : 0.0f; }

    // Total frames rendered
    uint64_t FrameCount() const { return m_FrameCount; }

private:
    TimePoint m_StartTime{};
    TimePoint m_LastFrame{};
    float     m_DeltaTime = 0.0f;
    float     m_TotalTime = 0.0f;
    uint64_t  m_FrameCount = 0;
};

} // namespace Engine
