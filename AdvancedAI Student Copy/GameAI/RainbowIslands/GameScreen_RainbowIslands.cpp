//------------------------------------------------------------------------
//  Author: Paul Roberts 2017
//------------------------------------------------------------------------

#include "GameScreen_RainbowIslands.h"
#include <iostream>
#include "LevelMap.h"
#include "Character.h"
#include "CharacterBub.h"
#include "CharacterCaterpillar.h"
#include "CharacterRainbow.h"
#include "CharacterFruit.h"
#include "CharacterChest.h"
//#include "Commons_RainbowIslands.h"
#include "Constants_RainbowIslands.h"
#include "../Texture2D.h"
#include "../Collisions.h"
#include "VirtualJoypad.h"
#include <time.h>
#include <fstream>

using namespace::std;

int RainbowOffsets[] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,22,22,23,23,
						 23,22,22,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0, };
//--------------------------------------------------------------------------------------------------

GameScreen_RainbowIslands::GameScreen_RainbowIslands(SDL_Renderer* renderer) : GameScreen(renderer)
{
	loadedWeightsAlready = false;
	mRunThrough = 0;
	mGeneration = 0;

	srand(time(NULL));
	mLevelMap = NULL;
	mGenomeVector.clear();
	SetUpLevel();
}

//--------------------------------------------------------------------------------------------------

GameScreen_RainbowIslands::~GameScreen_RainbowIslands()
{
	//Background image.
	delete mBackgroundTexture;
	mBackgroundTexture = NULL;

	//Player character.
	delete mBubCharacter;
	mBubCharacter = NULL;

	//Level map.
	delete mLevelMap;
	mLevelMap = NULL;

	//Treasure chest.
	delete mChest;
	mChest = NULL;

	//Enemies.
	mEnemies.clear();

	//Fruit.
	mFruit.clear();

	//Rainbows.
	mRainbows.clear();

	delete mGA;
	mThePopulation.clear();
}

//--------------------------------------------------------------------------------------------------

void GameScreen_RainbowIslands::Render()
{
	//Draw the background.
	RenderBackground();

	//Draw the chest.
	if (mChest != NULL)
		mChest->Render();

	//Draw the Fruit.
	for (unsigned int i = 0; i < mFruit.size(); i++)
	{
		mFruit[i]->Render();
	}

	//Draw the Enemies.
	for (unsigned int i = 0; i < mEnemies.size(); i++)
	{
		mEnemies[i]->Render();
		DrawDebugCircle(mEnemies[i]->GetCentralPosition(), mEnemies[i]->GetCollisionRadius(), 255, 0, 0);
	}

	//Draw the Rainbows.
	for (unsigned int i = 0; i < mRainbows.size(); i++)
	{
		mRainbows[i]->Render();
		if (mRainbows[i]->CanKill())
			DrawDebugCircle(mRainbows[i]->GetStrikePosition(), mRainbows[i]->GetCollisionRadius(), 255, 255, 255);
	}

	//Draw the player.
	mBubCharacter->Render();
	DrawDebugCircle(mBubCharacter->GetCentralPosition(), mBubCharacter->GetCollisionRadius(), 0, 255, 0);

	for (auto p : mDebugCirclePositions)
	{
		DrawDebugCircle(p, 10.0f, 0, 0, 255);
	}
}

//--------------------------------------------------------------------------------------------------

void GameScreen_RainbowIslands::RenderBackground()
{
	if (mLevelMap != NULL)
	{
		for (int x = 0; x < MAP_WIDTH; x++)
		{
			for (int y = 0; y < MAP_HEIGHT; y++)
			{
				//Get the portion of the spritesheet you want to draw.
				int num = mLevelMap->GetBackgroundTileAt(y, x);
				int w = TILE_WIDTH*(num % kTileSpriteSheetWidth);
				int h = TILE_HEIGHT*(num / kTileSpriteSheetWidth);
				//								{XPos, YPos, WidthOfSingleSprite, HeightOfSingleSprite}
				SDL_Rect portionOfSpritesheet = { w, h, TILE_WIDTH, TILE_HEIGHT };

				//Determine where you want it drawn.
				SDL_Rect destRect = { x*TILE_WIDTH, kRainbowIslandsScreenHeight - ((MAP_HEIGHT - y)*TILE_HEIGHT), TILE_WIDTH, TILE_HEIGHT };

				//Then draw it.
				mBackgroundTexture->Render(portionOfSpritesheet, destRect, SDL_FLIP_NONE);
			}
		}
	}
}

