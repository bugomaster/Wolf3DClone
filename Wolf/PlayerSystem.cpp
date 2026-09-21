#include "pch.hpp"
#include "PlayerSystem.hpp"
#include "Components.hpp"

#include "GFX.hpp"
#include "Input.hpp"
#include "AssetsLoads.hpp"
#include "GameScene.hpp"
#include "Map.hpp"
#include "MapSystem.hpp"
#include "RayCastingSystem.hpp"
#include "SoundManager.hpp"
#define DEV
/*
TODO:
GENERAL:

cutscenes between levels
menu
strip the real font


ENEMIES:
guard
dog
ss
mutant
officer
first bos

*/
/*
SOUND CHANNELS:
0 - 7
2 doors
3 moving walls




*/



PlayerSystem::PlayerSystem(GameScene* gameScene) :
    gameScene(gameScene)
{

}
void PlayerSystem::onAddedToWorld(World* world)
{
    initPlayer();
}
void PlayerSystem::update(World* world) {

    if (!playerEntity)
        return;
    if (!playerEntity->hasComponent<PlayerComponent>())
        return;

    updateCamera();
    updateInput();
    updateMovement();
    updateState();
    updateShooting();
    updateDoorOpen();
    Vector2i mapPos = playerEntity->getComponent<PositionComponent>()->position.to<int>();


    updatePos(world);
    Vector2i after = playerEntity->getComponent<PositionComponent>()->position.to<int>();
    if (mapPos != after)
    {
        world->find<InitEntityComponenet>([after](Entity* entity) {
            auto dis = std::hypot(
                after.x - entity->getComponent<PositionComponent>()->position.x,
                after.y - entity->getComponent<PositionComponent>()->position.y);
            auto* initcomp = entity->getComponent<InitEntityComponenet>();
            if (dis < initcomp->range)
            {
                initcomp->initFunc(entity);
                entity->removeComponent<InitEntityComponenet>();
            }

        });
    }
    // update user view anims
    




}



void PlayerSystem::initPlayer() {
    Entity* player = gameScene->world->createImmiditeEntity();
    this->playerEntity = player;
    gameScene->playerEntity = player;

    player->addComponent<PlayerComponent>()->data.weapon = PlayerData::Weapon::PISTOL;
    player->addComponent<InputComponent>();
    player->addComponent<TimerComponent>();
    player->addComponent<AnimationComponent>();
    player->addComponent<TextureComponent>(g_assets.weapons.texture);
    player->addComponent<SpritesheetComponent>(SPRSHEET_DATA::WEAPONS, 25);
    player->addComponent<VelocityComponent>(0.f, 0.f, 0.f);
    player->addComponent<PositionComponent>
        (gameScene->levelData.playerStart, GFX::PLAYER_RADIUS)
        ->setAngle(gameScene->levelData.cameraStart.x/180.f*GFX::PI);

    //face expression
    player->addComponent<RepeatedTimerComponent>(200, [](Entity* player)
    {
        auto* playerComp = player->getComponent<PlayerComponent>();
        playerComp->faceExpression = 1;
        player->getComponent<TimerComponent>()->addTimer(40,[](Entity* player) {
            auto* playerComp = player->getComponent<PlayerComponent>();
            playerComp->faceExpression = 2;
            player->getComponent<TimerComponent>()->addTimer(40, [](Entity* player) {
                auto* playerComp = player->getComponent<PlayerComponent>();
                playerComp->faceExpression = 0;
            });

        });
    });
}



void PlayerSystem::updateCamera(){}



