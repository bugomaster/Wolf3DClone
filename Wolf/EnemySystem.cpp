#include "pch.hpp"
#include "EnemySystem.hpp"
#include "MapSystem.hpp"
#include "Components.hpp"
#include "GameScene.hpp"
#include "Map.hpp"
#include "AssetsLoads.hpp"
#include "SoundManager.hpp"
#include "RenderSystem.hpp"
//#define DISABLE_ENEMIES


float EnemySystem::heuristic(PathNode* a, PathNode* b)
{
    return (float)abs(a->x - b->x) +
        (float)abs(a->y - b->y);
}
std::vector<Vector2f> EnemySystem::AStar(Vector2f start, Vector2f end)
{
    println("Generating path...");

    std::unique_ptr<PathNode> nodesMap[GFX::MAP_H][GFX::MAP_W];

    auto getNode = [&](int x, int y) -> PathNode*
    {
        auto& ptr = nodesMap[y][x];

        if (!ptr)
        {
            ptr = std::make_unique<PathNode>(x, y);

            ptr->g = std::numeric_limits<float>::infinity();
            ptr->h = 0.0f;
            ptr->f = std::numeric_limits<float>::infinity();
            ptr->parent = nullptr;
        }

        return ptr.get();
    };


    PathNode* startNode = getNode((int)start.x, (int)start.y);
    PathNode* goalNode = getNode((int)end.x, (int)end.y);

    startNode->g = 0.0f;
    startNode->h = heuristic(startNode, goalNode);
    startNode->f = startNode->h;


    std::vector<PathNode*> open;
    std::vector<PathNode*> closed;

    open.push_back(startNode);


    while (!open.empty())
    {
        // find closest node to end
        auto currentIt = std::min_element(open.begin(),open.end(),
        [](PathNode* a, PathNode* b)
        {
            return a->f < b->f;
        });

        PathNode* current = *currentIt;


        //reached!!!
        if (current->x == goalNode->x && current->y == goalNode->y)
        {
            std::vector<Vector2f> path;


            while (current != nullptr)
            {
                path.push_back(
                    {   current->x + getRandomRange(0.2f, 0.8f), 
                        current->y + getRandomRange(0.2f, 0.8f)});
                current = current->parent;
            }


            std::reverse(path.begin(), path.end());

            return path;
        }

        //didnt reach with that node
        open.erase(currentIt);
        closed.push_back(current);

        // neighbours
        static const int directions[4][2] =
        {
            { 1, 0 },
            {-1, 0 },
            { 0, 1 },
            { 0,-1 }
        };

        for (const auto& dir : directions)
        {
            int nx = current->x + dir[0];
            int ny = current->y + dir[1];


            if (nx < 0 || ny < 0 ||nx >= GFX::MAP_W ||ny >= GFX::MAP_H)
                continue;
            if (this->gameScene->wallMap[ny][nx] >= 1)
            {
                continue;
            }


            PathNode* neighbour = getNode(nx, ny);


            if (std::find(closed.begin(), closed.end(), neighbour) != closed.end())
                continue;
            bool inOpen = std::find(open.begin(), open.end(), neighbour) != open.end();


            float newG = current->g + 1.0f;
            if (!inOpen || newG < neighbour->g)
            {
                // re-set the node if there is a shorter way to get to it
                neighbour->parent = current;
                neighbour->g = newG;
                neighbour->h = heuristic(neighbour, goalNode);
                neighbour->f = neighbour->g + neighbour->h;


                // if the node is newly created
                if (!inOpen)
                    open.push_back(neighbour);
            }
        }
    }


    return {};
}
bool EnemySystem::seePos(Entity* enemy, Vector2f pos)
{
    static const float eye2eyeDis = 0.01f;


    auto* enemyPos = enemy->getComponent<PositionComponent>();

    float ex = enemyPos->position.x;
    float ey = enemyPos->position.y;

    float px = pos.x;
    float py = pos.y;

    float offset = enemyPos->radius * eye2eyeDis;

    Vector2f eyes[2] = {
        {ex - offset, ey},
        {ex + offset, ey}
    };

    auto raycast = [&](Vector2f eye) -> bool
    {
        float dx = px - eye.x;
        float dy = py - eye.y;

        float dist = std::sqrt(dx * dx + dy * dy);
        float angle = std::atan2(dy, dx);

        for (float step = 0; step < dist; step += 0.01f)
        {
            int x = (int)(eye.x + std::cos(angle) * step);
            int y = (int)(eye.y + std::sin(angle) * step);

            if (x < 0 || y < 0 || x >= GFX::MAP_W || y >= GFX::MAP_H)
                return false;

            if (this->gameScene->wallMap[y][x] != 0)
            {
                return false;// a wall
            }
        }

        return true;
    };

    return raycast(eyes[0]) || raycast(eyes[1]);
}