void GameScreen_RainbowIslands::UpdateNeuralNetwork()
{
	mInputs.clear();
	mOutputs.clear();
	mDebugCirclePositions.clear();

	//grid of feeler balls - 3x3 including bub's position
	int yTop = (int)mBubCharacter->GetCentralPosition().y - mBubCharacter->GetSingleSpriteHeight();
	int yBottom = (int)mBubCharacter->GetCentralPosition().y + mBubCharacter->GetSingleSpriteHeight();
	int xLeft = (int)mBubCharacter->GetCentralPosition().x - mBubCharacter->GetSingleSpriteWidth();
	int xRight = (int)mBubCharacter->GetCentralPosition().x + mBubCharacter->GetSingleSpriteWidth();
	int count = 0;
	for (int y = yTop; y < yBottom + mBubCharacter->GetSingleSpriteHeight(); y += mBubCharacter->GetSingleSpriteHeight())
	{
		for (int x = xLeft; x < xRight + mBubCharacter->GetSingleSpriteWidth(); x += mBubCharacter->GetSingleSpriteWidth())
		{
			count++;
			mDebugCirclePositions.push_back(Vector2D(x, y));	
			//no rainbow, so is this feeler of either side of the level?
			if (x <= 0 || x >= kRainbowIslandsScreenWidth)
			{
				mInputs.push_back(1.0);
			}
			else
			{
				//feeler is on screen and not on a rainbow, so use collision map as input
				mInputs.push_back((double)(mLevelMap->GetCollisionTileAt((y / TILE_HEIGHT), (x / TILE_WIDTH))));
			}
		}
	}

	//closest fruit
	Vector2D closestLoc = FindClosest(mFruit);
	mInputs.push_back((double)(1 - ((mBubCharacter->GetCentralPosition() - closestLoc).Length()) / kRainbowIslandsScreenHeight));

	//closest enemy
	Vector2D closestELoc = FindClosest(mEnemies);
	mInputs.push_back((((double)(mBubCharacter->GetCentralPosition() - closestELoc).Length()) / kRainbowIslandsScreenHeight));

	//height up screen
	mInputs.push_back((double)(1 - (mBubCharacter->GetCentralPosition().y / kRainbowIslandsScreenHeight)));

	if (mBubCharacter->OnARainbow())
	{
		mInputs.push_back(1.0);
	}
	else
	{
		mInputs.push_back(0.0);
	}
	mOutputs = mNeuralNet->Update(mInputs);
}

template<class T>
Vector2D GameScreen_RainbowIslands::FindClosest(vector<T*> theList)
{
	double smallestDist = MaxDouble;
	Vector2D closestLoc;
	for (auto item : theList)
	{
		double temp = Vec2DDistance(mBubCharacter->GetCentralPosition(), item->GetCentralPosition());
		if (temp < smallestDist)
		{
			smallestDist = temp;
			closestLoc = item->GetCentralPosition();
		}
	}
	return closestLoc;
}

void GameScreen_RainbowIslands::SaveWeightsFile()
{
	fstream fs;
	fs.open("weightsProgress.txt", fstream::out);

	for (size_t i = 0; i < mGenomeVector.size(); ++i)
	{
		for each (double dub in mGenomeVector[i].mWeights)
		{
			fs << dub << endl;
		}
	}
}

void GameScreen_RainbowIslands::ReadWeightsFile()
{
	fstream fs;
	fs.open("weightsProgress.txt", fstream::in);

	for (size_t i = 0; i < kRunsPerGeneration; ++i)
	{
		for (size_t j = 0; j < mNeuralNet->GetNumberOfWeights(); ++j)
		{
			string input;
			fs >> input;
			double weightAsDub = atof(input.c_str());
			mGenomeVector[i].mWeights.push_back(weightAsDub);
		}
	}
}
//--------------------------------------------------------------------------------------------------

