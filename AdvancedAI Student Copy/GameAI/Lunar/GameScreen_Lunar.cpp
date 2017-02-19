//------------------------------------------------------------------------
//  Author: Paul Roberts 2016
//------------------------------------------------------------------------

#include "GameScreen_Lunar.h"
#include <stdlib.h>
#include "../Texture2D.h"
#include "LunarConstants.h"
#include "LunarLander.h"
#include <time.h>
#include <fstream>
#include <sstream>
#include <vector>

//--------------------------------------------------------------------------------------------------

GameScreen_Lunar::GameScreen_Lunar(SDL_Renderer* renderer) : GameScreen(renderer)
{
	srand(static_cast <unsigned> (time(0)));

	//Set up the platform to land on.
	mPlatform = new Texture2D(renderer);
	mPlatform->LoadFromFile("Images/Lunar/Platform.png");

	//Create a lunar lander to use for tests.
	mPlatformPosition = Vector2D(198, 430);
	mLunarLander = new LunarLander(renderer, Vector2D(400, 125), Rect2D(mPlatformPosition.x, mPlatformPosition.y, mPlatform->GetWidth(), mPlatform->GetHeight()));

	//-------------------------------------
	//AI Controlled Lunar Lander setup.
	mGeneration = 0;
	GenerateRandomChromosomes();
	for (int i = 0; i < kNumberOfAILanders; i++)
	{
		mAILanders[i] = new LunarLander(renderer, Vector2D(400, 125), Rect2D(mPlatformPosition.x, mPlatformPosition.y, mPlatform->GetWidth(), mPlatform->GetHeight()));
	}
	RestartGA();
	mAllowMutation = true;
	//-------------------------------------

	//Start game in frozen state.
	mPause = true;

	//Start game in player control.
	mAIOn = false;

	//one ai action per second.
	mNextAction = true;

	mCycleCount = 0;
	mLandedCount = 0;
	mTotalLandedCount = 0;
	mVariableMutationRate = kMutationRate;
}

//--------------------------------------------------------------------------------------------------

GameScreen_Lunar::~GameScreen_Lunar()
{
	delete mPlatform;
	mPlatform = NULL;

	delete mLunarLander;
	mLunarLander = NULL;

	for (int i = 0; i < kNumberOfAILanders; i++)
	{
		delete *mAILanders;
	}
}

//--------------------------------------------------------------------------------------------------

void GameScreen_Lunar::Render()
{
	mPlatform->Render(mPlatformPosition);

	if(mAIOn)
	{
		for (int i = 0; i < kNumberOfAILanders; i++)
		{
			mAILanders[i]->Render();
		}
	}
	else
	{
		mLunarLander->Render();
	}
}

//--------------------------------------------------------------------------------------------------

void GameScreen_Lunar::Update(float deltaTime, SDL_Event e)
{
	//Additional input outside of player.
	switch(e.type)
	{
		//Deal with mouse click input.
		case SDL_KEYUP:
			switch(e.key.keysym.sym)
			{
				case SDLK_SPACE:
					mPause = !mPause;
					if (mPause)
					{
						cout << "Paused" << endl;
					}
					else
					{
						cout << "Unpaused" << endl;
					}
				break;

				case SDLK_a:
					mAIOn = !mAIOn;
					if (mAIOn)
					{
						cout << "AI on" << endl;
					}
					else
					{
						cout << "AI off" << endl;
					}
				break;

				case SDLK_LEFT:
					if (!mAIOn)
					{
						mLunarLander->TurnLeft(deltaTime);
					}
				break;

				case SDLK_RIGHT:
					if (!mAIOn)
					{
						mLunarLander->TurnRight(deltaTime);
					}
				break;

				case SDLK_UP:
					if (!mAIOn)
					{
						mLunarLander->Thrust();
					}
				break;

				case SDLK_r:
					if (!mAIOn)
					{
						mLunarLander->Reset();
					}
				break;
			}
		break;
	
		default:
		break;
	}
	
	if(!mPause)
	{
		if (!mAIOn)
		{
			mLunarLander->Update(deltaTime, e);

			if (!mLunarLander->IsAlive())
			{
				std::ostringstream ss;
				ss << mLunarLander->TestFitness();
				std::string fitStr = ss.str();
				cout << "Fitness score: " << mLunarLander->TestFitness() << endl;
				double disFit = fabs(mPlatformPosition.Length() - mLunarLander->GetCentralPosition().Length());
				cout << "disFit is: " << disFit << endl;
			}
		}
		else
		{
			UpdateAILanders(deltaTime, e);
		}
	}
}

//--------------------------------------------------------------------------------------------------

