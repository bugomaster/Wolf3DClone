#include "EnemySystem.hpp"
#include "Components.hpp"

#include "GameScene.hpp"
#include "Map.hpp"

EnemyState* EnemySystem::changeState(Entity* e, EnemyState* newState) {
    auto* comp = e->getComponent<EnemyComponent>();

    if (comp->currentState && comp->currentState->onExit)
        comp->currentState->onExit(e);

    comp->prevState = comp->currentState;
    comp->currentState = newState;

    if (newState && newState->onEnter)
        newState->onEnter(e);

    return newState;
}
EnemyState* EnemySystem::buildEnemyStates() {

    auto* patrol = new EnemyState{ EnemyStateType::Patrol };
    auto* alert = new EnemyState{ EnemyStateType::Alert };
    auto* chase = new EnemyState{ EnemyStateType::Chase };
    auto* attack = new EnemyState{ EnemyStateType::Attack };

    patrol->next = alert;

    alert->prev = patrol;
    alert->next = chase;

    chase->prev = alert;
    chase->next = attack;

    attack->prev = chase;

    patrol->onEnter = [](Entity* e) {
        auto* enemy = e->getComponent<EnemyComponent>();
        enemy->seeTargetTimer = enemy->reactionTime;
        };
    patrol->onUpdate = [](Entity* e) {
        auto* enemy = e->getComponent<EnemyComponent>();

        Vector2f target = Map::pickCoordsRadius(3, e->getComponent<PositionComponent>()->position)
            .convertTo<float>();

        enemy->target.pos = target;
    };
    alert->onEnter = [](Entity* e) {
        auto* enemy = e->getComponent<EnemyComponent>();
        enemy->seeTargetTimer = enemy->reactionTime;
    };

    alert->onUpdate = [](Entity* e) {
        auto* enemy = e->getComponent<EnemyComponent>();

        if (enemy->seeTargetTimer > 0)
            enemy->seeTargetTimer--;
        };

    chase->onEnter = [](Entity* e) {
        auto* enemy = e->getComponent<EnemyComponent>();
    };

    chase->onUpdate = [this](Entity* e) {
        auto* enemy = e->getComponent<EnemyComponent>();

        Entity* player = scene->playerEntity;
        auto* pPos = player->getComponent<PositionComponent>();
        auto* ePos = e->getComponent<PositionComponent>();

        float dx = pPos->position.x - ePos->position.x;
        float dy = pPos->position.y - ePos->position.y;

        enemy->target.pos = pPos->position;
        enemy->target.dist = std::sqrt(dx * dx + dy * dy);
        enemy->target.angle = std::atan2(dy, dx);
    };

    // ===== ATTACK =====
    attack->onEnter = [](Entity* e) {
        auto* enemy = e->getComponent<EnemyComponent>();
    };

    attack->onUpdate = [this](Entity* e) {
        auto* enemy = e->getComponent<EnemyComponent>();

        Entity* player = scene->playerEntity;
        auto* pPos = player->getComponent<PositionComponent>();
        auto* ePos = e->getComponent<PositionComponent>();

        float dx = pPos->position.x - ePos->position.x;
        float dy = pPos->position.y - ePos->position.y;

        float dist = std::sqrt(dx * dx + dy * dy);

        if (dist > 2.5f) {
            changeState(e, enemy->prevState); 
        }
    };

    return patrol; // HEAD of list
}


bool EnemySystem::reachedTarget(Entity* enemy) {
    auto* pos = enemy->getComponent<PositionComponent>();
    auto* enemyComp = enemy->getComponent<EnemyComponent>();
    auto* vel = enemy->getComponent<VelocityComponent>();

    Vector2f target = enemyComp->target.pos;

    float dx = target.x - pos->position.x;
    float dy = target.y - pos->position.y;

    float distSq = dx * dx + dy * dy;
    if (distSq < 0.05f * 0.05f)
        return true;

    float velSq = vel->dx * vel->dx + vel->dy * vel->dy;
    if (velSq < 0.00001f)
        return false;
    return true;
}


bool EnemySystem::seePlayer(Entity* enemy)
{
    Entity* player = scene->playerEntity;
    auto* posPlayer = player->getComponent<PositionComponent>();
    auto* posEnemy = enemy->getComponent<PositionComponent>();
    auto* enemyComp = enemy->getComponent<EnemyComponent>();
    auto* velEnemy = enemy->getComponent<VelocityComponent>();

    float enemyCx = posEnemy->position.x;
    float enemyCy = posEnemy->position.y;

    float playerCx = posPlayer->position.x;
    float playerCy = posPlayer->position.y;

    float eyeOffset = posEnemy->radius * 0.25f;

    Vector2f leftEye = { enemyCx - eyeOffset, enemyCy };
    Vector2f rightEye = { enemyCx + eyeOffset, enemyCy };

    auto canSeePlayer = [&](Vector2f eye) -> bool
        {
            float dx = playerCx - eye.x;
            float dy = playerCy - eye.y;

            float dist = std::sqrt(dx * dx + dy * dy);
            float angle = std::atan2(dy, dx);

            float step = 1.f;

            while (step < dist)
            {
                float fx = eye.x + std::cos(angle) * step;
                float fy = eye.y + std::sin(angle) * step;

                int x = (int)fx;
                int y = (int)fy;

                // out of bounds
                if (x < 0 || y < 0 || x >= GFX::MAP_W || y >= GFX::MAP_H)
                    return false;

                // wall hit
                if (Map::worldMap[y][x] != 0)
                    return false;

                step += 1.f;
            }

            return true;
        };
    return canSeePlayer(leftEye) || canSeePlayer(rightEye);





}