void GameScreen_RainbowIslands::Update(float deltaTime, SDL_Event e)
{
	//--------------------------------------------------------------------------------------------------
	//Update the Virtual Joypad.
	//--------------------------------------------------------------------------------------------------
	VirtualJoypad::Instance()->LeftArrow = false;
	VirtualJoypad::Instance()->RightArrow = false;
	VirtualJoypad::Instance()->UpArrow = false;
	VirtualJoypad::Instance()->DownArrow = false;
	VirtualJoypad::Instance()->ForceRestart = false;

	VirtualJoypad::Instance()->SetJoypadState(e);

	if (VirtualJoypad::Instance()->ForceRestart)
	{
		RestartLevel();
	}

	//--------------------------------------------------------------------------------------------------
	//Update the level time.
	//--------------------------------------------------------------------------------------------------
	mTimeToCompleteLevel -= (int)deltaTime;
	if (mTimeToCompleteLevel < TRIGGER_ANGRY_TIME && !mTriggeredAnger)
	{
		MakeEnemiesAngry();
		if (mTimeToCompleteLevel <= 0)
		{
			RestartLevel();
		}
	}

	//--------------------------------------------------------------------------------------------------
	//Update the neural net. populate inputs and outputs
	//--------------------------------------------------------------------------------------------------

	UpdateNeuralNetwork();


	if (mOutputs[0] > 0.5)
	{
		VirtualJoypad::Instance()->LeftArrow = true;
	}
	if (mOutputs[1] > 0.5)
	{
		VirtualJoypad::Instance()->UpArrow = true;
	}
	if (mOutputs[2] > 0.5)
	{
		VirtualJoypad::Instance()->RightArrow = true;
	}
	if (mOutputs[3] > 0.5)
	{
		VirtualJoypad::Instance()->DownArrow = true;
	}
	
	//--------------------------------------------------------------------------------------------------
	//Update the player.
	//--------------------------------------------------------------------------------------------------
	mBubCharacter->Update(deltaTime, e);
	
	if (!mBubCharacter->GetAlive())
	{
		RestartLevel();
	}
	else if (mBubCharacter->GetPosition().y < 60)
	{
		CreateChest(Vector2D(kRainbowIslandsScreenWidth*0.25f, -50.0f));
	}

	//--------------------------------------------------------------------------------------------------
	//Update the game objects.
	//--------------------------------------------------------------------------------------------------
	UpdateFruit(deltaTime, e);
	UpdateEnemies(deltaTime, e);
	UpdateRainbows(deltaTime, e);

	//We have a chest, but is it open yet.
	if (mChest != NULL)
	{
		mChest->Update(deltaTime, e);
		if (mChest->IsChestOpen() && !mTriggeredChestSpawns)
		{
			TriggerChestSpawns();
		}
	}

	//--------------------------------------------------------------------------------------------------
	//Check if we need to create a new rainbow.
	//--------------------------------------------------------------------------------------------------
	if (VirtualJoypad::Instance()->DownArrow && mCanSpawnRainbow)
	{
		bool collidingWithRainbow = false;

		for (unsigned int i = 0; i < mRainbows.size(); i++)
		{
			if (Collisions::Instance()->Box(mBubCharacter->GetCollisionBox(), mRainbows[i]->GetCollisionBox()))
			{
				collidingWithRainbow = true;
				break;
			}
		}

		if (!collidingWithRainbow)
		{
			Vector2D pos = mBubCharacter->GetPosition();
			pos.x += 10;
			if (mBubCharacter->GetFacing() == FACING_RIGHT)
				pos.x += mBubCharacter->GetCollisionBox().width - 15;
			pos.y -= mBubCharacter->GetCollisionBox().height*0.3f;
			CreateRainbow(pos, mBubCharacter->GetRainbowsAllowed());

			mCanSpawnRainbow = false;
		}
	}
	else if (!VirtualJoypad::Instance()->DownArrow)
	{
		mCanSpawnRainbow = true;
	}
}

//--------------------------------------------------------------------------------------------------

void GameScreen_RainbowIslands::MakeEnemiesAngry()
{
	if (!mEnemies.empty())
	{
		int enemyIndexToDelete = -1;
		for (unsigned int i = 0; i < mEnemies.size(); i++)
		{
			mEnemies[i]->MakeAngry();
		}
	}
}

