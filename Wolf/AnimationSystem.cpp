#include "pch.hpp"
#include "AnimationSystem.hpp"
#include "Components.hpp"

void AnimationSystem::update(World* world) {

    world->find<AnimationComponent, TextureComponent, SpritesheetComponent, PositionComponent>(
        [](Entity* entity) {
            auto* spritesheet = entity->getComponent<SpritesheetComponent>();


            auto* currentAnim = entity->getComponent<AnimationComponent>()->getAnim();
            auto* animation = entity->getComponent<AnimationComponent>();
            if (currentAnim)
            {
                currentAnim->frameTimer--;//every tick
                if (currentAnim->frameTimer < 0) {
                    currentAnim->frameTimer = currentAnim->ticksPerFrame;
                    currentAnim->currentFrame++;


                    if (currentAnim->currentFrame >= currentAnim->frameCount) {
                        currentAnim->currentFrame = 0;
                        if (!currentAnim->repeated)
                        {
                            animation->popAnim();
                            return;
                        }

                    }
                    spritesheet->frameID = currentAnim->frameIDS[currentAnim->currentFrame];
                }
            }
        });

}