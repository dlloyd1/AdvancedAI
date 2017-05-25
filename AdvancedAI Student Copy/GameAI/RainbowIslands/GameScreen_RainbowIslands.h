//------------------------------------------------------------------------
//  Author: Paul Roberts 2017
//------------------------------------------------------------------------

#ifndef _GAMESCREEN_RAINBOW_H   
#define _GAMESCREEN_RAINBOW_H

#include "../GameScreen.h"
#include "Commons_RainbowIslands.h"
#include "../Commons.h"
#include <SDL.h>
#include <vector>
#include "../GenAlg.h"
#include "../NeuralNet.h"

using namespace::std;

class Texture2D;
class Character;
class CharacterBub;
class CharacterFruit;
class CharacterRainbow;
class CharacterChest;
class LevelMap;


class GameScreen_RainbowIslands : GameScreen
{
//--------------------------------------------------------------------------------------------------
public:
	GameScreen_RainbowIslands(SDL_Renderer* renderer);
	~GameScreen_RainbowIslands();

	bool SetUpLevel();
	void Render();
	void RenderBackground();
	void Update(float deltaTime, SDL_Event e);

//--------------------------------------------------------------------------------------------------
private:
	void SetLevelMap();
	void RestartLevel();
	void CreateStartingCharacters();

	void MakeEnemiesAngry();
	void UpdateEnemies(float deltaTime, SDL_Event e);
	void CreateCaterpillar(Vector2D position, FACING direction);

	void UpdateRainbows(float deltaTime, SDL_Event e);
	void CreateRainbow(Vector2D position, int numberOfRainbows);

	void UpdateFruit(float deltaTime, SDL_Event e);
	void CreateFruit(Vector2D position, bool bounce);

	void CreateChest(Vector2D position);
	void TriggerChestSpawns();

	void DrawDebugCircle(Vector2D centrePoint, double radius, int red, int green, int blue);
	void DrawDebugLine(Vector2D startPoint, Vector2D endPoint, int red, int green, int blue);

	void UpdateNeuralNetwork();
	void ReadWeightsFile();
	void SaveWeightsFile();

	template<class T>
	Vector2D FindClosest(vector<T*> theList);

//--------------------------------------------------------------------------------------------------
private:
	Texture2D*				  mBackgroundTexture;
	CharacterBub*			  mBubCharacter;
	bool					  mCanSpawnRainbow;
	vector<Character*>		  mEnemies;
	vector<CharacterFruit*>	  mFruit;
	vector<CharacterRainbow*> mRainbows;
	CharacterChest*			  mChest;
	LevelMap*				  mLevelMap;

	int						  mTimeToCompleteLevel;
	bool					  mTriggeredAnger;

	bool					  mTriggeredChestSpawns;
	int						  mNumWeightsInNN;


	GenAlg*					  mGA;
	NeuralNet*				  mNeuralNet;
	vector<Genome>			  mThePopulation;
	int						  mRunThrough;
	int						  mGeneration;

	//store 10 sets of weights to use as a genome
	//vector<vector<double>>    mRandomWeightsVector;

	vector<Genome>			  mGenomeVector;
	vector<double> mInputs;
	vector<double> mOutputs;
	bool loadedWeightsAlready;



	vector<Vector2D> mDebugCirclePositions;
};


#endif //_GAMESCREEN_RAINBOW_H