EnemyState* EnemySystem::changeState(Entity* enemy, EnemyState* newState)
{
    auto* comp = enemy->getComponent<EnemyComponent>();
    if (!comp) return nullptr;

    if (comp->currentState && comp->currentState->onExit)
        comp->currentState->onExit(enemy);
    switch (newState->type)
    {
    case EnemyStateType::Patrol: {
        println("patrol", true);

    }        break;
    case EnemyStateType::Alert: {
        println("Alert", true);

    }        break;
    case EnemyStateType::Chase: {
        println("Chase", true);

    }        break;
    case EnemyStateType::Attack: {
        println("Attack", true);

    }        break;
    default:
        break;
    }

    comp->prevState = comp->currentState;
    comp->currentState = newState;

    if (newState && newState->onEnter)
        newState->onEnter(enemy);


    return newState;
}
EnemyState* EnemySystem::buildGuardStates()
{
    if (this->statesToFree.size() > 0)
        return this->statesToFree.at(0);


    EnemyState* patrol = new EnemyState{ EnemyStateType::Patrol };
    EnemyState* sawP_ReactTime = new EnemyState{ EnemyStateType::Alert };
    EnemyState* chase = new EnemyState{ EnemyStateType::Chase };
    EnemyState* attack = new EnemyState{ EnemyStateType::Attack };

    patrol->next = sawP_ReactTime;

    sawP_ReactTime->prev = patrol;
    sawP_ReactTime->next = chase;

    chase->prev = sawP_ReactTime;
    chase->next = attack;

    attack->prev = chase;



    //CONSTS

    const int SIGHT_RANGE = 20;
    const int  SHOOT_RANGE = 8;
    const int  MAX_WALKING_TIME = 50;

    const int MAX_SHOTS = 3;
    const int GUARD_DAMAGE = 5;
    const int HIT_PRECENTAGE = 50;
    patrol->onEnter = [](Entity* enemy)
    {

        enemy->getComponent<SpritesheetComponent>()->frameID = getRandomRange(0, 7);//the spawn -> standing texture face

        auto* velEnemy = enemy->getComponent<VelocityComponent>();
        auto* enemyComp = enemy->getComponent<EnemyComponent>();
        velEnemy->dx = 0.f;
        velEnemy->dy = 0.f;

        enemyComp->hearShot = false;

    };
    patrol->onUpdate = [=](Entity* enemy)
    {
        auto* enemyComp = enemy->getComponent<EnemyComponent>();
        auto* velEnemy = enemy->getComponent<VelocityComponent>();
        auto* ePos = enemy->getComponent<PositionComponent>();
        auto* playerPos = gameScene->playerEntity->getComponent<PositionComponent>();
        const float distToPlayer = std::hypot(
            playerPos->position.x - ePos->position.x,
            playerPos->position.y - ePos->position.y);

        if (distToPlayer < SIGHT_RANGE && seePos(enemy, playerPos->position))
        {
            changeState(enemy, sawP_ReactTime);
        }
        else if (enemyComp->hearShot)
        {
            changeState(enemy, chase);
        }
    };
    patrol->onExit = [=](Entity* enemy)
    {
        
    };
    sawP_ReactTime->onEnter = [=](Entity* enemy)
    {
        gameScene->getAudio()->playSound("halt");

        auto* enemyComp = enemy->getComponent<EnemyComponent>();
        enemyComp->seeTargetTimer = enemyComp->reactionTime;

    };
    sawP_ReactTime->onUpdate = [=](Entity* enemy)
    {        
        const auto* playerPos = gameScene->playerEntity->getComponent<PositionComponent>();
        auto* enemyComp = enemy->getComponent<EnemyComponent>();
        auto* ePos = enemy->getComponent<PositionComponent>();

        float distToPlayer = std::hypot(
            playerPos->position.x - ePos->position.x, 
            playerPos->position.y - ePos->position.y);

        if (distToPlayer > SIGHT_RANGE) 
        {
            changeState(enemy, patrol);
            return;
        }
        else if (seePos(enemy, playerPos->position))
        {
            if (enemyComp->seeTargetTimer > 0)
                enemyComp->seeTargetTimer--;
            else
            {
                changeState(enemy, attack);
            }
        }
        else
        {
            changeState(enemy, chase);
        }
    };
    sawP_ReactTime->onExit = [=](Entity* enemy) {
        

    };
    //
    
    chase->onEnter = [=](Entity* enemy) {
        auto* playerPos = gameScene->playerEntity->getComponent<PositionComponent>();



        auto* animComp = enemy->getComponent<AnimationComponent>();
        auto* ePos = enemy->getComponent<PositionComponent>();
        auto* enemyComp = enemy->getComponent<EnemyComponent>();
        animComp->addAnim(AnimCompData{ std::vector<int>{8, 16, 24, 32}, 8 , true});//walking anim


        enemyComp->zWalk = getRandomRange(0, 1) == 1;
        enemyComp->walkingTime = 0;
        enemyComp->angleOffset = 0.0f;
        enemy->getComponent<AnimationComponent>()->addAnim({ { 0, 8, 16, 24 }, 8, true });



        // every chase uses a new path
        enemyComp->path = {};
        enemyComp->iPath = 0;

        if (!seePos(enemy, playerPos->position))
        {
            enemyComp->path = AStar(ePos->position, playerPos->position);
            println("1");
        }
    };
    chase->onUpdate = [=](Entity* enemy)
    {
        const auto* playerPos = gameScene->playerEntity->getComponent<PositionComponent>();



        auto* enemyComp = enemy->getComponent<EnemyComponent>();
        auto* ePos = enemy->getComponent<PositionComponent>();
        auto* eVel = enemy->getComponent<VelocityComponent>();


        const float distToPlayer = std::hypot(
            playerPos->position.x - ePos->position.x,
            playerPos->position.y - ePos->position.y);
        if (distToPlayer > SIGHT_RANGE)
        {
            changeState(enemy, patrol);
            return;
        }



        float dx = playerPos->position.x - ePos->position.x;
        float dy = playerPos->position.y - ePos->position.y;
        // straight to player
        if (seePos(enemy, playerPos->position))
        {

            auto* animComp = enemy->getComponent<AnimationComponent>();
            auto currentAnim = animComp->getAnim();

            if (enemyComp->zWalk)
            {
                if (((int)((dx + dy) / 1.4f)) % 2 == 0)
                {
                    if (currentAnim->frameIDS[0] != 15)
                    {
                        enemyComp->angleOffset = getRandomRange(0.2f, 0.3f);
                        animComp->addAnim({ { 15, 23, 31, 39 }, 8, true });
                    }
                }
                else
                {
                    if (currentAnim->frameIDS[0] != 9)
                    {
                        enemyComp->angleOffset = -getRandomRange(0.2f, 0.3f);
                        animComp->addAnim({ { 9, 17, 25, 33 }, 8, true });
                    }
                }
            }


            if (distToPlayer < SHOOT_RANGE)
            {
                if (distToPlayer < 2)//too close
                {
                    changeState(enemy, attack);
                    return;
                }
                else if (enemyComp->walkingTime > MAX_WALKING_TIME * 4)
                {
                    changeState(enemy, attack);
                    return;
                }
                

            }
        }
        // path follow
        else 
        {
            int sizePath = (int)enemyComp->path.size();
            if (sizePath != 0)
            {
                Vector2f target =
                { enemyComp->path[enemyComp->iPath].x,
                  enemyComp->path[enemyComp->iPath].y };


                dx = target.x - ePos->position.x;
                dy = target.y - ePos->position.y;

                if (std::hypot(dx, dy) < 0.1f)
                {
                    enemyComp->iPath++;//reached the next node of the path
                }

                if (enemyComp->iPath >= sizePath)//finished the path
                {
                    //reset . recalculate path
                    enemyComp->path = {};
                    enemyComp->iPath = 0;
                    enemyComp->path = AStar(ePos->position, playerPos->position);
                    println("2");
                    enemyComp->iPath = 0;
                }

            }
            else
            {
                enemyComp->path = AStar(ePos->position, playerPos->position);

                println("3");
                enemyComp->iPath = 0;

            }
        }




        float angle = std::atan2(dx, dy);
        angle += enemyComp->angleOffset;
        eVel->dx = std::sin(angle) * MovementConstants::ENEMY_SPEED;
        eVel->dy = std::cos(angle) * MovementConstants::ENEMY_SPEED;
        enemyComp->walkingTime++;

    };
    chase->onExit = [=](Entity* enemy) {
    };
    

    attack->onEnter = [=](Entity* enemy) {

        auto* eVel = enemy->getComponent<VelocityComponent>();
        eVel->dx = 0.f;
        eVel->dy = 0.f;


        auto* enemyComp = enemy->getComponent<EnemyComponent>();
        enemyComp->guardData.counterShots = 0;
        if (enemyComp->prevState->type == EnemyStateType::Patrol)
            attack->ticks = -100;


        enemy->getComponent<AnimationComponent>()->addAnim(AnimCompData{ std::vector<int>{48}, 16, false });
        enemy->getComponent<AnimationComponent>()->addAnim(AnimCompData{ std::vector<int>{49, 50}, 16, true });
    };
    attack->onUpdate = [=](Entity* enemy)
    {
        static const int MAX_SHOOT_DIST = 10;
        attack->ticks++;
        if (attack->ticks < 0)
            return;


        //
        auto* enemyComp = enemy->getComponent<EnemyComponent>();
        auto* animComp = enemy->getComponent<AnimationComponent>()->getAnim();
        if (animComp)
        {
            if (attack->ticks % (animComp->ticksPerFrame * animComp->frameCount) == 0)
            {
                enemyComp->guardData.counterShots++;
                attack->dealDamage = enemyComp->guardData.counterShots % (int)(MAX_SHOTS / 1.5f);
            }
        }



        
        const auto* playerPos = gameScene->playerEntity->getComponent<PositionComponent>();
        auto* ePos = enemy->getComponent<PositionComponent>();
        auto* eVel = enemy->getComponent<VelocityComponent>();

        const float distToPlayer = std::hypot
                ( playerPos->position.x - ePos->position.x,
                playerPos->position.y - ePos->position.y);


        if (distToPlayer > SIGHT_RANGE)
        {
            changeState(enemy, patrol);
            return;
        }
        if (!seePos(enemy, playerPos->position) || distToPlayer > MAX_SHOOT_DIST)
        {
            changeState(enemy, chase);
            return;
        }

        
        if (attack->dealDamage && getRandomRange(0,100) < HIT_PRECENTAGE)
        {
            attack->dealDamage = false;
            int damage = GUARD_DAMAGE;
            switch (getRandomRange(1, 5))
            {
            case 1: {}break;
            case 2: {
                damage += 2;
            }break;
            case 3: {
                damage += 4;
            }break;
            case 4: {
                damage += 10;
            }break;
            case 5: {
                damage += 14;//critical hit
                RenderSystem::setFlash(20, SDL_Color{ 219, 22, 22 ,120 });

            }break;
            default:
                break;
            }
            gameScene->playerEntity->getComponent<PlayerComponent>()->data.health -= damage;


        }

        if (enemyComp->guardData.counterShots > MAX_SHOTS)
        {
            changeState(enemy, chase);

            return;

        }

    };
    attack->onExit = [=](Entity* enemy) {


    };
    this->statesToFree.push_back(patrol);
    return patrol; 
}
EnemyState* EnemySystem::buildHoundStates()
{
    if (this->statesToFree.size() > 1)
        return this->statesToFree.at(1);


    EnemyState* patrol = new EnemyState{ EnemyStateType::Patrol };
    EnemyState* sawP_ReactTime = new EnemyState{ EnemyStateType::Alert };
    EnemyState* chase = new EnemyState{ EnemyStateType::Chase };
    EnemyState* attack = new EnemyState{ EnemyStateType::Attack };

    patrol->next = sawP_ReactTime;

    sawP_ReactTime->prev = patrol;
    sawP_ReactTime->next = chase;

    chase->prev = sawP_ReactTime;
    chase->next = attack;

    attack->prev = chase;



    //CONSTS

    const int SIGHT_RANGE = 20;
    const int BITE_DIST = 1;
    const int HOUND_DAMAGE = 3;
    const int HIT_PRECENTAGE = 50;


    patrol->onEnter = [](Entity* enemy)
    {

        enemy->getComponent<SpritesheetComponent>()->frameID = getRandomRange(0, 7);//the spawn -> standing texture face

        auto* velEnemy = enemy->getComponent<VelocityComponent>();
        auto* enemyComp = enemy->getComponent<EnemyComponent>();
        velEnemy->dx = 0.f;
        velEnemy->dy = 0.f;

        enemyComp->hearShot = false;

    };
    patrol->onUpdate = [=](Entity* enemy)
    {
        auto* enemyComp = enemy->getComponent<EnemyComponent>();
        auto* velEnemy = enemy->getComponent<VelocityComponent>();
        auto* ePos = enemy->getComponent<PositionComponent>();
        auto* playerPos = gameScene->playerEntity->getComponent<PositionComponent>();
        const float distToPlayer = std::hypot(
            playerPos->position.x - ePos->position.x,
            playerPos->position.y - ePos->position.y);

        if (distToPlayer < SIGHT_RANGE && seePos(enemy, playerPos->position) )
        {
            changeState(enemy, sawP_ReactTime);
        }
        else if (enemyComp->hearShot)
        {
            changeState(enemy, chase);
        }
    };
    patrol->onExit = [=](Entity* enemy)
    {
        
    };
    sawP_ReactTime->onEnter = [=](Entity* enemy)
    {
        gameScene->getAudio()->playSound("bark");

        auto* enemyComp = enemy->getComponent<EnemyComponent>();
        enemyComp->seeTargetTimer = enemyComp->reactionTime;

    };
    sawP_ReactTime->onUpdate = [=](Entity* enemy)
    {        
        const auto* playerPos = gameScene->playerEntity->getComponent<PositionComponent>();
        auto* enemyComp = enemy->getComponent<EnemyComponent>();
        auto* ePos = enemy->getComponent<PositionComponent>();

        float distToPlayer = std::hypot(
            playerPos->position.x - ePos->position.x, 
            playerPos->position.y - ePos->position.y);

        if (distToPlayer > SIGHT_RANGE)
        {
            changeState(enemy, patrol);
            return;
        }
        else if (seePos(enemy, playerPos->position))
        {
            if (enemyComp->seeTargetTimer > 0)
                enemyComp->seeTargetTimer--;
            else
            {
                changeState(enemy, attack);
            }
        }
        else
        {
            changeState(enemy, chase);
        }
    };
    sawP_ReactTime->onExit = [=](Entity* enemy) {
        

    };
    //
    
    chase->onEnter = [=](Entity* enemy) {
        auto* playerPos = gameScene->playerEntity->getComponent<PositionComponent>();



        auto* animComp = enemy->getComponent<AnimationComponent>();
        auto* ePos = enemy->getComponent<PositionComponent>();
        auto* enemyComp = enemy->getComponent<EnemyComponent>();
        animComp->addAnim(AnimCompData{ std::vector<int>{0,8, 16, 24}, 8 , true});//walking anim


        enemyComp->walkingTime = 0;
        enemyComp->angleOffset = 0.0f;



        // every chase uses a new path
        enemyComp->path = {};
        enemyComp->iPath = 0;

        if (!seePos(enemy, playerPos->position))
        {
            enemyComp->path = AStar(ePos->position, playerPos->position);
            println("1");
        }
    };
    chase->onUpdate = [=](Entity* enemy)
    {

        const auto* playerPos = gameScene->playerEntity->getComponent<PositionComponent>();



        auto* enemyComp = enemy->getComponent<EnemyComponent>();
        auto* ePos = enemy->getComponent<PositionComponent>();
        auto* eVel = enemy->getComponent<VelocityComponent>();


        const float distToPlayer = std::hypot(
            playerPos->position.x - ePos->position.x,
            playerPos->position.y - ePos->position.y);
        if (distToPlayer > SIGHT_RANGE)
        {
            changeState(enemy, patrol);
            return;
        }



        float dx = playerPos->position.x - ePos->position.x;
        float dy = playerPos->position.y - ePos->position.y;
        // straight to player
        if (seePos(enemy, playerPos->position))
        {

            if (distToPlayer < BITE_DIST)
            {
                changeState(enemy, attack);
                return;

            }
            if (enemyComp->walkingTime % 400 == 0)
            {
                enemyComp->angleOffset = getRandomRange(-0.5f, 0.5f);
                auto* animComp = enemy->getComponent<AnimationComponent>();
                if (enemyComp->angleOffset > 0.f)
                    animComp->addAnim(AnimCompData{ {7,15,23, 31},8,true });
                else if (enemyComp->angleOffset < 0.f)
                    animComp->addAnim(AnimCompData{ {1,9,17, 25},8,true });
                else
                    animComp->addAnim(AnimCompData{ {0,8, 16, 24}, 8 , true });

            }
            else if (enemyComp->walkingTime % 200 == 0)
            {
                enemyComp->angleOffset = getRandomRange(-0.2f, 0.2f);
                auto* animComp = enemy->getComponent<AnimationComponent>();
                if (enemyComp->angleOffset > 0.f)
                    animComp->addAnim(AnimCompData{ {7,15,23, 31},8,true });
                else if (enemyComp->angleOffset < 0.f)
                    animComp->addAnim(AnimCompData{ {1,9,17, 25},8,true });
                else
                    animComp->addAnim(AnimCompData{ {0,8, 16, 24}, 8 , true });

            }
            else if(enemyComp->walkingTime % 100 == 0)
            {
                enemyComp->angleOffset = 0.f;
                auto* animComp = enemy->getComponent<AnimationComponent>();
                animComp->addAnim(AnimCompData{ {0,8, 16, 24}, 8 , true });

            }
        }
        // path follow
        else 
        {
            int sizePath = (int)enemyComp->path.size();
            if (sizePath != 0)
            {
                Vector2f target =
                { enemyComp->path[enemyComp->iPath].x,
                  enemyComp->path[enemyComp->iPath].y };


                dx = target.x - ePos->position.x;
                dy = target.y - ePos->position.y;

                if (std::hypot(dx, dy) < 0.1f)
                {
                    enemyComp->iPath++;//reached the next node of the path
                }

                if (enemyComp->iPath >= sizePath)//finished the path
                {
                    //reset . recalculate path
                    enemyComp->path = {};
                    enemyComp->iPath = 0;
                    enemyComp->path = AStar(ePos->position, playerPos->position);
                    println("2");
                    enemyComp->iPath = 0;
                }

            }
            else
            {
                enemyComp->path = AStar(ePos->position, playerPos->position);

                println("3");
                enemyComp->iPath = 0;

            }
        }




        float angle = std::atan2(dx, dy);
        angle += enemyComp->angleOffset;
        eVel->dx = std::sin(angle) * MovementConstants::HOUND_SPEED;
        eVel->dy = std::cos(angle) * MovementConstants::HOUND_SPEED;
        enemyComp->walkingTime++;

    };
    chase->onExit = [=](Entity* enemy) {
    };
    

    attack->onEnter = [=](Entity* enemy) {

        auto* eVel = enemy->getComponent<VelocityComponent>();
        eVel->dx = 0.f;
        eVel->dy = 0.f;


        auto* enemyComp = enemy->getComponent<EnemyComponent>();

        enemy->getComponent<SpritesheetComponent>()->frameID = 40;
        enemy->getComponent<AnimationComponent>()->addAnim(AnimCompData{ std::vector<int>{40, 41, 42}, 8, true });
    };
    attack->onUpdate = [=](Entity* enemy)
    {
        attack->ticks++;
        if (attack->ticks < 0)
            return;


        //
        auto* enemyComp = enemy->getComponent<EnemyComponent>();
        auto* animComp = enemy->getComponent<AnimationComponent>()->getAnim();
        if (animComp)
        {
            if (attack->ticks % (animComp->ticksPerFrame * animComp->frameCount) == 0)
            {
                attack->dealDamage = true;//bite
            }
        }



        
        const auto* playerPos = gameScene->playerEntity->getComponent<PositionComponent>();
        auto* ePos = enemy->getComponent<PositionComponent>();
        auto* eVel = enemy->getComponent<VelocityComponent>();

        const float distToPlayer = std::hypot
                ( playerPos->position.x - ePos->position.x,
                playerPos->position.y - ePos->position.y);


        if (distToPlayer > SIGHT_RANGE)//out of sight
        {
            changeState(enemy, patrol);
            return;
        }
        if (!seePos(enemy, playerPos->position) || distToPlayer > BITE_DIST)
        {
            changeState(enemy, chase);
            return;
        }

        
        if (attack->dealDamage && getRandomRange(0,100) < HIT_PRECENTAGE)
        {
            attack->dealDamage = false;
            int damage = HOUND_DAMAGE;
            switch (getRandomRange(1, 5))
            {
            case 1: {}break;
            case 2: {
                damage += 2;
            }break;
            case 3: {
                damage += 4;
            }break;
            case 4: {
                damage += 10;
            }break;
            case 5: {
                damage += 14;//critical hit
                RenderSystem::setFlash(20, SDL_Color{ 219, 22, 22 ,120 });

            }break;
            default:
                break;
            }
            gameScene->playerEntity->getComponent<PlayerComponent>()->data.health -= damage;

        
        }

    };
    attack->onExit = [=](Entity* enemy) {


    };

    this->statesToFree.push_back(patrol);

    return patrol; 
}
void EnemySystem::freeEnemyStates(EnemyState* first)
{
    if (!first)
        return;
    std::vector<EnemyState*> ptrs = {};
    while (first->next)
    {
        ptrs.push_back(first);
        first = first->next;
    }

    for (EnemyState* state : ptrs)
    {
        delete state;
    }
}