//--------------------------------------------------------------------------------------------------

void GameScreen_RainbowIslands::UpdateEnemies(float deltaTime, SDL_Event e)
{
	//--------------------------------------------------------------------------------------------------
	//Update the Enemies.
	//--------------------------------------------------------------------------------------------------
	if (!mEnemies.empty())
	{
		int enemyIndexToDelete = -1;
		for (unsigned int i = 0; i < mEnemies.size(); i++)
		{
			mEnemies[i]->Update(deltaTime, e);

			//Check to see if the enemy collides with the player.
			if (Collisions::Instance()->Circle(mEnemies[i]->GetCentralPosition(), mEnemies[i]->GetCollisionRadius(), mBubCharacter->GetCentralPosition(), mBubCharacter->GetCollisionRadius()))
			{
				mBubCharacter->SetState(CHARACTERSTATE_PLAYER_DEATH);
			}

			//If the enemy is no longer alive, then schedule it for deletion.
			if (!mEnemies[i]->GetAlive())
			{
				enemyIndexToDelete = i;
			}
		}

		//--------------------------------------------------------------------------------------------------
		//Remove a dead enemies - 1 each update.
		//--------------------------------------------------------------------------------------------------
		if (enemyIndexToDelete != -1)
		{
			Character* toDelete = mEnemies[enemyIndexToDelete];

			//We need a pickup to be generated.
			CreateFruit(toDelete->GetPosition(), true);

			mEnemies.erase(mEnemies.begin() + enemyIndexToDelete);
			delete toDelete;
			toDelete = NULL;
		}
	}
}

//--------------------------------------------------------------------------------------------------

void GameScreen_RainbowIslands::UpdateFruit(float deltaTime, SDL_Event e)
{
	//--------------------------------------------------------------------------------------------------
	//Update the Fruit.
	//--------------------------------------------------------------------------------------------------
	if (!mFruit.empty())
	{
		int fruitIndexToDelete = -1;
		for (unsigned int i = 0; i < mFruit.size(); i++)
		{
			//Update the fruit.
			mFruit[i]->Update(deltaTime, e);

			//check if the player has collided with it.
			if (Collisions::Instance()->Circle(mFruit[i]->GetCentralPosition(), mFruit[i]->GetCollisionRadius(), mBubCharacter->GetCentralPosition(), mBubCharacter->GetCollisionRadius()))
			{
				mFruit[i]->SetAlive(false);
				fruitIndexToDelete = i;
			}
		}

		//--------------------------------------------------------------------------------------------------
		//Remove a dead fruit - 1 each update.
		//--------------------------------------------------------------------------------------------------
		if (fruitIndexToDelete != -1)
		{
			mBubCharacter->AddPoints();

			Character* toDelete = mFruit[fruitIndexToDelete];
			mFruit.erase(mFruit.begin() + fruitIndexToDelete);
			delete toDelete;
			toDelete = NULL;
		}
	}
}

//--------------------------------------------------------------------------------------------------