void EnemySystem::updatePatrolTarget(World* world, Entity* enemy) {
    
    auto* posEnemy = enemy->getComponent<PositionComponent>();
    auto* enemyComp = enemy->getComponent<EnemyComponent>();
    auto* velEnemy = enemy->getComponent<VelocityComponent>();
    

    // patrol -> set target
    Vector2f targetCoords = Map::pickCoordsRadius(3,  posEnemy->position).convertTo<float>();

    float dx = targetCoords.x - posEnemy->position.x;
    float dy = targetCoords.y - posEnemy->position.y;
    float angle = std::atan2(dy, dx);    
    float dist = std::sqrt(dx * dx + dy * dy);

    //set target
    enemyComp->target = { targetCoords, dist,angle };

    posEnemy->setAngle(angle);


    velEnemy->dx = std::cos(angle) * MovementConstants::ENEMY_SPEED;
    velEnemy->dy = std::sin(angle) * MovementConstants::ENEMY_SPEED;

    println("adsf");
    enemyComp->patroling = true;

}

void EnemySystem::reactToPlayer(World* world, Entity* enemy)
{

    Entity* player = scene->playerEntity;
    auto* posPlayer = player->getComponent<PositionComponent>();
    auto* posEnemy = enemy->getComponent<PositionComponent>();
    auto* enemyComp = enemy->getComponent<EnemyComponent>();
    auto* velEnemy = enemy->getComponent<VelocityComponent>();

    float enemyCx = posEnemy->position.x;
    float enemyCy = posEnemy->position.y;

    float playerCx = posPlayer->position.x;
    float playerCy = posPlayer->position.y;


    float dx = playerCx - enemyCx;
    float dy = playerCy - enemyCy;
    float dist = std::sqrt(dx * dx + dy * dy);
    float angle = std::atan2(dy, dx);


    if (enemyComp->seeTargetTimer > 0)
    {
        enemyComp->seeTargetTimer--;
    }
    else
    {
        //set target
        enemyComp->target = { posPlayer->position, dist,angle };
        enemyComp->aproachingPlayer = true;
        enemyComp->patroling = false;
        enemyComp->patroling = false;

        velEnemy->dx = std::cos(enemyComp->target.angle) * MovementConstants::ENEMY_SPEED;
        velEnemy->dy = std::sin(enemyComp->target.angle) * MovementConstants::ENEMY_SPEED;
        posEnemy->setAngle(0.f);
    }


}
void EnemySystem::attackPlayer(World* world, Entity* enemy) {
    auto* posEnemy = enemy->getComponent<PositionComponent>();
    auto* enemyComp = enemy->getComponent<EnemyComponent>();
    auto* velEnemy = enemy->getComponent<VelocityComponent>();

    switch (enemyComp->type)
    {
    case EnemyType::GUARD: {

        auto animCompSave = *enemy->getComponent<AnimationComponent>();
        enemy->addComponent<AnimationComponent>(std::vector<int>{48, 49, 50}, 10, true);
        enemy->getComponent<SpritesheetComponent>()->useAngle = false;


        enemy->getComponent<TimerComponent>()->addTimer(30, [=](Entity* enemy) {
            enemy->addComponent<AnimationComponent>(animCompSave);
            enemy->getComponent<SpritesheetComponent>()->useAngle = true;
            enemyComp->attacking = false;
            });

    }break;
    default:
        break;
    }

}
void EnemySystem::approachPlayer(World* world, Entity* enemy)
{
    auto* posEnemy = enemy->getComponent<PositionComponent>();
    auto* enemyComp = enemy->getComponent<EnemyComponent>();
    auto* velEnemy = enemy->getComponent<VelocityComponent>();
    const auto& target = enemyComp->target;
    if (enemyComp->attacking)// if in attackState
        return;
        
    static const float SHOOT_DIST = 2.f;
    if (target.dist <= SHOOT_DIST)
    {
        velEnemy->dx = 0.f;
        velEnemy->dy = 0.f;
        enemyComp->aproachingPlayer = false;
        enemyComp->attacking = true;
    }


}

void EnemySystem::update(World* world){
    for (Entity* entity : world->getEntities()) {
        if (!entity->hasComponent<EnemyComponent>() ||
            !entity->hasComponent<PositionComponent>() ||
            !entity->hasComponent<VelocityComponent>())
            continue;//not a valid enemy
        auto* posEnemy = entity->getComponent<PositionComponent>();
        auto* enemyComp = entity->getComponent<EnemyComponent>();
        auto* velEnemy = entity->getComponent<VelocityComponent>();

        
        if (seePlayer(entity))
        {
            
            if (!enemyComp->attacking)
            {
                reactToPlayer(world, entity);
                if (enemyComp->aproachingPlayer)
                {
                    approachPlayer(world, entity);
                }
            }
            else
            {
                attackPlayer(world, entity);
            }
        }
        else
        {
            enemyComp->seeTargetTimer = enemyComp->reactionTime;
            enemyComp->aproachingPlayer = false;
            enemyComp->attacking = false;
            if (enemyComp->patroling && reachedTarget(entity))
            {
                enemyComp->patroling = false;
            }


            if (!enemyComp->patroling)
            {
                updatePatrolTarget(world, entity);
            }

        }
        
    }


}