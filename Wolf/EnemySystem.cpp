#include "pch.hpp"
#include "EnemySystem.hpp"
#include "MapSystem.hpp"
#include "Components.hpp"
#include "GameScene.hpp"
#include "Map.hpp"
#include "AssetsLoads.hpp"
#include "SoundManager.hpp"


float heuristic(PathNode* a, PathNode* b)
{
    return (float)abs(a->x - b->x) +
        (float)abs(a->y - b->y);
}
std::vector<Vector2f> AStar(Vector2f start, Vector2f end)
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
            if (Map::wallMap[ny][nx] >= 1)
            {
                continue;
            }
            // SPECIAL TILES
            //if (Map::wallMap[ny][nx] < 0)
            //{
            //    if (Map::wallMap[ny][nx] == -1 ||
            //        Map::wallMap[ny][nx] == -2)
            //    {
            //        bool doorOpen = MapSystem::gridObjectsMap[ny][nx]->getComponent<DoorComponent>()->open;
            //        if (!doorOpen)
            //            continue;
            //    }
            //}


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
    Entity* player = scene->playerEntity;
    if (!player) return false;


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

            if (Map::wallMap[y][x] != 0)
            {
                // door -> maybe open
                if (Map::wallMap[y][x] == -1 || Map::wallMap[y][x] == -2)
                {
                    if (MapSystem::gridObjectsMap[y][x] &&
                        MapSystem::gridObjectsMap[y][x]->hasComponent<DoorComponent>()&&
                        MapSystem::gridObjectsMap[y][x]->getComponent<DoorComponent>()->open)
                    {
                        continue;// its okay its open door
                    }
                }
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

    comp->prevState = comp->currentState;
    comp->currentState = newState;

    if (newState && newState->onEnter)
        newState->onEnter(enemy);

    return newState;
}
EnemyState* EnemySystem::buildEnemyStates()
{
    freeEnemyStates(this->states);
    EnemyState* patrol = new EnemyState{ EnemyStateType::Patrol };
    EnemyState* patrolAlert = new EnemyState{ EnemyStateType::Alert };
    EnemyState* chase = new EnemyState{ EnemyStateType::Chase };
    EnemyState* attack = new EnemyState{ EnemyStateType::Attack };

    patrol->next = patrolAlert;

    patrolAlert->prev = patrol;
    patrolAlert->next = chase;

    chase->prev = patrolAlert;
    chase->next = attack;

    attack->prev = chase;


    const int MAX_DIS_TO_PLAYER = 20;
    const int MAX_SHOTS = 3;

    patrol->onEnter = [](Entity* enemy)
    {

        enemy->removeComponent<AnimationComponent>();
        enemy->getComponent<SpritesheetComponent>()->frameID = getRandomRange(0, 7);

        auto* velEnemy = enemy->getComponent<VelocityComponent>();
        auto* enemyComp = enemy->getComponent<EnemyComponent>();
        velEnemy->dx = 0.f;
        velEnemy->dy = 0.f;

    };
    patrol->onUpdate = [=](Entity* enemy)
    {
        auto* enemyComp = enemy->getComponent<EnemyComponent>();
        auto* velEnemy = enemy->getComponent<VelocityComponent>();
        auto* ePos = enemy->getComponent<PositionComponent>();
        auto* playerPos = scene->playerEntity->getComponent<PositionComponent>();
        const float distToPlayer = std::hypot(
            playerPos->position.x - ePos->position.x,
            playerPos->position.y - ePos->position.y);

        if (seePos(enemy, playerPos->position) && distToPlayer  < MAX_DIS_TO_PLAYER)
        {
            changeState(enemy, patrolAlert);
        }
        else if (enemyComp->hearShot)
        {
            enemyComp->hearShot = false;
            changeState(enemy, chase);
        }
    };
    patrol->onExit = [=](Entity* enemy)
    {
        
    };
    patrolAlert->onEnter = [=](Entity* enemy)
    {
        scene->getAudio()->playSound("halt");

        auto* enemyComp = enemy->getComponent<EnemyComponent>();
        enemyComp->seeTargetTimer = enemyComp->reactionTime;

    };
    patrolAlert->onUpdate = [=](Entity* enemy)
    {        
        const auto* playerPos = scene->playerEntity->getComponent<PositionComponent>();
        auto* enemyComp = enemy->getComponent<EnemyComponent>();
        auto* ePos = enemy->getComponent<PositionComponent>();

        float distToPlayer = std::hypot(
            playerPos->position.x - ePos->position.x, 
            playerPos->position.y - ePos->position.y);


        if (seePos(enemy, playerPos->position) && distToPlayer < MAX_DIS_TO_PLAYER)
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
            changeState(enemy, patrol);
        }
    };
    patrolAlert->onExit = [=](Entity* enemy) {
        

    };
    //
    
    chase->onEnter = [=](Entity* enemy) {
        auto* ePos = enemy->getComponent<PositionComponent>();
        auto* playerPos = scene->playerEntity->getComponent<PositionComponent>();
        auto* enemyComp = enemy->getComponent<EnemyComponent>();
        enemyComp->walkingStraight = getRandomRange(0, 1) == 1;
        enemy->addComponent<AnimationComponent>(std::vector<int>{8, 16, 24, 32}, 8);
        ePos->setAngle(0.f);

        enemy->getComponent<SpritesheetComponent>()->frameID = 8;





        enemyComp->path = {};
        enemyComp->iPath = 0;

        if (!seePos(enemy, playerPos->position))
        {
            enemyComp->path = AStar(ePos->position, playerPos->position);
            println("1");
            enemyComp->iPath = 0;
        }
        
        enemyComp->walkingTime = 0;
        enemyComp->dirAngle *= -1.f;
    };
    chase->onUpdate = [=](Entity* enemy)
    {
        const auto* playerPos = scene->playerEntity->getComponent<PositionComponent>();

        auto* enemyComp = enemy->getComponent<EnemyComponent>();
        auto* ePos = enemy->getComponent<PositionComponent>();
        auto* eVel = enemy->getComponent<VelocityComponent>();
        const bool seePlayer = seePos(enemy, playerPos->position);
        const float distToPlayer = std::hypot(
            playerPos->position.x - ePos->position.x,
            playerPos->position.y - ePos->position.y);

        float dx = playerPos->position.x - ePos->position.x;
        float dy = playerPos->position.y - ePos->position.y;

        int sizePath = (int)enemyComp->path.size();
        if (sizePath != 0) // there is a path
        {
            if (seePlayer)
            {
                enemyComp->path = {};
                enemyComp->iPath = 0;
                changeState(enemy, chase);
            }
            else
            {
                Vector2f target = 
                { enemyComp->path[enemyComp->iPath].x,
                  enemyComp->path[enemyComp->iPath].y};


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
                    if (!seePlayer)
                    {
                        enemyComp->path = AStar(ePos->position, playerPos->position);
                        println("2");
                        enemyComp->iPath = 0;

                    }

                }

            }


        }


        float angle = std::atan2(dy, dx);


        
        if (distToPlayer > MAX_DIS_TO_PLAYER)
        {
            changeState(enemy, patrol);
            return;

        }
        else if (seePlayer && distToPlayer < 2)
        {
            changeState(enemy, attack);
            return;
        }        
        else if(!seePlayer && enemyComp->path.size() == 0)// no path and dont see player
        {
            enemyComp->path = AStar(ePos->position, playerPos->position);
            println("3");
            enemyComp->iPath = 0;
        }



        if (sizePath == 0 && !enemyComp->walkingStraight)
        {//walks diagonaly
            if (enemyComp->angleOffset != 0.f)
            {
                auto* animCompEnemy = enemy->getComponent<AnimationComponent>();
                auto& frameId = animCompEnemy->frameIDS[animCompEnemy->currentFrame];
                if (((int)((dx + dy) / 1.4f)) % 2 == 0)
                {
                    angle -= enemyComp->angleOffset;
                    animCompEnemy->frameIDS = std::vector<int>{ 15, 23, 31, 39 };
                }
                else
                {
                    angle += enemyComp->angleOffset;
                    animCompEnemy->frameIDS = std::vector<int>{ 9, 17, 25, 33 };
                }
            }
        }
        eVel->dx = std::cos(angle) * MovementConstants::ENEMY_SPEED;
        eVel->dy = std::sin(angle) * MovementConstants::ENEMY_SPEED;



        enemyComp->walkingTime++;


        if (seePlayer && enemyComp->walkingTime > enemyComp->maxWalkingTime && distToPlayer < 8)
        {
            changeState(enemy, attack);
            return;
        }
        
    };
    chase->onExit = [=](Entity* enemy) {

    };
    

    attack->onEnter = [=](Entity* enemy) {
        attack->ticks = 0;
        auto* eVel = enemy->getComponent<VelocityComponent>();
        eVel->dx = 0.f;
        eVel->dy = 0.f;

        enemy->addComponent<AnimationComponent>(std::vector<int>{49, 50}, 16);


        auto* enemyComp = enemy->getComponent<EnemyComponent>();
        enemyComp->counterShots = 0;
    };
    attack->onUpdate = [=](Entity* enemy)
    {
        attack->ticks++;
        auto* enemyComp = enemy->getComponent<EnemyComponent>();
        auto* animComp = enemy->getComponent<AnimationComponent>();
        if (attack->ticks % (animComp->ticksPerFrame * animComp->frameCount) == 0)
        {
            enemyComp->counterShots++;
            attack->dealDamage = enemyComp->counterShots % (int)(MAX_SHOTS/1.5f);
        }


        {
            const auto* playerPos = scene->playerEntity->getComponent<PositionComponent>();
            auto* ePos = enemy->getComponent<PositionComponent>();

            const float distToPlayer = std::hypot(
                playerPos->position.x - ePos->position.x,
                playerPos->position.y - ePos->position.y);


            if (distToPlayer > MAX_DIS_TO_PLAYER)
            {
                changeState(enemy, patrol);
                return;

            }

        }
        const auto* playerPos = scene->playerEntity->getComponent<PositionComponent>();

        auto* ePos = enemy->getComponent<PositionComponent>();
        auto* eVel = enemy->getComponent<VelocityComponent>();
        
        if (!seePos(enemy, playerPos->position))
        {
            changeState(enemy, chase);
            return;
        }

        
        if (attack->dealDamage && getRandomRange(0,10) > 5)
        {
            attack->dealDamage = false;
            scene->playerEntity->getComponent<PlayerComponent>()->health -= 10;
        }

        if (enemyComp->counterShots > MAX_SHOTS)
        {
            changeState(enemy, chase);
            return;

        }

    };
    attack->onExit = [=](Entity* enemy) {


        
    };
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


