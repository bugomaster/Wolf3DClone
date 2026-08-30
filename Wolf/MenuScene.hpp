#pragma once
#include "Scene.hpp"


class MenuScene : public Scene{
public:
	enum class MENUTYPE {
		FIRST,
		SECOND,
		NONE
	};


	MenuScene(AppScreen* window, Input* input, SoundManager* audio);
	void handleInput() override;
	void update() override;
	void render() override;
	bool initScene() override;
	void quitScene() override;
private:

};