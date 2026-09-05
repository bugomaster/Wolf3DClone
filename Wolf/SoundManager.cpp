#include "SoundManager.hpp"
#include "pch.hpp"
//#define MUTE
bool SoundManager::init()
{
#ifdef MUTE
    return false;
#endif // MUTE

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        SDL_Log("Failed to open audio: %s", Mix_GetError());
        return false;
    }
    bool loads = true;
    
    loads &= this->loadSound("ahh", "Assets/Sounds/enemies/AHHHGSND.wav");
    loads &= this->loadSound("halt", "Assets/Sounds/enemies/HALTSND.wav");
    loads &= this->loadSound("mynaven", "Assets/Sounds/enemies/LEBENSND.wav");
    loads &= this->loadSound("ammo", "Assets/Sounds/general/AMMOPICK.wav");
    loads &= this->loadSound("gunfire", "Assets/Sounds/weapons/ATKPISTOLSND.wav");
    return loads;
}

void SoundManager::shutdown()
{
    for (auto& [name, sound] : mSounds)
        Mix_FreeChunk(sound);

    for (auto& [name, music] : mMusic)
        Mix_FreeMusic(music);

    mSounds.clear();
    mMusic.clear();

    Mix_CloseAudio();
}

bool SoundManager::loadSound(const std::string& name, const std::string& file)
{
    Mix_Chunk* sound = Mix_LoadWAV(file.c_str());

    if (!sound)
    {
        SDL_Log("Failed to load sound %s", file.c_str());
        return false;
    }

    mSounds[name] = sound;
    return true;
}

void SoundManager::playSound(const std::string& name, int loops, int channel)
{
    auto it = mSounds.find(name);

    if (it == mSounds.end())
        return;

    Mix_PlayChannel(channel, it->second, loops);
}

bool SoundManager::loadMusic(const std::string& name, const std::string& file)
{
    Mix_Music* music = Mix_LoadMUS(file.c_str());

    if (!music)
    {
        SDL_Log("Failed to load music %s", file.c_str());
        return false;
    }

    mMusic[name] = music;
    return true;
}

void SoundManager::playMusic(const std::string& name, int loops)
{
    auto it = mMusic.find(name);

    if (it == mMusic.end())
        return;

    Mix_PlayMusic(it->second, loops);
}

void SoundManager::stopMusic()
{
    Mix_HaltMusic();
}

void SoundManager::setSoundVolume(int volume)
{
    Mix_Volume(-1, volume);
}

void SoundManager::setMusicVolume(int volume)
{
    Mix_VolumeMusic(volume);
}