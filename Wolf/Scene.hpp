#pragma once
class AppScreen;
class Input;
class SoundManager;
class Scene {
public:
	
	virtual ~Scene() = default;
	virtual void handleInput() = 0;
	virtual void update() = 0;
	virtual void render() = 0;
	virtual bool initScene() = 0;
	virtual void quitScene() = 0;
	bool isFinished() { return finished; }
	void setFinished(bool val) { finished = val; }
	AppScreen* getScreen() { return window; }
	Input* getInput() { return input; }
	SoundManager* getAudio() { return audio; }
protected:
	bool finished = false;
	AppScreen* window;
	Input* input;
	SoundManager* audio;
};