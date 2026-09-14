#pragma once
#include "System.hpp"
#include "EnemyState.hpp"
struct PathNode
{
    int x;
    int y;

    float g; // distance from start
    float h; // distance to goal
    float f; // g + h

    PathNode* parent;

    PathNode(int x, int y)
        : x(x), y(y), g(0), h(0), f(0), parent(nullptr)
    {
    }
};





class EnemySystem : public System {
public:
    EnemySystem(GameScene* scene);
    ~EnemySystem() {
        freeEnemyStates(this->states);
    }
    void update(World* world) override;
    void onAddedToWorld(World* world) override;

private:
    // return if the enemy see the player
    bool seePos(Entity* enemy, Vector2f pos);


    float heuristic(PathNode* a, PathNode* b);
    std::vector<Vector2f> AStar(Vector2f start, Vector2f end);

    EnemyState* changeState(Entity* enemy, EnemyState* newState);
    EnemyState* buildEnemyStates();
    void freeEnemyStates(EnemyState* first);
    EnemyState* states = nullptr;
    GameScene* gameScene;
};

