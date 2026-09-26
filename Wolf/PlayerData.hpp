#pragma once
enum class WeaponType : int
{
    KNIFE,
    PISTOL,
    RIFLE,
    MACHINE_GUN,
};

struct PlayerData {
    std::vector<WeaponType> weapons;
    int ammo = 500;
    int health = 100;
    int points = 0;
    int lives = 3;


};
