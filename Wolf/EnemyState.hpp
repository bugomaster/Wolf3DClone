#pragma once
#include <functional>
class Entity;

enum class EnemyStateType {
    Patrol,
    Alert,
    Chase,
    Attack
};

struct EnemyState {
    EnemyStateType type;

    EnemyState* next = nullptr;
    EnemyState* prev = nullptr;
    std::function<void(Entity*)> onEnter;
    std::function<void(Entity*)> onUpdate;
    std::function<void(Entity*)> onExit;


    //attack
    bool dealDamage = false;
    int ticks = 0;

};


