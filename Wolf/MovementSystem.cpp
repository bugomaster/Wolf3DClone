#include "pch.hpp"
#include "MovementSystem.hpp"
#include "Components.hpp"

#include "Map.hpp"





void MovementSystem::update(World* world) 
{




	for (Entity* entity : world->getEntities())
	{
		if (entity->hasComponent<PlayerComponent>())// doesnt update here
			continue;
		if (!entity->hasComponent<PositionComponent>() ||
			!entity->hasComponent<VelocityComponent>())
			continue;

		
		auto* posComp = entity->getComponent<PositionComponent>();
		const auto* velComp = entity->getComponent<VelocityComponent>();

		if (!entity->hasComponent<CantCollideAllComponent>())
		{
			auto moveData = Map::movePosWithCol(world, entity);
			if (moveData.wasCollsion)
			{
				entity->addComponent<WasCollisionComponent>(moveData);
			}
			else
			{
				entity->removeComponent<WasCollisionComponent>();
			}
			Map::applyMoveData(moveData, entity);
			
		}
		else
		{
			posComp->position.x += velComp->dx;
			posComp->position.y += velComp->dy;
		}
		posComp->changeAngle(velComp->dAngle);//rad

	}



}