void GameScreen_RainbowIslands::UpdateRainbows(float deltaTime, SDL_Event e)
{
	//--------------------------------------------------------------------------------------------------
	//Update the Rainbows.
	//--------------------------------------------------------------------------------------------------

	//Need to turn flag off each frame incase the rainbow disapated.
	mBubCharacter->SetOnARainbow(false);

	if (!mRainbows.empty())
	{
		int rainbowIndexToDelete = -1;

		for (unsigned int i = 0; i < mRainbows.size(); i++)
		{
			int xPosition = (int)mBubCharacter->GetPosition().x + (int)(mBubCharacter->GetCollisionBox().width*0.5f);
			int footPosition = (int)(mBubCharacter->GetPosition().y + mBubCharacter->GetCollisionBox().height);

			//Update the rainbow.
			mRainbows[i]->Update(deltaTime, e);

			if (!mRainbows[i]->GetAlive())
				rainbowIndexToDelete = i;
			else
			{
				//check if the player has collided with it.
				if (!mBubCharacter->IsJumping())
				{
					if (Collisions::Instance()->PointInBox(Vector2D(xPosition, footPosition), mRainbows[i]->GetCollisionBox()))
					{
						mBubCharacter->SetState(CHARACTERSTATE_WALK);
						mBubCharacter->SetOnARainbow(true);
						int xPointOfCollision = (int)(mRainbows[i]->GetPosition().x + mRainbows[i]->GetCollisionBox().width - xPosition);
						if (mBubCharacter->GetFacing() == FACING_RIGHT)
							xPointOfCollision = (int)(xPosition - mRainbows[i]->GetPosition().x);

						//We don't want to pop between walking on different rainbows. Ensure the switch between rainbows looks 'realistic'
						double distanceBetweenPoints = footPosition - (mRainbows[i]->GetPosition().y - RainbowOffsets[xPointOfCollision]);
						if (distanceBetweenPoints < 40.0)
							mBubCharacter->SetPosition(Vector2D(mBubCharacter->GetPosition().x, mRainbows[i]->GetPosition().y - RainbowOffsets[xPointOfCollision]));
					}
				}

				//Check for collisions with enemies.
				for (unsigned int j = 0; j < mEnemies.size(); j++)
				{
					if (mRainbows[i]->CanKill())
					{
						if (Collisions::Instance()->Circle(mRainbows[i]->GetStrikePosition(), mRainbows[i]->GetCollisionRadius(), mEnemies[j]->GetPosition(), mEnemies[j]->GetCollisionRadius()))
						{
							mEnemies[j]->SetAlive(false);
						}
					}
				}
			}
		}

		//--------------------------------------------------------------------------------------------------
		//Remove a dead rainbow - 1 each update.
		//--------------------------------------------------------------------------------------------------
		if (rainbowIndexToDelete != -1)
		{
			Character* toDelete = mRainbows[rainbowIndexToDelete];
			mRainbows.erase(mRainbows.begin() + rainbowIndexToDelete);
			delete toDelete;
			toDelete = NULL;
		}
	}
}

//--------------------------------------------------------------------------------------------------

bool GameScreen_RainbowIslands::SetUpLevel()
{
	//Load the background texture.
	mBackgroundTexture = new Texture2D(mRenderer);
	if (!mBackgroundTexture->LoadFromFile("Images/RainbowIslands/Tiles.png"))
	{
		cout << "Failed to load background texture!";
		return false;
	}

	//Create the level map.
	SetLevelMap();

	mNeuralNet = new NeuralNet();
	CreateStartingCharacters();

	//now we have a character, note the weights in his NN
	mNumWeightsInNN = mNeuralNet->GetNumberOfWeights();

	//instantiate GA class
	mGA = new GenAlg(kRunsPerGeneration, mNumWeightsInNN);
	mGenomeVector = mGA->GetChromos();

	ReadWeightsFile();

	mTimeToCompleteLevel = LEVEL_TIME;
	mTriggeredAnger = false;

	cout << "Run through: " << mRunThrough << endl;
	cout << "GENERATION: " << mGeneration << endl;
	return true;
}

//--------------------------------------------------------------------------------------------------

void GameScreen_RainbowIslands::RestartLevel()
{
	mNeuralNet->NetworkTrainingEpoch(mBubCharacter->GetPoints(), mInputs, mOutputs, kMaxBubScore);

	cout << "Run through: " << mRunThrough << endl;
	cout << "GENERATION: " << mGeneration << endl;
	
	//every 10th restart, epoch the GA, reset genome vector with functional operators applied to it from ga
	
	//every restart, calc fitness, store against weights as genome
	double fitness = ((kRainbowIslandsScreenHeight - mBubCharacter->GetCentralPosition().y) * kPosWeight) + (mBubCharacter->GetTimeAlive() * kAliveWeight) + (mBubCharacter->GetPoints() * kScoreWeight);
	cout << "Fitness: " << fitness << endl;

	mGenomeVector[mRunThrough].mFitness = fitness;
	mNeuralNet->PutWeights(mGenomeVector[mRunThrough++].mWeights);
	

	//increment counter
	mRunThrough++;

	//10 runs per generation, then run the GA
	if (mRunThrough >= (kRunsPerGeneration)) //0-9
	{
		SaveWeightsFile();

		//set of 10 finished
		mRunThrough = 0;
		mGeneration++;
		{
			vector<Genome> oldGenome = mGenomeVector;
			//so update the genome vector
			mGenomeVector = mGA->Epoch(oldGenome);
		}
	}

	delete mBubCharacter; 
	mBubCharacter = NULL;

	//Level map.
	delete mLevelMap;
	mLevelMap = NULL;

	//Enemies.
	for each (Character* var in mEnemies)
	{
		var->~Character();
		delete var;
	}
	mEnemies.clear();

	//Fruit.
	for each(Character* var in mFruit)
	{
		delete var;
	}
	mFruit.clear();

	//Rainbows.
	mRainbows.clear();

	//Treasure chest.
	if (mChest != NULL)
	{
		delete mChest;
		mChest = NULL;
	}

	//Create the level map.
	SetLevelMap();

	//Respawn characters and map.
	CreateStartingCharacters();

	mTimeToCompleteLevel = LEVEL_TIME;
	mTriggeredAnger = false;
}

