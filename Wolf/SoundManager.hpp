#pragma once

#include <SDL_mixer.h>

#include <string>
#include <unordered_map>

class SoundManager
{
public:
    bool init();
    void shutdown();

    bool loadSound(const std::string& name, const std::string& file);
    void playSound(const std::string& name, int loops = 0, int channel = -1);

    bool loadMusic(const std::string& name, const std::string& file);
    void playMusic(const std::string& name, int loops = -1);
    void stopMusic();

    void setSoundVolume(int volume);
    void setMusicVolume(int volume);

private:
    std::unordered_map<std::string, Mix_Chunk*> mSounds;
    std::unordered_map<std::string, Mix_Music*> mMusic;
};