#include "pch.hpp"
#include "AnimationSystem.hpp"
#include "Components.hpp"

void AnimationSystem::update(World* world) {

    world->find<AnimationComponent, TextureComponent, SpritesheetComponent, PositionComponent>(
        [](Entity* entity) {
            auto* animation = entity->getComponent<AnimationComponent>();
            auto* spritesheet = entity->getComponent<SpritesheetComponent>();
            animation->frameTimer--;//every tick
            if (animation->frameTimer < 0) {
                animation->frameTimer = animation->ticksPerFrame;
                animation->currentFrame++;


                if (animation->currentFrame >= animation->frameCount) {
                    animation->currentFrame = 0;
                    if (!animation->repeated)
                    {
                        entity->removeComponent<AnimationComponent>();
                        return;
                    }

                }
                spritesheet->frameID = animation->frameIDS[animation->currentFrame];
            }
        });

}