void GameScreen_Lunar::GenerateRandomChromosomes()
{
	for (int i = 0; i < kNumberOfAILanders; i++)
	{
		for (int j = 0; j < kNumberOfChromosomeElements; j++)
		{
			int action = rand() % 4;
			mChromosomes[i][j] = static_cast<LunarAction>(action);
		}
	}
}

inline const std::string GameScreen_Lunar::LunarActionAsString(LunarAction LA)
{
	switch (LA)
	{
		case LunarAction_None:
			return "LunarAction_None";
			break;
		case LunarAction_LunarLeft:
			return "LunarAction_Left";
			break;
		case LunarAction_LunarRight:
			return "LunarAction_LunarRight";
			break;
		case LunarAction_LunarThrust:
			return "LunarAction_LunarThrust";
			break;
		case LunarAction_MaxActions:
			return "LunarAction_MaxActions";
			break;
		default:
			return "No String conversion available for that LunarAction.";
			break;
	}
}

//--------------------------------------------------------------------------------------------------

void GameScreen_Lunar::ClearChromosomes()
{
	for (int i = 0; i < kNumberOfAILanders; i++)
	{
		for (int j = 0; j < kNumberOfChromosomeElements; j++)
		{
			mChromosomes[i][j] = static_cast<LunarAction>(0);
		}
	}
}

//--------------------------------------------------------------------------------------------------

void GameScreen_Lunar::UpdateAILanders(float deltaTime, SDL_Event e)
{	
	for (int x = 0; x < kNumberOfAILanders; x++)
	{
		mAILanders[x]->Update(deltaTime, e);
	}

	mCycleCount++;

	mAccumulatedDeltaTime += deltaTime;

	if (mNextAction)
	{
		mNextAction = false;
		for (int i = 0; i < kNumberOfAILanders; i++)
		{
			if (!mAILanders[i]->IsAlive())
			{
				//no need to check chromosomes if lander is dead or landed, continue to next AI lander.
				continue;
			}

			switch (mChromosomes[i][mCurrentAction])
			{
			case LunarAction_LunarLeft:
				mAILanders[i]->TurnLeft(deltaTime);
				break;
			case LunarAction_LunarRight:
				mAILanders[i]->TurnRight(deltaTime);
				break;
			case LunarAction_LunarThrust:
				mAILanders[i]->Thrust();
				break;
			case LunarAction_None:
				break;
			default:
				break;
			}
		}
	}

	if(mCycleCount >= 2.0f)
	{
		mCycleCount = 0.0f;
		mAccumulatedDeltaTime = 0.0f;
		mCurrentAction++;
		mNextAction = true;
	}

	if (AllAILandersLanded())
	{
		SaveSolution();
	}
	else if (AllAILandersDeadOrLanded())
	{
		CalculateFitness();
		Selection();
		//ElitistSelection();
		Crossover();
		Mutation();

		int fitnessTotal = 0;
		int i = 0;
		for (i = 0; i < kNumberOfAILanders; i++)
		{
			fitnessTotal += mFitnessValues[i];
		}
		cout << "Average fitness of all landers is: " << fitnessTotal / kNumberOfAILanders << endl;
		cout << "Successful landings this GA: " << mTotalLandedCount << endl;
		RestartGA();
	}

}

bool GameScreen_Lunar::AllAILandersLanded()
{
	for (int i = 0; i < kNumberOfAILanders; i++)
	{
		if (!mAILanders[i]->HasLanded() || !mAILanders[i]->IsAlive())
		{
			return false;
		}
	}
	return true;
}

bool GameScreen_Lunar::AllAILandersDeadOrLanded()
{
	for (int i = 0; i < kNumberOfAILanders; i++)
	{
		//If it's landed, it's ALIVE!
		if (mAILanders[i]->IsAlive() && !mAILanders[i]->HasLanded())
		{
			return false;
		}
	}
	return true;
}

//--------------------------------------------------------------------------------------------------