EnemySystem::EnemySystem(GameScene* gameScene) :
gameScene(gameScene)
{
}
void EnemySystem::onAddedToWorld(World* world)
{


#ifndef  DISABLE_ENEMIES
    EnemyState* guardStates = buildGuardStates();
    EnemyState* houndStates = buildHoundStates();

    for (const Vector2f& gPos : this->gameScene->levelData.guards)
    {
        Entity* guard = world->createEntity();
        guard->addComponent<PositionComponent>(gPos, GFX::ENEMY_RADIUS);
        guard->addComponent<InitEntityComponenet>(10, [guardStates](Entity* guard) {
            guard->addComponent<RayCastDotObjectComponent>();
            guard->addComponent<VelocityComponent>(0.f, 0.f);
            guard->addComponent<SpritesheetComponent>(SPRSHEET_DATA::GUARDTMAP, 0);
            guard->addComponent<AnimationComponent>();
            guard->addComponent<TextureComponent>(g_assets.guard.texture);
            //guard->addComponent<CantCollideWithComponent>(getComponentTypeID<EnemyComponent>());



            guard->addComponent<EnemyComponent>(EnemyType::GUARD, 20, guardStates);

            guard->addComponent<TimerComponent>();

        });



    }
    for (const Vector2f& dPos : this->gameScene->levelData.hounds)
    {

        Entity* hound = world->createEntity();
        hound->addComponent<PositionComponent>(dPos, GFX::ENEMY_RADIUS);
        hound->addComponent<InitEntityComponenet>(10, [houndStates](Entity* hound) {
            hound->addComponent<RayCastDotObjectComponent>();
            hound->addComponent<VelocityComponent>(0.f, 0.f);
            hound->addComponent<SpritesheetComponent>(SPRSHEET_DATA::HOUNDTMAP, 22);
            hound->addComponent<AnimationComponent>();
            hound->addComponent<TextureComponent>(g_assets.hound.texture);
            //hound->addComponent<CantCollideWithComponent>(getComponentTypeID<EnemyComponent>());



            hound->addComponent<EnemyComponent>(EnemyType::HOUND, 20, houndStates);

            hound->addComponent<TimerComponent>();

        });



    }

#endif //  



}
void EnemySystem::update(World* world)
{

    
    world->find<EnemyComponent>([this](Entity* enemy) {
        auto* enemyComp = enemy->getComponent<EnemyComponent>();
        if (enemy->hasComponent<PositionComponent>() && enemy->hasComponent<VelocityComponent>())
        {
            enemyComp->currentState->onUpdate(enemy);
        }
        // KILL ENEMY
        if (enemyComp->lives <= 0 || enemy->hasComponent<KillEnemyComponent>())
        {

            Collectible drop;
            // Get the drop
            {
                if (enemy->hasComponent<KillEnemyComponent>())
                {
                    drop = enemy->getComponent<KillEnemyComponent>()->drop;
                    enemy->removeComponent<KillEnemyComponent>();
                }
                else
                {
                    drop = enemy->getComponent<EnemyComponent>()->drop;
                }
            }

            const auto& worldPos = enemy->getComponent<PositionComponent>()->position;
            MapSystem::createCollectibleEntity(this->gameScene->world, worldPos, drop);
            auto* sprSheetComp = enemy->getComponent<SpritesheetComponent>();
            // Death anim
            {
                std::vector<int> deathAnim;
                switch (enemyComp->type)
                {
                case EnemyType::GUARD:
                {
                    deathAnim = arrToVec(ANIMATIONS::GUARD_DIE);
                    gameScene->getAudio()->playSound("ahh");
                    sprSheetComp->sprSheetData = SPRSHEET_DATA::DEAD_GUARDTMAP;
                } break;
                case EnemyType::HOUND:
                {
                    deathAnim = arrToVec(ANIMATIONS::HOUND_DIE);
                    gameScene->getAudio()->playSound("bark");
                    sprSheetComp->sprSheetData = SPRSHEET_DATA::DEAD_HOUNDTMAP;

                } break;
                default:
                    break;
                }

                static const int animSpeed = 6;
                enemy->getComponent<AnimationComponent>()->addAnim(AnimCompData{ deathAnim, animSpeed, false });
            }


            enemy->addComponent<NotCollideableComponent>();
            enemy->removeComponent<VelocityComponent>();
            enemy->removeComponent<EnemyComponent>();//corpse

        }

        
    });
}