#pragma once
#include <SDL_pixels.h>

namespace ANIMATIONS
{
	constexpr int GUARD_DIE[5] = { 40, 41, 42, 43, 44 };

}



namespace COLORS {
	constexpr SDL_Color BACKGROUND_COLOR = { 147, 187, 236, 255 };

	constexpr SDL_Color WHITE{ 255, 255, 255, 255 };
	constexpr SDL_Color FONT{ 184 , 188 ,252,255 };
	constexpr SDL_Color BLACK{ 0,   0,   0,   255 };
	constexpr SDL_Color RED{ 255, 0,   0,   255 };
	constexpr SDL_Color GREEN{ 0,   255, 0,   255 };
	constexpr SDL_Color BLUE{ 0,   0,   255, 255 };
	constexpr SDL_Color YELLOW{ 255, 255, 0,   255 };
	constexpr SDL_Color CYAN{ 0,   255, 255, 255 };
	constexpr SDL_Color MAGENTA{ 255, 0,   255, 255 };
	constexpr SDL_Color ORANGE{ 255, 165, 0,   255 };
	constexpr SDL_Color PURPLE{ 128, 0,   128, 255 };
	constexpr SDL_Color PINK{ 255, 105, 180, 255 };
	constexpr SDL_Color GRAY{ 128, 128, 128, 255 };


}
namespace GFX
{
	constexpr float MAX_SHOOT_RANGE = 20.f;
	constexpr float PLAYER_RADIUS = 0.3f;
	constexpr int MAX_DEPTH = 64;
	constexpr float ENEMY_RADIUS = 0.1f;
	constexpr int SCREEN_WIDTH = 800;
	constexpr int SCREEN_HEIGHT = 600;
	constexpr int MAX_FPS = 60;
	constexpr float PI = 3.141592653589793115997963468544185161590576171875f;
	constexpr float TAU = PI*2.f;


	constexpr int NUM_RAYS = 400;
	constexpr int MAP_W = 20;
	constexpr int MAP_H = 20;
	constexpr int GRID_TILE_SIZE = 64;
	constexpr int SCALE = SCREEN_WIDTH / NUM_RAYS;

	constexpr float FOV = (PI / 3.0f);
	constexpr float DELTA_ANGLE = (FOV / NUM_RAYS);



}

namespace MovementConstants {
	constexpr float ENEMY_SPEED = 0.027f;
}
#define SCREEN_DIST (float)(GFX::SCREEN_WIDTH / (2.0f * tanf(GFX::FOV / 2.0f)))