//--------------------------------------------------------------------------------------------------

void GameScreen_RainbowIslands::CreateStartingCharacters()
{
	//Set up the player character.
	mBubCharacter = new CharacterBub(mRenderer, "Images/RainbowIslands/bub.png", mLevelMap, Vector2D(100, 570));
	mCanSpawnRainbow = true;

	//Set up the bad guys.
	CreateCaterpillar(Vector2D(150, 500), FACING_LEFT);
	CreateCaterpillar(Vector2D(130, 403), FACING_LEFT);
	CreateCaterpillar(Vector2D(270, 450), FACING_LEFT);
	CreateCaterpillar(Vector2D(30, 350), FACING_RIGHT);
	CreateCaterpillar(Vector2D(30, 160), FACING_RIGHT);
	CreateCaterpillar(Vector2D(270, 160), FACING_LEFT);
	CreateCaterpillar(Vector2D(250, 227), FACING_RIGHT);

	//Setup starting pickups.
	CreateFruit(Vector2D(26, 575), false);
	CreateFruit(Vector2D(46, 575), false);
	CreateFruit(Vector2D(256, 575), false);
	CreateFruit(Vector2D(276, 575), false);
	CreateFruit(Vector2D(26, 544), false);
	CreateFruit(Vector2D(256, 544), false);
	CreateFruit(Vector2D(300, 544), false);
	CreateFruit(Vector2D(100, 496), false);
	CreateFruit(Vector2D(200, 496), false);
	CreateFruit(Vector2D(256, 450), false);
	CreateFruit(Vector2D(246, 227), false);
	CreateFruit(Vector2D(276, 227), false);
	CreateFruit(Vector2D(8, 160), false);
	CreateFruit(Vector2D(32, 160), false);
	CreateFruit(Vector2D(56, 160), false);
	CreateFruit(Vector2D(270, 160), false);
	CreateFruit(Vector2D(140, 177), false);
	CreateFruit(Vector2D(83, 112), false);
	CreateFruit(Vector2D(103, 112), false);
	CreateFruit(Vector2D(123, 112), false);

	mChest = NULL;
	mTriggeredChestSpawns = false;
}

//--------------------------------------------------------------------------------------------------

