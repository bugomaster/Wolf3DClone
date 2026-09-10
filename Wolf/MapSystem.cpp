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
Entity* MapSystem::createCollectibleEntity(World* world, Vector2f worldPos, Collectible type)
{
	Entity* collectible = world->createEntity();
	int frameId = 0;
	switch (type)
	{
	case Collectible::AMMO: {
		frameId = 4;
	}break;
	case Collectible::GOLDBOX: {
		frameId = 10;
	}break;
	case Collectible::TROPHIE: {
		frameId = 9;
	}break;
	case Collectible::MEAL: {
		frameId = 1;
	}break;
	case Collectible::MEATBALLS: {
		frameId = 0;
	}break;
	case Collectible::BLUEKEY: {
		frameId = 13;
	}break;
	case Collectible::GOLDKEY: {
		frameId = 14;
	}break;
	default:
		break;
	}

	collectible->addComponent<SpritesheetComponent>(SPRSHEET_DATA::COLLECTIBLESPR[(int)(type)], frameId);

	collectible->addComponent<CollectibleComponent>(type);
	collectible->addComponent<PositionComponent>(worldPos, 0.2f);

	collectible->addComponent<TextureComponent>(g_assets.collectibleTMap.texture);
	collectible->addComponent<RayCastDotObjectComponent>();
	collectible->addComponent<NotCollideableComponent>(std::vector<ComponentID>{getComponentTypeID<PlayerComponent>()});
	collectible->addComponent<ColNotEffectMovement>();
	return collectible;

}

