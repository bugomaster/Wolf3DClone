#include "pch.hpp"

#include "MenuScene.hpp"
#include "AppWindow.hpp"
#include "GFX.hpp"
#include "AssetsLoads.hpp"
#include "Input.hpp"
#include "SoundManager.hpp"
#include <fstream>


MenuScene::MenuScene(AppScreen* window, Input* input, SoundManager* audio)
{
	this->window = window;
	this->input = input;
	this->audio = audio;
}

bool MenuScene::initScene()
{
	return true;
}

void MenuScene::handleInput()
{
}

void MenuScene::update()
{
}

void MenuScene::render()
{
}

void MenuScene::quitScene()
{
}