void GameScreen_RainbowIslands::SetLevelMap()
{
	int backgroundMap[MAP_HEIGHT][MAP_WIDTH] = {
	{5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5},
	{5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5},
	{5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5},
	{5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,30,38,31},
	{7,7,7,7,8,5,5,6,7,7,7,7,7,7,7,8,5,5,5,5},
	{12,12,12,12,13,4,4,11,12,12,12,12,12,12,12,13,4,4,4,4},
	{5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5},
	{5,5,27,28,29,5,5,5,5,5,5,5,5,5,5,25,26,5,5,5},
	{5,5,32,33,34,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5},
	{5,5,5,5,5,5,5,5,5,5,5,25,26,5,5,5,5,5,5,5},
	{22,5,14,14,5,5,5,5,5,5,5,5,5,5,5,5,14,5,21,22},
	{4,4,4,4,4,5,14,14,14,14,5,5,5,5,5,5,4,4,4,4},
	{5,5,5,5,5,5,4,4,4,4,4,5,5,5,5,5,5,5,5,5},
	{5,5,5,5,5,5,5,5,5,5,5,5,5,27,28,29,5,30,37,31},
	{5,5,5,5,5,5,5,5,5,5,5,5,5,32,33,34,14,5,14,14},
	{4,4,4,5,5,14,14,25,26,5,5,5,5,5,5,4,4,4,4,4},
	{5,5,5,5,5,4,4,5,5,5,5,5,5,5,5,5,5,5,5,5},
	{5,25,26,5,5,5,5,14,14,14,14,14,6,7,7,7,7,8,5,5},
	{5,5,5,5,4,4,4,4,4,4,4,4,11,12,12,12,12,13,4,4},
	{5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5},
	{4,4,4,5,5,5,19,5,19,5,25,26,5,5,5,5,5,5,5,25},
	{5,5,5,5,5,17,18,17,18,5,5,5,5,5,5,5,5,5,5,5},
	{21,22,21,22,5,15,16,15,16,5,5,5,5,19,5,5,5,30,36,31},
	{0,0,0,1,0,0,1,0,0,0,0,1,0,0,5,5,5,19,5,5},
	{5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,0,0,0},
	{26,5,5,5,5,5,14,14,14,14,14,14,14,14,14,5,5,5,5,5},
	{5,5,5,19,0,0,0,0,0,0,0,0,0,0,0,5,5,5,5,19},
	{5,5,17,18,5,5,5,5,5,5,5,5,5,5,5,5,5,5,17,18},
	{14,14,15,16,5,5,5,5,5,5,5,5,27,28,29,5,14,14,15,16},
	{0,1,0,1,5,5,25,26,5,5,5,5,32,33,34,5,1,0,1,0},
	{5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5},
	{28,29,5,5,5,14,14,5,5,14,14,5,5,14,14,5,5,5,5,25},
	{33,34,19,5,5,0,0,1,0,0,0,0,1,0,0,5,5,5,5,5},
	{5,17,18,5,5,5,5,5,5,5,5,5,5,5,5,5,5,30,35,31},
	{14,15,16,5,27,28,29,5,19,5,5,5,19,5,5,5,5,5,5,5},
	{2,1,0,0,32,33,34,17,18,5,5,17,18,5,5,5,0,0,1,23},
	{24,24,24,9,9,9,5,15,16,5,5,15,16,5,9,9,9,24,24,24},
	{2,0,0,0,0,1,0,0,0,1,1,0,0,0,1,0,0,0,0,23},
	{20,20,20,20,20,20,10,10,10,20,20,10,10,10,20,20,20,20,20,20} };

	int collisionMap[MAP_HEIGHT][MAP_WIDTH] = {
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1 },
		{ 0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1 },
		{ 0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
		{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 } };

	//Clear up any old map.
	if (mLevelMap != NULL)
	{
		delete mLevelMap;
	}

	//Set the new one.
	mLevelMap = new LevelMap(collisionMap, backgroundMap);
}

//--------------------------------------------------------------------------------------------------

void GameScreen_RainbowIslands::CreateCaterpillar(Vector2D position, FACING direction)
{
	CharacterCaterpillar* caterpillarCharacter = new CharacterCaterpillar(mRenderer, "Images/RainbowIslands/Caterpillar.png", mLevelMap, position, direction);
	Character* tempCharacter = (Character*)caterpillarCharacter;
	mEnemies.push_back(tempCharacter);
	caterpillarCharacter = NULL;
}

//--------------------------------------------------------------------------------------------------

void GameScreen_RainbowIslands::CreateRainbow(Vector2D position, int numberOfRainbows)
{
	double xOffset = 0.0;
	float  spwnDelay = 0.0f;
	do
	{
		if (mBubCharacter->GetFacing() == FACING_LEFT)
			position.x -= xOffset;
		else
			position.x += xOffset;

		CharacterRainbow* rainbowCharacter = new CharacterRainbow(mRenderer, "Images/RainbowIslands/Rainbow.png", position, mBubCharacter->GetFacing(), spwnDelay);
		if (rainbowCharacter->GetFacing() == FACING_LEFT)
			rainbowCharacter->SetPosition(Vector2D(rainbowCharacter->GetPosition().x - rainbowCharacter->GetCollisionBox().width, rainbowCharacter->GetPosition().y));

		mRainbows.push_back(rainbowCharacter);
		numberOfRainbows--;
		xOffset = rainbowCharacter->GetCollisionBox().width - 10.0;
		spwnDelay += 200;
	} while (numberOfRainbows > 0);
}

