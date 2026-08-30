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
    Entity* player = gameScene->world->createEntity();
    this->playerEntity = player;
    gameScene->playerEntity = player;

    //todo init player
    player->addComponent<PlayerComponent>()->weapon = PlayerComponent::Weapon::PISTOL;
    player->addComponent<InputComponent>();
    player->addComponent<TimerComponent>();
    player->addComponent<TextureComponent>(g_assets.weapons.texture);
    player->addComponent<SpritesheetComponent>(SPRSHEET_DATA::WEAPONS, 25);
    player->addComponent<VelocityComponent>(0.f, 0.f, 0.f);
    player->addComponent<PositionComponent>(Vector2f{2.f, 2.f}, GFX::PLAYER_RADIUS);

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
    if (gameScene->getInput()->held(SDL_SCANCODE_R))
        gameScene->setFinished(true);
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
    if (gameScene->getInput()->held(SDL_SCANCODE_E))
        inputComp->e = true;
    if (gameScene->getInput()->mousePressed(SDL_BUTTON_LEFT))
        inputComp->space = true;

}


void PlayerSystem::updateDoorOpen() {
    auto* inputComp = playerEntity->getComponent<InputComponent>();
    if (inputComp->e)//open door
    {
        auto mid = RayCastingSystem::middleRay;
        if (mid.entity && mid.rayHit.dist < 3.f)// middle ray -> points to object 
        {
            Entity* door = MapSystem::gridObjectsMap[mid.rayHit.mapCoords.y][mid.rayHit.mapCoords.x];
            if (door)
            {
                auto* doorComp = door->getComponent<DoorComponent>();
                if (doorComp && !doorComp->open)
                {
                    doorComp->opening = true;
                }
            }

        }
    }

}
void PlayerSystem::updateShooting() {
    auto* playerComp = playerEntity->getComponent<PlayerComponent>();
    auto* inputComp = playerEntity->getComponent<InputComponent>();
    if (inputComp->space)
    {
        if(!playerComp->shoot && playerComp->ammo > 0)
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
                    enemy->getComponent<EnemyComponent>()->hearShot = true;
                }
                
                
            });
            playerComp->ammo--;
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
                switch (playerComp->weapon)
                {
                case PlayerComponent::Weapon::PISTOL:
                {
                    weaponID = 5;
                }break;
                default:
                    break;
                }
            }
            std::vector<int> shootAnimFrameIDs =
            { weaponID, weaponID + 1, weaponID + 2, weaponID + 3, weaponID + 4, weaponID };
            playerEntity->addComponent<AnimationComponent>(shootAnimFrameIDs, animTicksTimer, false);
            gameScene->getAudio()->playSound("gunfire");


            // if hit enemy
            auto* enemyEntity = RayCastingSystem::middleRay.entity;
            auto* enemyComp = enemyEntity->getComponent<EnemyComponent>();
            if (enemyComp && RayCastingSystem::middleRay.rayHit.dist < GFX::MAX_SHOOT_RANGE)
            {
                enemyComp->lives--;
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

            // collect
            Collectible typeCollectible = col.entity->getComponent<CollectibleComponent>()->type;
            switch (typeCollectible)
            {
            case Collectible::AMMO: {
                gameScene->getAudio()->playSound("ammo");
                playerComp->ammo += 10;
            }break;
            default:
                break;
            }
            world->destroyEntity(col.entity);


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