void EnemySystem::onAddedToWorld(World* world) {
    EnemyState* states = buildEnemyStates();
    this->states = states;


    auto startPos = Vector2f{ 8.5f,16.5f };
    static const unsigned long enemiesCount = 10;
    bool inWall = false;
    for (size_t i = 0; i < enemiesCount; i++)
    {
        startPos.x = getRandomRange(2.f, 18.f);
        startPos.y = getRandomRange(2.f, 18.f);
        inWall = Map::wallMap[(int)(startPos.y)][(int)(startPos.x)] != 0;
        inWall &= Map::wallMap[(int)(startPos.y - GFX::ENEMY_RADIUS)][(int)(startPos.x)] != 0;
        inWall &= Map::wallMap[(int)(startPos.y + GFX::ENEMY_RADIUS)][(int)(startPos.x)] != 0;
        inWall &= Map::wallMap[(int)(startPos.y)][(int)(startPos.x + GFX::ENEMY_RADIUS)] != 0;
        inWall &= Map::wallMap[(int)(startPos.y)][(int)(startPos.x - GFX::ENEMY_RADIUS)] != 0;
        while (inWall)
        {
            inWall = Map::wallMap[(int)(startPos.y)][(int)(startPos.x)] != 0;
            inWall &= Map::wallMap[(int)(startPos.y - GFX::ENEMY_RADIUS)][(int)(startPos.x)] != 0;
            inWall &= Map::wallMap[(int)(startPos.y + GFX::ENEMY_RADIUS)][(int)(startPos.x)] != 0;
            inWall &= Map::wallMap[(int)(startPos.y)][(int)(startPos.x + GFX::ENEMY_RADIUS)] != 0;
            inWall &= Map::wallMap[(int)(startPos.y)][(int)(startPos.x - GFX::ENEMY_RADIUS)] != 0;

            startPos.x = getRandomRange(2.f, 18.f);
            startPos.y = getRandomRange(2.f, 18.f);
        }
        Entity* guard = world->createEntity();
        guard->addComponent<PositionComponent>(startPos, GFX::ENEMY_RADIUS);
        guard->addComponent<InitEntityComponenet>(10, [states](Entity* guard) {
            guard->addComponent<RayCastDotObjectComponent>();
            guard->addComponent<VelocityComponent>(0.f, 0.f);
            guard->addComponent<SpritesheetComponent>(SPRSHEET_DATA::GUARDTMAP, 0);
            guard->addComponent<AnimationComponent>(std::vector<int>{8, 16, 24, 32}, 8);
            guard->addComponent<TextureComponent>(g_assets.guard.texture);
            guard->addComponent<CantCollideWithComponent>(getComponentTypeID<EnemyComponent>());



            guard->addComponent<EnemyComponent>(EnemyType::GUARD, 20, 50, states);

            guard->addComponent<TimerComponent>();

            });



    }


}
void EnemySystem::update(World* world)
{

    
    for (Entity* enemy : world->getEntities())
    {
        // update movement
        if (!enemy->hasComponent<EnemyComponent>())
            continue;
        auto* enemyComp = enemy->getComponent<EnemyComponent>();
        if (enemy->hasComponent<PositionComponent>() && enemy->hasComponent<VelocityComponent>())
        {
            enemyComp->currentState->onUpdate(enemy);
        }
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
            MapSystem::createCollectibleEntity(world, worldPos, (int)drop);

            // Death anim
            {
                const auto& deathAnim = arrToVec(ANIMATIONS::GUARD_DIE);
                static const int animSpeed = 6;
                enemy->addComponent<AnimationComponent>(deathAnim, animSpeed, false);
            }
            int snd = getRandomRange(1, 4);
            switch (snd)
            {
            case 1: {
                scene->getAudio()->playSound("ahh");
            }break;
            case 2: {
                scene->getAudio()->playSound("mynaven");
            }break;
            default:
                break;
            }

            enemy->addComponent<NotCollideableComponent>();
            enemy->getComponent<SpritesheetComponent>()->sprSheetData = SPRSHEET_DATA::DEAD_GUARDTMAP;
            enemy->removeComponent<VelocityComponent>();
            enemy->removeComponent<EnemyComponent>();//corpse

        }
    }
}