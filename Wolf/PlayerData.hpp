#pragma once
struct PlayerData {
    enum class Weapon : int
    {
        KNIFE,
        PISTOL,
        RIFLE,
        MACHINE_GUN,
    };
    Weapon weapon = Weapon::PISTOL;
    int ammo = 500;
    int health = 100;
    int points = 0;
    int lives = 3;


};