void PlayerSystem::updateInput() {
    auto* inputComp = playerEntity->getComponent<InputComponent>();
    if (!inputComp->isActive())
        return;



    inputComp->reset();
    auto* playerComp = playerEntity->getComponent<PlayerComponent>();
    if (playerComp->data.health < 1)
    {
        //todo disable death here!
    #ifndef DEV
            this->gameScene->setFinished(true);
            this->gameScene->setPlayerDead(true);
    #endif // DEV

    }
    if (gameScene->getInput()->held(SDL_SCANCODE_LEFT))
        inputComp->left = true;
    if (gameScene->getInput()->held(SDL_SCANCODE_RIGHT))
        inputComp->right = true;
    if (gameScene->getInput()->held(SDL_SCANCODE_DOWN))
        inputComp->down = true;
    if (gameScene->getInput()->held(SDL_SCANCODE_UP))
        inputComp->up = true;
    if (gameScene->getInput()->held(SDL_SCANCODE_W))
        inputComp->w = true;
    if (gameScene->getInput()->held(SDL_SCANCODE_S))
        inputComp->s = true;
    if (gameScene->getInput()->pressed(SDL_SCANCODE_E))
        inputComp->e = true;
    if (gameScene->getInput()->pressed(SDL_SCANCODE_ESCAPE)) 
        this->gameScene->setFinished(true);
    if (gameScene->getInput()->mousePressed(SDL_BUTTON_LEFT))
        inputComp->leftButton = true;

    {
        //if (gameScene->getInput()->pressed(SDL_SCANCODE_P))
        //{
        //    auto* posComp = playerEntity->getComponent<PositionComponent>();
        //    println(posComp->position, true);
        //}
    }

}


