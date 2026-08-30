#include "pch.hpp"
#include "Map.hpp"
#include "Components.hpp"






int Map::wallMap[GFX::MAP_H][GFX::MAP_W] =
{
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,-1,1,0,0,1,1,1,1,-1,1,1,1},
    {1,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

//int Map::wallMap[GFX::MAP_H][GFX::MAP_W] =
//{
//    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
//    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
//    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
//    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
//    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
//    {1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1},
//    {1,0,0,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,1},
//    {1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1},
//    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
//    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
//    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
//    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
//    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
//    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
//    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
//    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
//    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
//    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
//    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
//    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
//};
//


ColData Map::checkEntitiesOverlap(World* world, Entity* mainEntity, const SDL_FRect& pos)
{
    ColData colData = {};
    auto* cantColWith = mainEntity->getComponent<CantCollideWithComponent>();
    for (Entity* entity : world->getEntities())
    {
        if (entity == mainEntity)
            continue;
        if (cantColWith && cantColWith->checkEntityForComponent(entity))
            continue;

        if (entity->hasComponent<NotCollideableComponent>())
        {
            auto* collideableWith = entity->getComponent<NotCollideableComponent>();
            if (!collideableWith->checkEntityForComponent(mainEntity))
                continue;
        }



        const auto* otherPos = entity->getComponent<PositionComponent>();
        const auto& otherHB = otherPos->getRelativeHB();

        if (otherHB.w <= 0.f || otherHB.h <= 0.f)
            continue;
        auto overLap = AABBOverlapRect(pos, otherHB);
        if (overLap.w != 0.f || overLap.h != 0.f )// overlapping
        {
            colData.wasCol = true;
            colData.overLapRect = overLap;
            colData.entity = entity;
            return colData;
        }
            
    }

    return colData;
}



void Map::moveEntity(Entity* entity) {

    auto* vel = entity->getComponent<VelocityComponent>();
    auto* pos = entity->getComponent<PositionComponent>();
    pos->position.x += vel->dx;
    pos->position.y += vel->dy;


}
MoveData Map::movePosWithCol(World* world, Entity* entity) {
    
    
    
    
    
    const auto* originalPosition = entity->getComponent<PositionComponent>();
    const auto* originalVelocity = entity->getComponent<VelocityComponent>();
    

    auto positionModified = *originalPosition;
    auto velocityModified = *originalVelocity;
   

    MoveData moveData;
    moveData.newPos = positionModified.position;
    moveData.newVel = { velocityModified.dx, velocityModified.dy };

    // resolved col check
    {
        ColData resolvedCol = Map::checkEntitiesOverlap(world, entity, positionModified.getRelativeHB());
        if (resolvedCol.wasCol)
        {
            moveData.resolvedCol = resolvedCol;
            moveData.collisions.push_back(resolvedCol);
            if (!resolvedCol.entity->hasComponent<ColNotEffectMovement>())
            {
                if (resolvedCol.overLapRect.x == positionModified.position.x)
                {
                    velocityModified.dx = 0.f;
                    velocityModified.dy = 0.f;
                    positionModified.position.x += resolvedCol.overLapRect.w;
                }
                else if (positionModified.position.x > resolvedCol.overLapRect.x)
                {
                    velocityModified.dx = 0.f;
                    velocityModified.dy = 0.f;
                    positionModified.position.x -= resolvedCol.overLapRect.w;
                }
                if (resolvedCol.overLapRect.y == positionModified.position.y)
                {
                    velocityModified.dx = 0.f;
                    velocityModified.dy = 0.f;
                    positionModified.position.y += resolvedCol.overLapRect.h;
                }
                else if (positionModified.position.y > resolvedCol.overLapRect.y)
                {
                    velocityModified.dx = 0.f;
                    velocityModified.dy = 0.f;
                    positionModified.position.y -= resolvedCol.overLapRect.h;

                }

                moveData.newPos = positionModified.position;
                moveData.newVel = { velocityModified.dx, velocityModified.dy };
                moveData.wasCollsion = true;
                return moveData;

            }
        }
    }


    if (originalVelocity->dx != 0.0f)
    {
        Vector2f testPos = positionModified.position;
        testPos.x += originalVelocity->dx;

        PositionComponent temp = positionModified;;   // copy
        temp.position = testPos;

        SDL_FRect testHB = temp.getRelativeHB();
        ColData colDataX = Map::checkEntitiesOverlap(world, entity, testHB);
        if (colDataX.wasCol) 
        {
            moveData.wasCollsion = true;
            moveData.xCollision = colDataX;
            moveData.collisions.push_back(colDataX);
            if (colDataX.entity->hasComponent<ColNotEffectMovement>())
            {
                positionModified.position.x += originalVelocity->dx;
            }
            else
            {
                velocityModified.dx = 0.f;
            }
        }
        else
        {
            positionModified.position.x += originalVelocity->dx;
        }
    }
    if (originalVelocity->dy != 0.0f)
    {
        Vector2f testPos = positionModified.position;
        testPos.y += originalVelocity->dy;

        PositionComponent temp = positionModified;
        temp.position = testPos;

        SDL_FRect testHB = temp.getRelativeHB();

        ColData colDataY = Map::checkEntitiesOverlap(world, entity, testHB);
        if (colDataY.wasCol) 
        {
            moveData.wasCollsion = true;
            moveData.collisions.push_back(colDataY);
            moveData.yCollision = colDataY;
            if (colDataY.entity->hasComponent<ColNotEffectMovement>())
            {
                positionModified.position.y += originalVelocity->dy;
            }
            else
            {
                velocityModified.dy = 0.f;
            }
        }
        else 
        {
            positionModified.position.y += originalVelocity->dy;
        }
    }
    moveData.newPos = positionModified.position;
    moveData.newVel = { velocityModified.dx, velocityModified.dy };
    return moveData;
}
void Map::applyMoveData(const MoveData& data, Entity* entity)
{
    entity->getComponent<PositionComponent>()->position = data.newPos;
    entity->getComponent<VelocityComponent>()->dx = data.newVel.x;
    entity->getComponent<VelocityComponent>()->dy = data.newVel.y;
}