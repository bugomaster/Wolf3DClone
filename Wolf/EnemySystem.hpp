#pragma once
#include "System.hpp"
#include "EnemyState.hpp"


class GameScene;



class EnemySystem : public System {
public:
    EnemySystem(GameScene* scene) :
        scene(scene)
    {
    }
    ~EnemySystem() {
        freeEnemyStates(this->states);
    }
    void update(World* world) override;
    void onAddedToWorld(World* world) override;

private:
    // return if the enemy see the player
    bool seePos(Entity* enemy, Vector2f pos);



    EnemyState* changeState(Entity* enemy, EnemyState* newState);
    EnemyState* buildEnemyStates();
    void freeEnemyStates(EnemyState* first);
    EnemyState* states = nullptr;
    GameScene* scene;
};