//--------------------------------------------------------------------------------------------------

void GameScreen_RainbowIslands::CreateFruit(Vector2D position, bool bounce)
{
	CharacterFruit* fruitCharacter = new CharacterFruit(mRenderer, "Images/RainbowIslands/Pickups.png", mLevelMap, position, (FRUIT_TYPE)(rand() % FRUIT_MAX));
	mFruit.push_back(fruitCharacter);

	if (bounce)
		fruitCharacter->Jump();
}

//--------------------------------------------------------------------------------------------------

void GameScreen_RainbowIslands::CreateChest(Vector2D position)
{
	if (mChest == NULL)
	{
		mChest = new CharacterChest(mRenderer, "Images/RainbowIslands/Chest.png", mLevelMap, position);
	}
}

//--------------------------------------------------------------------------------------------------

void GameScreen_RainbowIslands::TriggerChestSpawns()
{
	Vector2D pos = mChest->GetCentralPosition();
	int xRange = (int)mChest->GetCollisionBox().width;
	int xDeductRange = (int)mChest->GetCollisionBox().width / 2;

	CreateFruit(Vector2D(pos.x + (rand() % xRange) - xDeductRange, pos.y), true);
	CreateFruit(Vector2D(pos.x + (rand() % xRange) - xDeductRange, pos.y), true);
	CreateFruit(Vector2D(pos.x + (rand() % xRange) - xDeductRange, pos.y), true);
	CreateFruit(Vector2D(pos.x + (rand() % xRange) - xDeductRange, pos.y), true);
	CreateFruit(Vector2D(pos.x + (rand() % xRange) - xDeductRange, pos.y), true);
	CreateFruit(Vector2D(pos.x + (rand() % xRange) - xDeductRange, pos.y), true);
	CreateFruit(Vector2D(pos.x + (rand() % xRange) - xDeductRange, pos.y), true);
	CreateFruit(Vector2D(pos.x + (rand() % xRange) - xDeductRange, pos.y), true);
	CreateFruit(Vector2D(pos.x + (rand() % xRange) - xDeductRange, pos.y), true);
	CreateFruit(Vector2D(pos.x + (rand() % xRange) - xDeductRange, pos.y), true);
	CreateFruit(Vector2D(pos.x + (rand() % xRange) - xDeductRange, pos.y), true);
	CreateFruit(Vector2D(pos.x + (rand() % xRange) - xDeductRange, pos.y), true);
	CreateFruit(Vector2D(pos.x + (rand() % xRange) - xDeductRange, pos.y), true);
	CreateFruit(Vector2D(pos.x + (rand() % xRange) - xDeductRange, pos.y), true);

	mTriggeredChestSpawns = true;
}

//--------------------------------------------------------------------------------------------------
//Debug output - Use to determine where you are checking
//--------------------------------------------------------------------------------------------------

void GameScreen_RainbowIslands::DrawDebugCircle(Vector2D centrePoint, double radius, int red, int green, int blue)
{
	Vector2D polarVec(0.0f, radius);

	float stepSize = 0.02f;
	float _360DegAsRads = (float)DegsToRads(360.0f);

	while (polarVec.x < _360DegAsRads)
	{
		Vector2D polarAsCart(polarVec.y * cos(polarVec.x), polarVec.y * sin(polarVec.x));
		Vector2D drawPoint(centrePoint.x + polarAsCart.x, centrePoint.y + polarAsCart.y);

		SDL_SetRenderDrawColor(mRenderer, red, green, blue, 255);
		SDL_RenderDrawPoint(mRenderer, (int)drawPoint.x, (int)drawPoint.y);

		polarVec.x += stepSize;
	}
}

//--------------------------------------------------------------------------------------------------

void GameScreen_RainbowIslands::DrawDebugLine(Vector2D startPoint, Vector2D endPoint, int red, int green, int blue)
{
	SDL_SetRenderDrawColor(mRenderer, red, green, blue, 255);
	SDL_RenderDrawLine(mRenderer, (int)startPoint.x, (int)startPoint.y, (int)endPoint.x, (int)endPoint.y);
}

//--------------------------------------------------------------------------------------------------