void PlayerSystem::updateDoorOpen() {
    auto* inputComp = playerEntity->getComponent<InputComponent>();
    auto* playerComp = playerEntity->getComponent<PlayerComponent>();
    auto mid = RayCastingSystem::middleRay;
    Entity* midEntity = this->gameScene->world->getEntity(mid.entityID);
    if (midEntity && mid.rayHit.dist < 3.f)// middle ray -> points to object 
    {
        Entity* wall = gameScene->world->getEntity(MapSystem::gridObjectsMap[mid.rayHit.mapCoords.y][mid.rayHit.mapCoords.x]);
        if (!wall)
            return;
        if (wall->hasComponent<SecretWallComponent>())
        {
            auto* wallComp = wall->getComponent<SecretWallComponent>();
            if (mid.rayHit.dist < 0.5f && !wallComp->moving && !wallComp->reached)
            {
                wallComp->moving = true;
                auto* wallComp = wall->getComponent<SecretWallComponent>();
                auto* velDoor = wall->getComponent<VelocityComponent>();

                velDoor->dx = wallComp->moveDir.x * 0.01f;
                velDoor->dy = wallComp->moveDir.y * 0.01f;

                this->gameScene->getAudio()->playSound("wallPush", 100, 3);

                wall->addComponent<WaitUntilComponent>([](Entity* wall)
                {
                    auto* velDoor = wall->getComponent<VelocityComponent>();
                    return (velDoor->dx == 0.f && velDoor->dy == 0.f);
                },
                [this, wallComp](Entity* wall)
                {
                    wallComp->reached = true;
                    this->gameScene->getAudio()->stopSound(3);
                    this->gameScene->getAudio()->playSound("wallPushEnd", 1, 3);
                    wallComp->moving = false;
                    auto* posWall = wall->getComponent<PositionComponent>();
                    Vector2i currentPosI = { (int)posWall->position.x , (int)posWall->position.y };
                    Vector2i prevPosI = wallComp->prevMapCoord;

                    if (prevPosI != currentPosI) {
                        //gridObjectsMap switch
                        {
                            const auto valSwitch = MapSystem::gridObjectsMap[prevPosI.y][prevPosI.x];
                            MapSystem::gridObjectsMap[prevPosI.y][prevPosI.x] =
                                MapSystem::gridObjectsMap[currentPosI.y][currentPosI.x];
                            MapSystem::gridObjectsMap[currentPosI.y][currentPosI.x] = valSwitch;
                        }
                        wallComp->prevMapCoord = currentPosI;
                    }
                }
                );

            }

        }
        else if (inputComp->e && wall->hasComponent<DoorComponent>())//open door
        {
            auto* doorComp = wall->getComponent<DoorComponent>();
            if (!doorComp->open) {
                doorComp->opening = true;
                this->gameScene->getAudio()->playSound("doorOpen", 0, 7);
            }
        }
        else if (inputComp->e && wall->hasComponent<LockGateComponent>() && mid.rayHit.dist < 1.5f)
        {
            auto* lockComp = wall->getComponent<LockGateComponent>();
            if (!lockComp->on &&
                ((lockComp->keyID == 1 && playerComp->key1) ||
                (lockComp->keyID == 2 && playerComp->key2) ))
            {
                lockComp->on = true;
                wall->getComponent<RectFacesComponent>()->faceIDs.at(0) = 43;// open texture
                
            }
        }
        else if (inputComp->e && wall->hasComponent<EndGateComponent>() && mid.rayHit.dist < 1.5f)
        {
            wall->getComponent<SpritesheetComponent>()->frameID = 61;
            wall->addComponent<VelocityComponent>(-0.01f, 0.f);
            playerEntity->getComponent<TimerComponent>()->addTimer(100, [this](Entity* player) {
                this->gameScene->setFinished(true);
                this->gameScene->finishedLevel = true;
            });
        }
    }

}
void PlayerSystem::updateShooting() {
    auto* playerComp = playerEntity->getComponent<PlayerComponent>();
    auto* inputComp = playerEntity->getComponent<InputComponent>();
    if (inputComp->leftButton)
    {
        if(!playerComp->shoot && playerComp->data.ammo > 0)
        {
            playerComp->shoot = true;
            this->gameScene->world->find<EnemyComponent>([=](Entity* enemy)
            {
                
                Vector2f posEnemy = enemy->getComponent<PositionComponent>()->position;
                Vector2f posPlayer = playerEntity->getComponent<PositionComponent>()->position;
                float dx = posPlayer.x - posEnemy.x;
                float dy = posPlayer.y - posEnemy.y;
                float dis = std::hypot(dx, dy);
                if (dis < 5)
                {
                    enemy->getComponent<EnemyComponent>()->hearShot = true;// i shot . u guys should hear it
                }
                
                
            });
            playerComp->data.ammo--;
            // 10 frame cooldown between each shot
            playerEntity->getComponent<TimerComponent>()->addTimer
            (10, [=](Entity* playerEntity)
            {
                playerComp->shoot = false;
            });

            int animTicksTimer = 1;
            int weaponID;
            //get weaponID
            {
                switch (playerComp->data.weapon)
                {
                case PlayerData::Weapon::PISTOL:
                {
                    weaponID = 5;
                }break;
                default:
                    break;
                }
            }
            std::vector<int> shootAnimFrameIDs =
            { weaponID, weaponID + 1, weaponID + 2, weaponID + 3, weaponID + 4, weaponID };
            playerEntity->getComponent<AnimationComponent>()->addAnim(AnimCompData{shootAnimFrameIDs, animTicksTimer, false });
            gameScene->getAudio()->playSound("gunfire");


            // if hit enemy
            Entity* midEntity = this->gameScene->world->getEntity(RayCastingSystem::middleRay.entityID);
            if (midEntity)
            {
                auto* enemyComp = midEntity->getComponent<EnemyComponent>();
                if (enemyComp && RayCastingSystem::middleRay.rayHit.dist < GFX::MAX_SHOOT_RANGE)
                {
                    enemyComp->lives--;
                    if (enemyComp->lives > 0)
                    {
                        Entity* bloodEntity = this->gameScene->world->createEntity();
                        bloodEntity->addComponent<PositionComponent>(midEntity->getComponent<PositionComponent>()->position);
                        bloodEntity->addComponent<SpritesheetComponent>(SPRSHEET_DATA::BLOOD, 0);
                        bloodEntity->addComponent<AnimationComponent>();
                        bloodEntity->getComponent<AnimationComponent>()->addAnim(AnimCompData{ shuffleVtrInt({0,1,2}), 8, false });
                        bloodEntity->addComponent<TextureComponent>(g_assets.bloodTMap.texture);
                        bloodEntity->addComponent<RayCastDotObjectComponent>(false);
                        bloodEntity->addComponent<DestroyDelayComponent>(24);


                        if (enemyComp->type == EnemyType::GUARD)//hit by bullet anim
                        {
                            midEntity->getComponent<AnimationComponent>()->addFirst(AnimCompData{ std::vector<int>{47}, 10, false });
                        }

                    }
                }
            }

        }

    }
}
void PlayerSystem::updateMovement() {

    const auto* posPlayer = playerEntity->getComponent<PositionComponent>();
    const auto* inputComp = playerEntity->getComponent<InputComponent>();
    

    auto& yScreenOffset = gameScene->yScreenOffset;
    auto* velocity = playerEntity->getComponent<VelocityComponent>();

    static const float moveSpeed = 0.2f;
    static const float rotSpeed = 0.05f;
    static const float zAxisSpeed = moveSpeed * 200.f;

    // Rotation
    static const float mouseSensitivity = 0.0045f;

    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    if (mouseX < GFX::SCREEN_WIDTH && mouseX > 0 &&
        mouseY < GFX::SCREEN_HEIGHT && mouseY > 0)
    {
        int centerX = GFX::SCREEN_WIDTH / 2;
        int centerY = GFX::SCREEN_HEIGHT / 2;

        int mouseDeltaX = mouseX - centerX;

        velocity->dAngle = mouseDeltaX * mouseSensitivity;
        //
        float mouseDeltaY = (float)mouseY - (float)centerY;

        


        yScreenOffset += mouseDeltaY * -2.f;
        // Wrap

        static const float limit = GFX::SCREEN_HEIGHT / 2.0f;
        if (yScreenOffset < -limit)
            yScreenOffset = -limit;
        else if (yScreenOffset > limit)
            yScreenOffset = limit;



        // Move mouse back to center
        SDL_WarpMouseInWindow(nullptr, centerX, GFX::SCREEN_HEIGHT / 2);


    }

    // Keyboard rotation (optional)
    if (inputComp->left)
        velocity->dAngle = -0.05f;

    if (inputComp->right)
        velocity->dAngle = 0.05f;





    float dx = 0.0f;
    float dy = 0.0f;
    // Screen offset
    if (inputComp->down)
    {
        yScreenOffset = std::max(yScreenOffset - zAxisSpeed, -GFX::SCREEN_HEIGHT / 2.0f);
    }

    if (inputComp->up)
    {
        yScreenOffset = std::min(yScreenOffset + zAxisSpeed, GFX::SCREEN_HEIGHT / 2.0f);
    }


    if (inputComp->w)
    {
        dx += cosf(posPlayer->getAngle()) * moveSpeed;
        dy += sinf(posPlayer->getAngle()) * moveSpeed;
    }

    if (inputComp->s)
    {
        dx -= cosf(posPlayer->getAngle()) * moveSpeed;
        dy -= sinf(posPlayer->getAngle()) * moveSpeed;
    }
    velocity->dx = dx;
    velocity->dy = dy;

}