//calculate fitness for each AI lander based on the current chromosome generation.
void GameScreen_Lunar::CalculateFitness()
{
	for (int i = 0; i < kNumberOfAILanders; i++)
	{
		double distFromPlatformX = (mAILanders[i]->GetCentralPosition().x - mPlatformPosition.x);
		double distFromPlatformY = (mAILanders[i]->GetCentralPosition().y - mPlatformPosition.y);

		double disFit = (kLunarScreenWidth - distFromPlatformX) + (kLunarScreenHeight - distFromPlatformY);
		double rotFit = 1 / (fabs(mAILanders[i]->GetRotationAngle() + 1));
		double airTime = mAILanders[i]->GetSurvivalTime() / (mAILanders[i]->GetSpeed() + 1);
		
		mFitnessValues[i] = (kDistWeight * disFit) + (kRotWeight * rotFit) + (kAirTimeWeight * airTime);

		if (mAILanders[i]->HasLanded())
		{
			//mFitnessValues[i] += kLandingBonus;
			mFitnessValues[i] += kPerfectFitnessBonus;
			mLandedCount++;
			mTotalLandedCount++;
		}
		if (mLandedCount >= kRequiredLandings)
		{
			//mAllowMutation = false;
			if (mAllowMutation)
			{
				mVariableMutationRate--;
				mLandedCount = 0;
			}
			if (mVariableMutationRate < 1)
			{
				mVariableMutationRate = 1;
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------

void GameScreen_Lunar::Selection()
{
	//edited Tournament selection
	for (int i = 0; i < kNumberOfAILanders; i++)
	{
		//int randomIndex1 = rand() % kNumberOfAILanders;
		//int randomIndex2 = rand() % kNumberOfAILanders;
		int tournWinIndex = TournamentSelection(kTournamentSelectSampleSize);
		for (int j = 0; j < kNumberOfChromosomeElements; j++)
		{
			mSelectedAIChromosomes[i][j] = mChromosomes[tournWinIndex][j];
		}
		/*

		//int randInt = rand() % 10000;
		//if (randInt > kElitistRate) //biggest fitness goes in 75% of time
		//{
			//put biggest fitness in
			if (mFitnessValues[randomIndex1] > mFitnessValues[randomIndex2])
			{
				//index 1 is biggest
				for (int j = 0; j < kNumberOfChromosomeElements; j++)
				{
					mSelectedAIChromosomes[i][j] = mChromosomes[randomIndex1][j];
				}
			}
			else
			{
				//index 2 is biggest
				for (int j = 0; j < kNumberOfChromosomeElements; j++)
				{
					mSelectedAIChromosomes[i][j] = mChromosomes[randomIndex2][j];
				}
			}*/
		//}
		/*else
		{
			//put weakest fitness in
			if (mFitnessValues[randomIndex1] < mFitnessValues[randomIndex2])
			{
				//randomIndex1 is smallest
				for (int j = 0; j < kNumberOfChromosomeElements; j++)
				{
					mSelectedAIChromosomes[i][j] = mChromosomes[randomIndex1][j];
				}
			}
			else
			{
				//randomIndex2 is smallest
				for (int j = 0; j < kNumberOfChromosomeElements; j++)
				{
					mSelectedAIChromosomes[i][j] = mChromosomes[randomIndex2][j];
				}
			}
		}*/
	}
}

void GameScreen_Lunar::ElitistSelection()
{
	//find m copies of the n largest fitness to skip crossover and mutation
	int highestIndex = FindLargest(kNumberOfAILanders);
	int secondhighestIndex = FindSecondLargest(kNumberOfAILanders);
	for (int i = 0; i < kNumberOfChromosomeElements; i++)
	{
		mChromosomes[0][i] = mSelectedAIChromosomes[highestIndex][i];
		mChromosomes[1][i] = mSelectedAIChromosomes[secondhighestIndex][i];
	}
}

//returns index of largest
int GameScreen_Lunar::FindLargest(int sampleSize)
{
	int highestFitness = 0;
	int highestIndex = 0;
	for (int i = 0; i < sampleSize; i++)
	{
		if (mFitnessValues[i] > highestFitness)
		{
			highestFitness = mFitnessValues[i];
			highestIndex = i;
		}
	}
	return highestIndex;
}

int GameScreen_Lunar::FindSecondLargest(int sampleSize)
{
	int highestFitness = 0;
	int secondHighestFitness = 0;
	int highestIndex = 0;
	int secondHighestIndex = 0;

	for (int i = 0; i < sampleSize; i++)
	{
		if (mFitnessValues[i] > highestFitness)
		{
			secondHighestFitness = highestFitness;
			secondHighestIndex = highestIndex;
			highestFitness = mFitnessValues[i];
			secondHighestIndex = i;
		}
		else if (mFitnessValues[i] > secondHighestFitness)
		{
			secondHighestFitness = mFitnessValues[i];
			secondHighestIndex = i;
		}
	}

	return secondHighestIndex;

}

//returns index of the Lander with the highest fitness value, out of n randomly selected ones.
int GameScreen_Lunar::TournamentSelection(int n)
{
	int largestFitness = 0;
	int indexOfLargest = 0;
	
	for (int i = 0; i < n; i++)
	{
		int randomIndex = rand() % kNumberOfAILanders;
		if (mFitnessValues[randomIndex] > largestFitness)
		{
			largestFitness = mFitnessValues[randomIndex];
			indexOfLargest = randomIndex;
		}
	}
	return indexOfLargest;
}

void GameScreen_Lunar::Crossover()
{
	//multi point crossover
	for (int i = 0; i < kNumberOfAILanders; i += 2)
	{
		for (int j = 0; j < kNumberOfChromosomeElements; j++)
		{
			int crossoverCheckInt = rand() % 10000; //50% chance
			if (crossoverCheckInt < kCrossoverRate)
			{
				mChromosomes[i + 1][j] = mSelectedAIChromosomes[i][j]; //child 2 = parent 1
				mChromosomes[i][j] = mSelectedAIChromosomes[i + 1][j]; //child 1 = parent 2
			}
			else
			{
				mChromosomes[i][j] = mSelectedAIChromosomes[i][j]; //child 1 = parent 1
				mChromosomes[i + 1][j] = mSelectedAIChromosomes[i + 1][j]; //child 2 = parent 2
			}
		}
	}
	//single point crossover.
	/*
	for (int i = 2; i < kNumberOfAILanders; i+=2)
	{
		bool parentsIdentical = true;
		//parental check
		for (int j = 0; j < kNumberOfChromosomeElements; j++)
		{
			if (mSelectedAIChromosomes[i][j] != mSelectedAIChromosomes[i + 1][j])
			{
				parentsIdentical = false;
			}
		}

		int crossoverCheckInt = rand() % 10000;
		if ((crossoverCheckInt > kCrossoverRate) || parentsIdentical)
		{
			//no crossover for these two
			for (int k = 0; k < kNumberOfChromosomeElements; k++)
			{
				mChromosomes[i][k] = mSelectedAIChromosomes[i][k];
				mChromosomes[i+1][k] = mSelectedAIChromosomes[i + 1][k];
			}
		}
		else
		{
			//single point crossover occurs
			int crossOverPoint = rand() % kNumberOfChromosomeElements;
			for (int m = 0; m < crossOverPoint; m++)
			{
				mChromosomes[i][m] = mSelectedAIChromosomes[i][m];//mum //child1
				mChromosomes[i+1][m] = mSelectedAIChromosomes[i+1][m];//dad //child2
			}
			for (int n = crossOverPoint; n < kNumberOfChromosomeElements; n++)
			{
				mChromosomes[i][n] = mSelectedAIChromosomes[i + 1][n];//dad //child1
				mChromosomes[i+1][n] = mSelectedAIChromosomes[i][n];//mum //child2
			}
		}
	}*/
	
}

void GameScreen_Lunar::TransferSelected()
{
	for (int i = 0; i < kNumberOfAILanders; i++)
	{
		for (int j = 0; j < kNumberOfChromosomeElements; j++)
		{
			mChromosomes[i][j] = mSelectedAIChromosomes[i][j];
		}
	}
}

//--------------------------------------------------------------------------------------------------

void GameScreen_Lunar::Mutation()
{
	if (mAllowMutation)
	{
		for (int i = 0; i < kNumberOfAILanders; i++)
		{
			for (int j = 0; j < kNumberOfChromosomeElements; j++)
			{
				int randomInt = rand() % 10000;
				if (randomInt <= mVariableMutationRate)
				{
					mChromosomes[i][j] = static_cast<LunarAction>(rand() % 4);
				}
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------

void GameScreen_Lunar::RestartGA()
{
	/*if(mGeneration >= 500)
	{
		GenerateRandomChromosomes();
		mGeneration = 0;
		mAllowMutation = true;
		cout << endl << "---FAILED (Restart)---" << endl << endl;
	}*/

	for(int i = 0; i < kNumberOfAILanders; i++)
	{
		mFitnessValues[i] = 0.0f;
		mAILanders[i]->Reset();
	}

	mAccumulatedDeltaTime = 0.0f;
	mCurrentAction		  = 0;

	mGeneration++;
	cout << "---GENERATION " << mGeneration << "---" << endl;
}

//--------------------------------------------------------------------------------------------------

void GameScreen_Lunar::SaveSolution()
{
	//Output the solution of the first chromosome - They should all match at the point we output so it
	//shouldn't matter which chromosome is used for the solution.
	ofstream outFile("Solution.txt");
	if(outFile.is_open())
	{
		for(int action = 0; action < kNumberOfChromosomeElements; action++)
		{
			switch(mChromosomes[0][action])
			{
				case LunarAction_None:
					outFile << "0";
				break;
			
				case LunarAction_LunarThrust:
					outFile << "1";
				break;

				case LunarAction_LunarLeft:
					outFile << "2";
				break;

				case LunarAction_LunarRight:
					outFile << "3";
				break;
			}
		
			outFile << ",";
		}

		outFile << endl << "Generations taken: " << mGeneration;
		outFile << endl << "Platform start (" << mPlatformPosition.x << "," << mPlatformPosition.y << ")";
		outFile << endl << "Ship start (" << mAILanders[0]->GetStartPosition().x << "," << mAILanders[0]->GetStartPosition().y << ")";

		outFile.close();
	}
}

//--------------------------------------------------------------------------------------------------
