#pragma once
class World;
class System {
public:
    virtual ~System() = default;

    virtual void update(World* world) = 0;
    virtual void onAddedToWorld(World* world) {}
    bool enabled = true;
};