#pragma once
#include "GFX.hpp"
#include "pch.hpp"
class World; class Entity;

class Map {
private:


public:



    // return true was collision with wall 
    // get relative hitbox
    static ColData checkEntitiesOverlap(World* world, Entity* mainEntity, const SDL_FRect& pos);
    static MoveData movePosWithCol(World* world, Entity* entity);
    static void applyMoveData(const MoveData& data, Entity* entity);
private:
    static void moveEntity(Entity* entity);
};


