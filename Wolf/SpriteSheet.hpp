#pragma once

struct SpriteSheetData
{

	int entityWidth;
	int entityHeight;
	int entityXPad;
	int entityYPad;

	//
	int xOffset;
	int yOffset;
	int gridRowGap;
	int gridColGap;
	int gridWidth;
	int gridHeight;
	int rows;
	int cols;
};

inline Vector2i getTileMapTexCoord(int id, const SpriteSheetData& sprSheet)
{
	int colIndex = id % sprSheet.cols;
	int rowIndex = id / sprSheet.cols;

	return {
		sprSheet.xOffset + colIndex * (sprSheet.gridWidth + sprSheet.gridColGap),
		sprSheet.yOffset + rowIndex * (sprSheet.gridHeight + sprSheet.gridRowGap)
	};
}



namespace SPRSHEET_DATA
{
	constexpr int GRIDSIZE = 64;
	constexpr SpriteSheetData WEAPONS = {
		GRIDSIZE, GRIDSIZE,0,0, 1, 16, 16, 1,
		GRIDSIZE ,GRIDSIZE,7,5 };
	
	
	
	//
	constexpr SpriteSheetData COLLECTIBLES = {
		GRIDSIZE, GRIDSIZE,0,0, 1, 0, 16, 1,
		GRIDSIZE ,GRIDSIZE,4,4 };

	constexpr SpriteSheetData AMMO = {
		10, 10, 27, 54, 1, 0, 16, 1,
		GRIDSIZE ,GRIDSIZE,4,4 };




	//
	constexpr SpriteSheetData GUARDTMAP = {
		22, GRIDSIZE, 22, 0, 0, 0, 1, 1,
		GRIDSIZE ,GRIDSIZE,8,8 };
	constexpr SpriteSheetData DEAD_GUARDTMAP = {
		GRIDSIZE, 22, 0, 42, 0, 0, 1, 1,
		GRIDSIZE ,GRIDSIZE,8,8 };


	constexpr SpriteSheetData BASIC8X8 = {
		GRIDSIZE, GRIDSIZE,0,0, 0, 0, 1, 1,
		GRIDSIZE ,GRIDSIZE,8,8 };
	constexpr SpriteSheetData BASIC4X4 = {
		GRIDSIZE, GRIDSIZE,0,0, 0, 0, 1, 1,
		GRIDSIZE ,GRIDSIZE,4,4 };
	constexpr SpriteSheetData WALLTMAP = {
		GRIDSIZE, GRIDSIZE,0,0, 1, 16, 1, 1,
		GRIDSIZE ,GRIDSIZE,8,8 };
}
