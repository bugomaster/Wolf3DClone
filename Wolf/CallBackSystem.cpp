#include "pch.hpp"
#include "CallBackSystem.hpp"
#include "Components.hpp"

#include "Map.hpp"

CallBackSystem::CallBackSystem(GameScene* scene) :
    scene(scene)
{

}
void CallBackSystem::update(World* world)
{


    world->find<DestroyDelayComponent>([world](Entity* entity)
    {
            auto* descomp = entity->getComponent<DestroyDelayComponent>();
            descomp->accFrames++;

            if (descomp->accFrames >= descomp->frames)
            {
                world->destroyEntity(entity);
            }
    });
    world->find<TimerComponent>([](Entity* entity)
        {
            auto* timerComp = entity->getComponent<TimerComponent>();
            auto& timers = timerComp->timers;

            for (size_t i = 0; i < timers.size(); )
            {
                auto& timer = timers[i];
                timer.accFrames++;
                if (timer.accFrames >= timer.frames)
                {
                    auto callback = std::move(timer.onFinish);
                    callback(entity);

                    timers.erase(timers.begin() + i);
                }
                else
                    ++i;
            }
        });
    world->find<RepeatedTimerComponent>([](Entity* entity)
        {
            auto* timerComp = entity->getComponent<RepeatedTimerComponent>();
            timerComp->accFrames++;
            if (timerComp->accFrames >= timerComp->frames)
            {
                timerComp->accFrames = 0;
                timerComp->onFinish(entity);
            }
        });
    world->find<RepeatedTimerNTimesComponent>([](Entity* entity)
        {
            auto* timerComp = entity->getComponent<RepeatedTimerNTimesComponent>();
            timerComp->accFrames++;
            if (timerComp->accFrames >= timerComp->frameGap)
            {
                timerComp->accFrames = 0;
                timerComp->onFinish(entity);
                timerComp->counterTimes++;
                if (timerComp->counterTimes == timerComp->times)
                {
                    entity->removeComponent<RepeatedTimerNTimesComponent>();
                }
            }
        });
    world->find<WaitUntilComponent>([](Entity* entity)
        {

            auto* waitUntilComponent = entity->getComponent<WaitUntilComponent>();
            if (waitUntilComponent->condition(entity))
            {
                auto doAfter = entity->getComponent<WaitUntilComponent>()->doAfter;
                entity->removeComponent<WaitUntilComponent>();
                doAfter(entity);

            }
        });

}