void MapSystem::createDoorHor(World* world, const Door& door)
{
	auto posDoor = door.position.to<float>();
	posDoor.y += 0.5f;
	Entity* doorEntity = world->createEntity();
	MapSystem::gridObjectsMap[(int)posDoor.y][(int)posDoor.x] = doorEntity;
	//left and right to the door 
	MapSystem::gridObjectsMap[(int)posDoor.y][(int)posDoor.x + 1]->getComponent<RectFacesComponent>()->faceIDs[0] = 58;
	MapSystem::gridObjectsMap[(int)posDoor.y][(int)posDoor.x - 1]->getComponent<RectFacesComponent>()->faceIDs[1] = 58;


	doorEntity->addComponent<PositionComponent>(posDoor.to<float>())->hitBox = { 0.f,0.f, 1.f,0.01f };
	doorEntity->addComponent<RayCastRectObjectComponent>(Vector2f{ 1.f,0.01f });
	doorEntity->addComponent<SpritesheetComponent>(SPRSHEET_DATA::WALLTMAP, 56);
	doorEntity->addComponent<TextureComponent>(g_assets.wallTMap.texture);
	doorEntity->addComponent<CantCollideWithComponent>(std::vector<ComponentID>
	{ getComponentTypeID<WallComponent>() });
	doorEntity->addComponent<TimerComponent>();
	doorEntity->addComponent<DoorComponent>(door.dirMove, 300, posDoor.to<float>());

}
void MapSystem::createDoorVer(World* world, const Door& door)
{
	auto posDoor = door.position.to<float>();
	posDoor.x += 0.5f;
	Entity* doorEntity = world->createEntity();

	MapSystem::gridObjectsMap[(int)posDoor.y][(int)posDoor.x] = doorEntity;
	//left and right to the door 
	MapSystem::gridObjectsMap[(int)posDoor.y - 1][(int)posDoor.x]->getComponent<RectFacesComponent>()->faceIDs[3] = 58;
	MapSystem::gridObjectsMap[(int)posDoor.y + 1][(int)posDoor.x]->getComponent<RectFacesComponent>()->faceIDs[2] = 58;


	doorEntity->addComponent<PositionComponent>(posDoor.to<float>())->hitBox = { 0.f,0.f, 0.01f,1.f };
	doorEntity->addComponent<RayCastRectObjectComponent>(Vector2f{ 0.01f, 1.f });
	doorEntity->addComponent<SpritesheetComponent>(SPRSHEET_DATA::WALLTMAP, 56);
	doorEntity->addComponent<TextureComponent>(g_assets.wallTMap.texture);

	doorEntity->addComponent<CantCollideWithComponent>(std::vector<ComponentID>
	{ getComponentTypeID<WallComponent>() });

	doorEntity->addComponent<TimerComponent>();
	doorEntity->addComponent<DoorComponent>(door.dirMove, 300, posDoor.to<float>());

}
void MapSystem::createSecretMoveableWall(World* world, const SecretWall& wall)
{
	Entity* door = world->createEntity();
	MapSystem::gridObjectsMap[wall.position.y][wall.position.x] = door;

	door->addComponent<PositionComponent>(wall.position.to<float>())->hitBox = { 0.f,0.f, 1.f,1.f };
	door->addComponent<RayCastRectObjectComponent>(Vector2f{ 1.f, 1.f });
	door->addComponent<SpritesheetComponent>(SPRSHEET_DATA::WALLTMAP, wall.tileID);
	door->addComponent<TextureComponent>(g_assets.wallTMap.texture);


	door->addComponent<TimerComponent>();
	door->addComponent<VelocityComponent>(0.f, 0.f);

	door->addComponent<SecretWallComponent>(wall.position, wall.dirMove);

}
void MapSystem::createLockGate(World* world, const LockGate& lockGate)
{
	Entity* door = world->createEntity();
	MapSystem::gridObjectsMap[lockGate.position.y][lockGate.position.x] = door;

	door->addComponent<PositionComponent>(lockGate.position.to<float>())->hitBox = { 0.f,0.f, 1.f,1.f };
	door->addComponent<RayCastRectObjectComponent>(Vector2f{ 1.f, 1.f });
	door->addComponent<RectFacesComponent>(std::vector<int>{lockGate.tileID, 40, 40, 40});

	door->addComponent<SpritesheetComponent>(SPRSHEET_DATA::WALLTMAP, 0);//no need for tile id cuz rectfacescomp
	door->addComponent<TextureComponent>(g_assets.wallTMap.texture);


	door->addComponent<TimerComponent>();
	door->addComponent<VelocityComponent>(0.f, 0.f);

	door->addComponent<LockGateComponent>(lockGate.position, 0, lockGate.keyID);

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
			wallBox->addComponent<PositionComponent>(Vector2f{ (float)x,(float)y })->hitBox = { 0.f,0.f, 1.f,1.f };
			wallBox->addComponent<WallComponent>();
			wallBox->addComponent<RayCastRectObjectComponent>(Vector2f{ 1.f,1.f });
			MapSystem::gridObjectsMap[y][x] = wallBox;

		}

	}

	//DOORS
	for (const auto& door : this->gameScene->levelData.doors)
	{
		if (door.dirMove.x != 0)
		{
			createDoorHor(world, door);
		}
		else if (door.dirMove.y != 0)
		{
			createDoorVer(world, door);
		}
	}
	for (const auto& secretWall : this->gameScene->levelData.secretWalls)
	{
		createSecretMoveableWall(world, secretWall);
	}
	for (const auto& lockGate : this->gameScene->levelData.lockGates)
	{
		createLockGate(world, lockGate);
	}
	for (const auto& collectible : this->gameScene->levelData.collectibles)
	{
		createCollectibleEntity(world, collectible.position, collectible.type);
	}
	for (const auto& keyData : this->gameScene->levelData.keys)
	{
		Collectible type = Collectible::BLUEKEY;
		if (keyData.keyID == 2)
			type = Collectible::GOLDKEY;
		Entity* keyEntity = createCollectibleEntity(world, keyData.position, type);
	}
	for (const auto& decoration : this->gameScene->levelData.decorations)
	{
		Entity* decorationEntity = world->createEntity();
		int frameId = 0;
		float radius = 0.2f;
		switch (decoration.type)
		{
		case Decoration::LAMP:{
			frameId = 2;
		}break;
		case Decoration::TREE:{
			frameId = 5;
		}break;
		case Decoration::FLAG:{
			frameId = 17;
		}break;
		case Decoration::TABLE:{
			radius = 0.4f;
			frameId = 9;
		}break;
		default:
			break;
		}

		decorationEntity->addComponent<SpritesheetComponent>(SPRSHEET_DATA::DECORATIONSPR[(int)(decoration.type)], frameId);
		decorationEntity->addComponent<PositionComponent>(decoration.position, radius);
		decorationEntity->addComponent<TextureComponent>(g_assets.decorations2TMap.texture);
		decorationEntity->addComponent<RayCastDotObjectComponent>(false);


	}


}