void PlayerSystem::checkCollectibleCollision(World* world , MoveData& moveData)
{
    
    auto* playerComp = playerEntity->getComponent<PlayerComponent>();

    for (const auto& col : moveData.collisions)
    {
        if (col.entity->hasComponent<CollectibleComponent>())
        {
            //todo
            // collect
            Collectible typeCollectible = col.entity->getComponent<CollectibleComponent>()->type;
            switch (typeCollectible)
            {
            case Collectible::AMMO: {
                gameScene->getAudio()->playSound("ammo");
                playerComp->data.ammo += 10;
            }break;
            case Collectible::GOLDBOX: {
                gameScene->getAudio()->playSound("ammo");
                playerComp->data.points += 100;
            }break;
            case Collectible::TROPHIE: {
                gameScene->getAudio()->playSound("ammo");
                playerComp->data.points += 100;
            }break;
            case Collectible::MEAL:
            case Collectible::MEATBALLS:
            {
                if (playerComp->data.health >= 100)
                    continue;

                gameScene->getAudio()->playSound("ammo");
                playerComp->data.health += 20;
            }break;
            case Collectible::BLUEKEY:
            {
                gameScene->getAudio()->playSound("ammo");
                playerComp->key1 = true;
            }break;
            case Collectible::GOLDKEY:
            {
                gameScene->getAudio()->playSound("ammo");
                playerComp->key2 = true;
            }break;

            default:
                break;
            }
            if (playerComp->data.health > 100)
                playerComp->data.health = 100;

            world->destroyEntity(col.entity);

            return;
        }
    }
}
void PlayerSystem::updateState() {


}
void PlayerSystem::updatePos(World* world) {

    auto* velPlayer = playerEntity->getComponent<VelocityComponent>();
    auto* posPlayer = playerEntity->getComponent<PositionComponent>();



    MoveData moveData = Map::movePosWithCol(world, playerEntity);
    checkCollectibleCollision(world, moveData);
    Map::applyMoveData(moveData, playerEntity);



    posPlayer->changeAngle(velPlayer->dAngle);




    velPlayer->reset();

}

//


//


//





