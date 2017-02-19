//------------------------------------------------------------------------
//  Author: Paul Roberts 2016
//------------------------------------------------------------------------

#ifndef _GAMESCREEN_CONWAY_H
#define _GAMESCREEN_CONWAY_H

#include "../GameScreen.h"
#include "../Commons.h"
#include <SDL.h>
#include <array>

using namespace::std;

class Texture2D;

class GameScreen_Conway : GameScreen
{
	//--------------------------------------------------------------------------------------------------
public:
	GameScreen_Conway(SDL_Renderer* renderer);
	~GameScreen_Conway();

	void Render();
	void Update(float deltaTime, SDL_Event e);

	//--------------------------------------------------------------------------------------------------
protected:

	//TODO: Students to code these functions.
	void UpdateMap();//{;}
	void CreateRandomMap(int percentageInactive);//{;}
	void LoadMap(std::string path);//{;}
	int GetLivingNeighbourCount(int mapX, int mapY);
	void SetNextMapState();
	void ZeroMap(int** &mMap);

	//--------------------------------------------------------------------------------------------------
private:
	int**		mMap;
	int**		mNextMapState;

	Texture2D*	mWhiteTile;
	Texture2D*	mBlackTile;

	bool		mPause;
};


#endif //_GAMESCREEN_CONWAY_H