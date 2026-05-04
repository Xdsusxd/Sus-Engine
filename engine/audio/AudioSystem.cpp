#include "audio/AudioSystem.h"
#include "core/Logger.h"

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

namespace Engine {

bool AudioSystem::Init() {
    if (m_Initialized) return true;

    m_Engine = new ma_engine;

    ma_engine_config config = ma_engine_config_init();
    config.listenerCount = 1; // 1 listener for 3D audio

    ma_result result = ma_engine_init(&config, m_Engine);
    if (result != MA_SUCCESS) {
        LOG_ERROR("Audio", "Failed to initialize miniaudio engine");
        delete m_Engine;
        m_Engine = nullptr;
        return false;
    }

    m_Initialized = true;
    LOG_INFO("Audio", "Miniaudio initialized successfully");
    return true;
}

void AudioSystem::Shutdown() {
    if (!m_Initialized) return;

    for (ma_sound* sound : m_ActiveSounds) {
        ma_sound_uninit(sound);
        delete sound;
    }
    m_ActiveSounds.clear();

    if (m_Engine) {
        ma_engine_uninit(m_Engine);
        delete m_Engine;
        m_Engine = nullptr;
    }

    m_Initialized = false;
    LOG_INFO("Audio", "Miniaudio shut down");
}

void AudioSystem::Update() {
    if (!m_Initialized || !m_Engine) return;

    // Garbage collect finished 3D sounds
    for (auto it = m_ActiveSounds.begin(); it != m_ActiveSounds.end(); ) {
        ma_sound* sound = *it;
        if (ma_sound_at_end(sound)) {
            ma_sound_uninit(sound);
            delete sound;
            it = m_ActiveSounds.erase(it);
        } else {
            ++it;
        }
    }
}

void AudioSystem::SetListenerPosition(const glm::vec3& position, const glm::vec3& forward, const glm::vec3& up) {
    if (!m_Initialized || !m_Engine) return;

    ma_engine_listener_set_position(m_Engine, 0, position.x, position.y, position.z);
    ma_engine_listener_set_direction(m_Engine, 0, forward.x, forward.y, forward.z);
    ma_engine_listener_set_world_up(m_Engine, 0, up.x, up.y, up.z);
}

void AudioSystem::PlaySound2D(const std::string& filePath, float volume, bool loop) {
    if (!m_Initialized || !m_Engine) return;

    // For fire-and-forget 2D, we can just use play_sound unless we want looping
    if (!loop) {
        ma_engine_play_sound(m_Engine, filePath.c_str(), nullptr);
    } else {
        ma_sound* sound = new ma_sound;
        ma_result result = ma_sound_init_from_file(m_Engine, filePath.c_str(), 0, nullptr, nullptr, sound);
        if (result == MA_SUCCESS) {
            ma_sound_set_volume(sound, volume);
            ma_sound_set_looping(sound, MA_TRUE);
            ma_sound_start(sound);
            m_ActiveSounds.push_back(sound);
        } else {
            delete sound;
            LOG_ERROR("Audio", "Failed to load 2D sound: {}", filePath);
        }
    }
}

void AudioSystem::PlaySound3D(const std::string& filePath, const glm::vec3& position, float volume, bool loop) {
    if (!m_Initialized || !m_Engine) return;

    ma_sound* sound = new ma_sound;
    ma_result result = ma_sound_init_from_file(m_Engine, filePath.c_str(), 0, nullptr, nullptr, sound);
    
    if (result == MA_SUCCESS) {
        ma_sound_set_position(sound, position.x, position.y, position.z);
        ma_sound_set_volume(sound, volume);
        ma_sound_set_looping(sound, loop ? MA_TRUE : MA_FALSE);
        
        // Settings for 3D attenuation
        ma_sound_set_spatialization_enabled(sound, MA_TRUE);
        ma_sound_set_min_distance(sound, 2.0f);
        ma_sound_set_max_distance(sound, 30.0f);
        
        ma_sound_start(sound);
        m_ActiveSounds.push_back(sound);
    } else {
        delete sound;
        LOG_ERROR("Audio", "Failed to load 3D sound: {}", filePath);
    }
}

} // namespace Engine
