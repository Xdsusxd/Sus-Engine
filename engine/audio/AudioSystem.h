#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>

// Forward declaration to avoid including miniaudio.h in the header
struct ma_engine;
struct ma_sound;

namespace Engine {

class AudioSystem {
public:
    static AudioSystem& Get() {
        static AudioSystem instance;
        return instance;
    }

    bool Init();
    void Shutdown();
    void Update();

    // Global listener (typically the camera)
    void SetListenerPosition(const glm::vec3& position, const glm::vec3& forward, const glm::vec3& up);

    // Play a standard 2D sound
    void PlaySound2D(const std::string& filePath, float volume = 1.0f, bool loop = false);

    // Play a spatialized 3D sound at a specific position
    void PlaySound3D(const std::string& filePath, const glm::vec3& position, float volume = 1.0f, bool loop = false);

private:
    AudioSystem() = default;
    ~AudioSystem() = default;

    AudioSystem(const AudioSystem&) = delete;
    AudioSystem& operator=(const AudioSystem&) = delete;

    ma_engine* m_Engine = nullptr;
    bool       m_Initialized = false;

    std::vector<ma_sound*> m_ActiveSounds;
};

} // namespace Engine
