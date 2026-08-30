#pragma once

#include "System.hpp"
 
class GameScene;
class AnimationSystem : public System {
public:
	AnimationSystem(GameScene* scene) :
		scene(scene)
	{
	};
	AnimationSystem() = default;

	~AnimationSystem() override = default;

	void update(World* world) override;

private:
	GameScene* scene;
};
