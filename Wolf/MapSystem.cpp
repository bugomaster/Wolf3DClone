#include "pch.hpp"
#include "MapSystem.hpp"
#include "Components.hpp"

#include "GameScene.hpp"
#include "AssetsLoads.hpp"
#include "GFX.hpp"
#include "Map.hpp"
Entity* MapSystem::gridObjectsMap[GFX::MAP_H][GFX::MAP_W];


MapSystem::MapSystem(GameScene* gameScene) {
	this->gameScene = gameScene;
}
void MapSystem::update(World* world) {


	for (Entity* entity : world->getEntities())
	{
		//DOOR
		if (entity->hasComponent<DoorComponent>())
		{
			Entity* door = entity;
			auto* doorComp = door->getComponent<DoorComponent>();
			auto* doorPos = door->getComponent<PositionComponent>();


			if (doorComp->opening)
			{
				doorComp->open = true;
				doorPos->position.x += doorComp->velDoor.x;
				doorPos->position.y += doorComp->velDoor.y;
				if (wentPast(doorComp->velDoor, doorPos->position, doorComp->openPos))
				{
					doorComp->opening = false;
					door->getComponent<TimerComponent>()->addTimer(1000, [](Entity* entity) {
						entity->getComponent<DoorComponent>()->closing = true;
					});
				}
			}
			else if (doorComp->closing)
			{
				auto nextPos = doorPos->getRelativeHB();
				nextPos.x += doorComp->velDoor.x * -1.f;
				nextPos.y += doorComp->velDoor.y * -1.f;
				const auto& data = Map::checkEntitiesOverlap(world, door, nextPos);
				if (!data.wasCol)
				{
					doorPos->position.x = nextPos.x;
					doorPos->position.y = nextPos.y;
				}

				if (wentPast(doorComp->velDoor*-1.f, doorPos->position, doorComp->originalPos))
				{
					doorComp->closing = false;
					doorComp->open = false;

				}

			}
		}
	}

}
void MapSystem::createCollectibleEntity(World* world, Vector2f worldPos, int collectibleID)
{
	switch (collectibleID)
	{
	case 0: {
		Entity* ammo = world->createEntity();
		ammo->addComponent<CollectibleComponent>(Collectible::AMMO);
		ammo->addComponent<PositionComponent>(worldPos, 0.2f);
		ammo->addComponent<SpritesheetComponent>(SPRSHEET_DATA::AMMO, 4);
		ammo->addComponent<TextureComponent>(g_assets.collectibleTMap.texture);
		ammo->addComponent<RayCastDotObjectComponent>();
		ammo->addComponent<NotCollideableComponent>(std::vector<ComponentID>{getComponentTypeID<PlayerComponent>()});
		ammo->addComponent<ColNotEffectMovement>();

	}break;
	default:
		break;
	}
}

void MapSystem::createDoorHor(World* world, Vector2f posDoor)
{
	posDoor.y += 0.5f;
	Entity* door = world->createEntity();
	MapSystem::gridObjectsMap[(int)posDoor.y][(int)posDoor.x] = door;
	//left and right to the door 
	MapSystem::gridObjectsMap[(int)posDoor.y][(int)posDoor.x + 1]->getComponent<RectFacesComponent>()->faceIDs[0] = 58;
	MapSystem::gridObjectsMap[(int)posDoor.y][(int)posDoor.x - 1]->getComponent<RectFacesComponent>()->faceIDs[1] = 58;


	door->addComponent<PositionComponent>(posDoor)->hitBox = { 0.f,0.f, 1.f,0.01f };
	door->addComponent<RayCastRectObjectComponent>(Vector2f{ 1.f,0.01f });
	door->addComponent<SpritesheetComponent>(SPRSHEET_DATA::WALLTMAP, 56);
	door->addComponent<TextureComponent>(g_assets.wallTMap.texture);
	door->addComponent<CantCollideWithComponent>(std::vector<ComponentID>
	{ getComponentTypeID<WallComponent>() });
	door->addComponent<TimerComponent>();
	door->addComponent<DoorComponent>(Vector2i{1, 0}, 300, posDoor);

}
void MapSystem::createDoorVer(World* world, Vector2f posDoor)
{
	posDoor.x += 0.5f;
	Entity* door = world->createEntity();
	MapSystem::gridObjectsMap[(int)posDoor.y][(int)posDoor.x] = door;
	//left and right to the door 
	MapSystem::gridObjectsMap[(int)posDoor.y - 1][(int)posDoor.x]->getComponent<RectFacesComponent>()->faceIDs[3] = 58;
	MapSystem::gridObjectsMap[(int)posDoor.y + 1][(int)posDoor.x]->getComponent<RectFacesComponent>()->faceIDs[2] = 58;


	door->addComponent<PositionComponent>(posDoor)->hitBox = { 0.f,0.f, 0.01f,1.f };
	door->addComponent<RayCastRectObjectComponent>(Vector2f{ 0.01f, 1.f });
	door->addComponent<SpritesheetComponent>(SPRSHEET_DATA::WALLTMAP, 56);
	door->addComponent<TextureComponent>(g_assets.wallTMap.texture);

	door->addComponent<CantCollideWithComponent>(std::vector<ComponentID>
	{ getComponentTypeID<WallComponent>() });

	door->addComponent<TimerComponent>();
	door->addComponent<DoorComponent>(Vector2i{ 0, -1 }, 300, posDoor);

}
void MapSystem::onAddedToWorld(World* world)
{
	// WALLS
	for (int y = 0; y < GFX::MAP_H; y++)
	{
		for (int x = 0; x < GFX::MAP_W; x++)
		{
			int textureIDWall = Map::wallMap[y][x];
			if (textureIDWall < 1)
				continue;
			Entity* wallBox = world->createEntity();
			wallBox->addComponent<SpritesheetComponent>(SPRSHEET_DATA::WALLTMAP, textureIDWall);
			wallBox->addComponent<RectFacesComponent>(
				std::vector<int> { textureIDWall, textureIDWall, textureIDWall, textureIDWall });
			wallBox->addComponent<TextureComponent>(g_assets.wallTMap.texture);
			wallBox->addComponent<PositionComponent>(Vector2f{ (float)x,(float)y})->hitBox = { 0.f,0.f, 1.f,1.f };
			wallBox->addComponent<WallComponent>();
			wallBox->addComponent<RayCastRectObjectComponent>(Vector2f{ 1.f,1.f });
			MapSystem::gridObjectsMap[y][x] = wallBox;

		}

	}

	//DOORS
	for (int y = 0; y < GFX::MAP_H; y++)
	{
		for (int x = 0; x < GFX::MAP_W; x++)
		{
			int textureIDWall = Map::wallMap[y][x];
			if (textureIDWall == -1)
			{
				createDoorHor(world, Vector2f{ (float)x, (float)y});
			}
			if (textureIDWall == -2)
			{
				createDoorVer(world, Vector2f{ (float)x, (float)y});
			}
		}

	}

	

	{
		//Entity* lamp = world->createEntity();
		//lamp->addComponent<RayCastDotObjectComponent>();
		//lamp->addComponent<PositionComponent>(Vector2f{ 8.5f,8.5f });
		//lamp->addComponent<SpritesheetComponent>(SPRSHEET_DATA::BASIC4X4, 3);
		//lamp->addComponent<TextureComponent>(g_assets.decorationsTMap.texture);
		//lamp->addComponent<DecorativeObjectComponent>();
		//lamp->addComponent<NotCollideableComponent>();
	}


}
